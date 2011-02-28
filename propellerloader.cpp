#include "propellerloader.h"
#include <QApplication>

using namespace PZST;

PropellerLoader::PropellerLoader() : QThread(), device(NULL), progress(NULL)
{

}

PropellerLoader::PropellerLoader(ESerialPort *device, QProgressBar *progress, QLabel *label)
    : QThread()
{
    this->device = device;
    this->label = label;
    this->progress = progress;
    if (label) connect(this, SIGNAL(setLabelText(QString)), label, SLOT(setText(QString)));
    if (progress) {
        connect(this, SIGNAL(setProgressMax(int)), progress, SLOT(setMaximum(int)));
        connect(this, SIGNAL(setProgressValue(int)), progress, SLOT(setValue(int)));
    }
}

void PropellerLoader::updateFirmware(int command, QByteArray data)
{
    this->command = command;
    this->data = data;
    this->result = LoaderBusy;
    start();
}

bool PropellerLoader::getReply(char *c, int timeout)
{
    t.start();
    int time = 0;
    while (timeout > t.elapsed()) {
        device->putChar(0xF9);
        int spent = t.elapsed() - time;
        advance(spent);
        time += spent;
        if (device->getChar(c)) {
            return true;
        }
        msleep(20);
    }
    return false;
}

void PropellerLoader::sendByte(uint8_t byte)
{
    char buf[8];
    for (int i = 0; i < 8; i++) {
        if (byte & 1) buf[i] = -1;
        else buf[i] = -2;
    }
    device->write(buf, 8);
}

void PropellerLoader::sendLong(uint32_t num)
{
    char buf[32];
    for (int i = 0; i < 32; i++) {
        if (num & 1) buf[i] = -1;
        else buf[i] = -2;
        num >>= 1;
    }
    device->write(buf, 32);
}

void PropellerLoader::sendBytes(char* data, uint16_t count)
{
    int sent = 0;
    int time = 0;
    while (sent < count) {
        int toSend = count - sent;
        if (toSend > 16) {
            toSend = 16;
        }
        int written = device->write(data + sent, toSend);
        if (written < 0) break;
        sent += written;
        int spent = t.elapsed() - time;
        advance(spent);
        time += spent;
    }
}

void PropellerLoader::sendCalibration()
{
    device->putChar(0xF9);
}

uint8_t PropellerLoader::iterateLFSR()
{
    uint8_t bit = LFSR & 1;
    LFSR = (LFSR << 1) | (((LFSR >> 7) ^ (LFSR >> 5) ^ (LFSR >> 4) ^ (LFSR >> 1)) & 1);
    return bit;
}

void PropellerLoader::advance(int value)
{
    progressValue += value;
    if (progress) {
        emit setProgressMax(progressMax);
        emit setProgressValue(progressValue);
    }
}

PropellerPacker::PropellerPacker() : bits(0xFFFFU), bitsUsed(0)
{

}

void PropellerPacker::pack(uint8_t bit)
{
    int bitsRequired;
    uint16_t mask;
    if (bit) {
        bitsRequired = 2;
        mask = 1;
    } else {
        bitsRequired = 3;
        mask = 3;
    }
    if (bitsRequired + bitsUsed > 10) {
        uint8_t byte = (bits >> 1) & 0xFF;
        bytes.append(byte);
        bitsUsed = 0;
        bits = 0xFFFFU;
    }
    mask = ~(mask << bitsUsed);
    bits &= mask;
    bitsUsed += bitsRequired;
}

QByteArray& PropellerPacker::finalize()
{
    if (bitsUsed) {
        uint8_t byte = (bits >> 1) & 0xFF;
        bytes.append(byte);
    }
    bitsUsed = 0;
    bits = 0xFFFFU;
    return bytes;
}

void PropellerLoader::run()
{
    int res, i;
    device->setDtr(true);
    device->setDtr(false);
    {
        // allow some time for chip startup
        t.start();
        while (t.elapsed() < 100) {};
    }

    sendCalibration();
    char bytes[258];

    LFSR = 'P';
    for (i = 0; i < 250; i++) {
        bytes[i] = (iterateLFSR() ? -1 : -2);
    }
    device->write(bytes, 250);

    for (i = 0; i < 258; i++) {
        bytes[i] = 0xF9;
    }
    device->write(bytes, 250);

    int bytesRead = 0;
    char c[250];
    while ((bytesRead < 250) && (t.elapsed() < 1000)) {
        res = device->read(c + bytesRead, 250 - bytesRead);
        if (res < 0) {
            result = LoaderCommError;
            return;
        }
        bytesRead += res;
    }

    for (i = 0; i < 250; i++) {
        uint8_t my = iterateLFSR();
        uint8_t reply = (c[i] == -1) ? 1 : 0;
        if (my != reply) {
            result = PropellerLoader::LoaderCommError;
            return;
        }
    }

    bytesRead = 0;
    device->write(bytes + 250, 8);
    while ((bytesRead < 8) && (t.elapsed() < 1000)) {
        res = device->read(c + bytesRead, 8 - bytesRead);
        if (res < 0) {
            result = LoaderCommError;
            return;
        }
        bytesRead += res;
    }
    if (bytesRead < 8) {
        result = PropellerLoader::LoaderCommError;
        return;
    }

    uint8_t version = 0;
    for (i = 0; i < 8; i++) {
        version = (version >> 1) | ((c[i] == -1) ? 0x80 : 0);
    }
    if (version != 1) {
        result = LoaderCommError;
        return;
    }

    if (command < 1) {
        result = LoaderOK;
        return;
    }

    // naive sanity checks
    if (data.size() < 10)  {
        result = PropellerLoader::LoaderInvalidFile;
        return;
    }
    if (data.size() > 0x8000)  {
        result = PropellerLoader::LoaderInvalidFile;
        return;
    }
    uint16_t binarySize = (uint8_t)data[8] + ((uint8_t)data[9] << 8);
    if (binarySize != data.size()) {
        result = PropellerLoader::LoaderInvalidFile;
        return;
    }

    if (binarySize & 3) {
        result = PropellerLoader::LoaderInvalidFile;
        return;
    }
    // pack binary
    PropellerPacker packer;
    for (i = 0; i < data.size(); i++) {
        uint8_t byte = (uint8_t)data[i];
        for (int j = 0; j < 8; j++) {
            packer.pack(byte & 1);
            byte >>= 1;
        }
    }
    QByteArray packed = packer.finalize();

    progressValue = 0;
    progressMax = packed.size() * 868 / 10000;

    if (command > 1) {
        progressMax += programMilli + verifyMilli;
    }

    // send command
    sendLong(command);

    if (command < 1) {
        result = LoaderOK;
        return;
    }

    // send count
    sendLong(binarySize >> 2);

    emit setLabelText(tr("Uploading program"));
    t.start();
    sendBytes(packed.data(), packed.size());
    int time = t.elapsed();
    while (t.elapsed() < packed.size()* 868 / 10000) {
        int spent = t.elapsed() - time;
        if (spent) {
            advance(spent);
            time += spent;
        }
    };

    char reply;
    if (!getReply(&reply, 1500)) {
        result = PropellerLoader::LoaderCommError;
        return;
    }
    if (reply != -2) {
        result = PropellerLoader::LoaderCommError;
        return;
    }
    if (command > 1) {
        emit setLabelText(tr("Programming EEPROM"));
        if (!getReply(&reply, programMilli)) {
            result = PropellerLoader::LoaderCommError;
            return;
        }
        if (reply != -2) {
            result = PropellerLoader::LoaderCommError;
            return;
        }
        emit setLabelText(tr("Verifying EEPROM"));
        if (!getReply(&reply, verifyMilli)) {
            result = PropellerLoader::LoaderCommError;
            return;
        }
        if (reply != -2) {
            result = PropellerLoader::LoaderCommError;
            return;
        }
    }
    result = PropellerLoader::LoaderOK;
}


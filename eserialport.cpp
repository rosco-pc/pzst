#include "eserialport.h"

using namespace PZST;

ESerialPort::ESerialPort()
    : QIODevice()
{
    initialize();
}

ESerialPort::ESerialPort(QObject *parent)
    : QIODevice(parent)
{
    initialize();
}

ESerialPort::~ESerialPort()
{
    close();
}

bool ESerialPort::canReadLine() const
{
    return false;
}

void ESerialPort::initialize()
{
    fd = ESP_INVALID_HANDLE;
    baudRate = B_9600;
    timeout = 10;
}

bool ESerialPort::setBaudRate(BaudRate rate)
{
    if (nativeSetBaudRate(rate)) {
        baudRate = rate;
        return true;
    }
    return false;
}

bool ESerialPort::isOpen()
{
    return fd >= 0;
}

bool ESerialPort::isSequential() const
{
    return true;
}

void ESerialPort::setDeviceName(QString name)
{
    deviceName = name;
}

bool ESerialPort::reset()
{
    return false;
}

bool ESerialPort::seek(qint64 pos)
{
    Q_UNUSED(pos);
    return false;
}

void ESerialPort::setTimeout(int milli)
{
    timeout = milli;
}

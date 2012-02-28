#include "eserialportproxy.h"
#include "eserialportmanager.h"

namespace PZST {

ESerialPortProxy::ESerialPortProxy(ESerialPort *port) :
        port(port), isOwned(false), dtrState(false),
        rate(ESerialPort::B_9600)
{
}

ESerialPortProxy::~ESerialPortProxy()
{
    ESerialPortManager::release(this);
}

void ESerialPortProxy::grab()
{
    if (isOwned) return;
    isOwned = ESerialPortManager::grab(this);
    if (isOwned) restoreState();
}

void ESerialPortProxy::release()
{
    if (!isOwned) return;
}

bool ESerialPortProxy::open(QIODevice::OpenMode mode)
{
    if (!isOwned) return false;
    return port->open(mode);
}

bool ESerialPortProxy::setBaudRate(ESerialPort::BaudRate rate)
{
    this->rate = rate;
    if (!isOwned) return false;
    return port->setBaudRate(rate);
}

void ESerialPortProxy::close()
{
    if (isOwned) port->close();
}

bool ESerialPortProxy::putChar(char c)
{
    if (!isOwned) return false;
    return port->putChar(c);
}

bool ESerialPortProxy::getChar(char *c)
{
    if (!isOwned) return false;
    return port->getChar(c);
}

qint64 ESerialPortProxy::write(const char *data)
{
    if (!isOwned) return -1;
    return port->write(data);
}

qint64 ESerialPortProxy::write(const char *data, qint64 maxSize)
{
    if (!isOwned) return -1;
    return port->write(data, maxSize);
}

qint64 ESerialPortProxy::write(const QByteArray &byteArray)
{
    if (!isOwned) return -1;
    return port->write(byteArray);
}

qint64 ESerialPortProxy::read(char *data, qint64 maxSize)
{
    if (!isOwned) return -1;
    return port->read(data, maxSize);
}

QByteArray ESerialPortProxy::read(qint64 maxSize)
{
    if (!isOwned) return QByteArray();
    return port->read(maxSize);
}

void ESerialPortProxy::setDtr(bool v)
{
    if (isOwned) port->setDtr(v);
}

void ESerialPortProxy::restoreState()
{
    setDtr(dtrState);
    setBaudRate(rate);
}

};

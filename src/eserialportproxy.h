#ifndef ESERIALPORTPROXY_H
#define ESERIALPORTPROXY_H

#include "eserialport.h"

namespace PZST {

class ESerialPortProxy
{
    friend class ESerialPortManager;
public:
    ESerialPortProxy(ESerialPort *port);
    ~ESerialPortProxy();
    bool open(QIODevice::OpenMode mode);
    bool setBaudRate(ESerialPort::BaudRate rate);
    const QString &getDeviceName() {return port->getDeviceName();}
    void close();
    bool putChar (char c);
    bool getChar (char *c);
    qint64 write(const char* data, qint64 maxSize);
    qint64 write(const char* data );
    qint64 write(const QByteArray& byteArray);
    qint64 read(char* data, qint64 maxSize);
    QByteArray read(qint64 maxSize);
    void setDtr(bool v);
    void grab();
    void release();
    bool isOpen();
private:
    ESerialPort *getPort() {return port;}
    void restoreState();
    ESerialPort *port;
    bool isOwned;
    bool dtrState;
    ESerialPort::BaudRate rate;
};

};

#endif // ESERIALPORTPROXY_H

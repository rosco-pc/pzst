#include "eserialport.h"
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <QFile>
#include <QStringList>

using namespace PZST;

qint64 ESerialPort::bytesAvailable() const
{
    if (fd < 0) return -1;
    size_t nbytes = 0;
    if (::ioctl(fd, FIONREAD, &nbytes) == -1) {
        return (qint64)-1;
    }
    return (qint64)nbytes + QIODevice::bytesAvailable();
}

void ESerialPort::close()
{
    if (fd >= 0) {
        QIODevice::close();
        ::close(fd);
        fd = -1;
    }
}

bool ESerialPort::open(OpenMode mode)
{
    if (!(mode & ReadWrite)) return false;
    int options = O_NDELAY;
    if ((mode & ReadWrite) == ReadOnly) options |= O_RDONLY;
    if ((mode & ReadWrite) == WriteOnly) options |= O_WRONLY;
    if ((mode & ReadWrite) == ReadWrite) options |= O_RDWR;
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
        QIODevice::close();
    }
    fd = ::open(deviceName.toAscii().data(), options);
    if (fd < 0) {
        return false;
    }
    tcflush(fd, TCIOFLUSH);
    struct termios newtio;
    if (tcgetattr(fd, &newtio))
    {
        ::close(fd);
        fd = -1;
        return false;
    }
    speed_t baud = (speed_t)nativeBaudRate(baudRate);
    cfsetospeed(&newtio, baud);
    cfsetispeed(&newtio, baud);
    newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~(PARENB | PARODD);
    newtio.c_cflag &= ~CRTSCTS;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_iflag=IGNBRK;
    newtio.c_iflag &= ~(IXON|IXOFF|IXANY);
    newtio.c_lflag=0;
    newtio.c_oflag=0;
    newtio.c_cc[VTIME]=1;
    newtio.c_cc[VMIN]=60;
    if (tcsetattr(fd, TCSANOW, &newtio)!=0)
    {
        ::close(fd);
        fd = -1;
        return false;
    }

    int mcs=0;
    ioctl(fd, TIOCMGET, &mcs);
    mcs |= TIOCM_RTS;
    ioctl(fd, TIOCMSET, &mcs);

    if (tcgetattr(fd, &newtio)!=0)
    {
        ::close(fd);
        fd = -1;
        return false;
    }

    newtio.c_cflag &= ~CRTSCTS;
    if (tcsetattr(fd, TCSANOW, &newtio)!=0)
    {
        ::close(fd);
        fd = -1;
        return false;
    }
    QIODevice::open(mode);
    return true;
}

qint64 ESerialPort::readData(char *data, qint64 maxlen)
{
    if (fd < 0) return -1;
    ssize_t n;
    qint64 bytesRead = 0;
    struct timeval tv;
    fd_set rfd;
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;
    FD_ZERO( &rfd );
    FD_SET(fd, &rfd );
    while (bytesRead < maxlen) {
        n = ::select(1 + fd, &rfd, 0, 0, &tv);
        if (n < 0) return -1;
        if (!n) return bytesRead;
        int toRead = qMin(maxlen, (qint64)512);
        n = ::read(fd, data + bytesRead, toRead);
        if (n < 0) {
            if (errno != EAGAIN) {
                return -1;
            }
            n = 0;
        }
        if (n < 0) return -1;
        bytesRead += n;
        maxlen -= n;
    }
    return bytesRead;
}

qint64 ESerialPort::writeData(const char *data, qint64 len)
{
    struct timeval tv;
    qint64 bytesWritten = 0;
    fd_set rfd;
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;
    FD_ZERO( &rfd );
    FD_SET(fd, &rfd);
    int n;
    while (bytesWritten < len) {
        n = ::select(1 + fd, 0, &rfd, 0, &tv);
        if (n < 0) return -1;
        if (!n) return bytesWritten;
        qint64 toWrite = qMin(len, (qint64)512);
        n = ::write(fd, (char*)data + bytesWritten, toWrite);
        if (n < 0) {
            if (errno != EAGAIN) {
                return -1;
            }
            n = 0;
        }
        if (n < 0) return -1;
        bytesWritten += n;
        len -= n;
    }
    return bytesWritten;
}

bool ESerialPort::nativeSetBaudRate(BaudRate rate)
{
    if (fd < 0) return true;
    speed_t r = nativeBaudRate(rate);
    struct termios newtio;
    if (::tcgetattr(fd, &newtio))
    {
        return false;
    }
    ::cfsetospeed(&newtio, r);
    cfsetispeed(&newtio, r);
    if (tcsetattr(fd, TCSANOW, &newtio)!=0)
    {
        return false;
    }
    return true;
}

int ESerialPort::nativeBaudRate(BaudRate rate)
{
    speed_t r;
    switch (rate) {
    case B_50:
            r = B50;
            break;
    case B_75:
            r = B75;
            break;
    case B_110:
            r = B110;
            break;
    case B_134:
            r = B134;
            break;
    case B_150:
            r = B150;
            break;
    case B_200:
            r = B200;
            break;
    case B_300:
            r = B300;
            break;
    case B_600:
            r = B600;
            break;
    case B_1200:
            r = B1200;
            break;
    case B_1800:
            r = B1800;
            break;
    case B_2400:
            r = B2400;
            break;
    case B_4800:
            r = B4800;
            break;
    case B_9600:
            r = B9600;
            break;
    case B_19200:
            r = B19200;
            break;
    case B_38400:
            r = B38400;
            break;
    case B_57600:
            r = B57600;
            break;
    case B_115200:
            r = B115200;
            break;
    case B_230400:
            r = B230400;
            break;
    case B_460800:
            r = B460800;
            break;
    case B_500000:
            r = B500000;
            break;
    case B_576000:
            r = B576000;
            break;
    case B_921600:
            r = B921600;
            break;
    case B_1000000:
            r = B1000000;
            break;
    case B_1152000:
            r = B1152000;
            break;
    case B_1500000:
            r = B1500000;
            break;
    case B_2000000:
            r = B2000000;
            break;
    case B_2500000:
            r = B2500000;
            break;
    case B_3000000:
            r = B3000000;
            break;
    case B_3500000:
            r = B3500000;
            break;
    case B_4000000:
            r = B4000000;
            break;
    default:
            r = (speed_t)rate;
            break;
    }

    return (int)r;
}

void ESerialPort::setDtr(bool v)
{
    if (fd < 0) return;
    int control;
    if (ioctl(fd, TIOCMGET, &control)) return;
    if (v) {
        control |= TIOCM_DTR;
    } else {
        control &= ~TIOCM_DTR;
    }
    ioctl(fd, TIOCMSET, &control);
}


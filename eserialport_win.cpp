#include "eserialport.h"
#include <QTime>
void ESerialPort::close()
{
    if (fd != ESP_INVALID_HANDLE) {
        QIODevice::close();
        ::CloseHandle(fd);
        fd = ESP_INVALID_HANDLE;
    }
}

bool ESerialPort::open(OpenMode mode)
{
    if (!(mode & ReadWrite)) return false;
    DWORD options = 0;
    if (mode & ReadOnly) options |= GENERIC_READ;
    if (mode & ReadWrite) options |= GENERIC_WRITE;
    if (fd != ESP_INVALID_HANDLE) {
        ::CloseHandle(fd);
        fd = ESP_INVALID_HANDLE;
        QIODevice::close();
    }
    fd = CreateFileA(deviceName.toAscii().data(),
                    options,
                    0,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    0);
    if (fd == ESP_INVALID_HANDLE) {
        return false;
    }
    FlushFileBuffers(fd);
    PurgeComm(fd, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(fd, &dcbSerialParams)) {
        ::CloseHandle(fd);
        fd = ESP_INVALID_HANDLE;
        return false;
    }
    DWORD baud = (DWORD)nativeBaudRate(baudRate);
    dcbSerialParams.BaudRate = baud;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    if(!SetCommState(fd, &dcbSerialParams)){
        ::CloseHandle(fd);
        fd = ESP_INVALID_HANDLE;
        return false;
    }


    COMMTIMEOUTS timeouts={0};
    timeouts.ReadIntervalTimeout=1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier=1;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=10;
    if(!SetCommTimeouts(fd, &timeouts)){
        ::CloseHandle(fd);
        fd = ESP_INVALID_HANDLE;
        return false;
    }

    if (!EscapeCommFunction(fd, CLRDTR)) {
        ::CloseHandle(fd);
        fd = ESP_INVALID_HANDLE;
        return false;
    }
    {
        QTime t;
        t.start();
        while (t.elapsed() < 60) {};
    }
    QIODevice::open(mode);
    return true;
}

qint64 ESerialPort::readData(char *data, qint64 maxlen)
{
    if (fd == ESP_INVALID_HANDLE) return -1;
    bool success= false;
    OVERLAPPED o= {0};
    DWORD dwRead = 0;
    o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!ReadFile(fd, data, maxlen, &dwRead, &o))
    {
       if (GetLastError() == ERROR_IO_PENDING)
           if (WaitForSingleObject(o.hEvent, 1000) == WAIT_OBJECT_0)
              if (GetOverlappedResult(fd, &o, &dwRead, FALSE))
                success = true;
    } else
        success = true;
    if (dwRead != maxlen)
       success = false;
    CloseHandle(o.hEvent);
    return dwRead;
}

qint64 ESerialPort::writeData(const char *data, qint64 len)
{
    QTime t;
    t.start();
    if (fd == ESP_INVALID_HANDLE) return -1;
    bool success= false;
    OVERLAPPED o= {0};
    DWORD dwWritten = 0;
    o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!WriteFile(fd, (LPCVOID)data, len, &dwWritten, &o))
    {
       if (GetLastError() == ERROR_IO_PENDING)
           if (WaitForSingleObject(o.hEvent, INFINITE) == WAIT_OBJECT_0)
              if (GetOverlappedResult(fd, &o, &dwWritten, FALSE))
                success = true;
    } else
        success = true;
    if (dwWritten != len)
       success = false;
    CloseHandle(o.hEvent);
    return dwWritten;
}

bool ESerialPort::nativeSetBaudRate(BaudRate rate)
{
    if (fd == ESP_INVALID_HANDLE) return true;
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(fd, &dcbSerialParams)) {
        return false;
    }
    DWORD baud = (DWORD)nativeBaudRate(rate);
    dcbSerialParams.BaudRate = baud;
    if(!SetCommState(fd, &dcbSerialParams)){
        return false;
    }
    return true;
}

int ESerialPort::nativeBaudRate(BaudRate rate)
{
    DWORD r;
    switch (rate) {
    case B_50:
            r = 50;
            break;
    case B_75:
            r = 75;
            break;
    case B_110:
            r = 110;
            break;
    case B_134:
            r = 134;
            break;
    case B_150:
            r = 150;
            break;
    case B_200:
            r = 200;
            break;
    case B_300:
            r = 300;
            break;
    case B_600:
            r = 600;
            break;
    case B_1200:
            r = 1200;
            break;
    case B_1800:
            r = 1800;
            break;
    case B_2400:
            r = 2400;
            break;
    case B_4800:
            r = 4800;
            break;
    case B_9600:
            r = 9600;
            break;
    case B_19200:
            r = 19200;
            break;
    case B_38400:
            r = 38400;
            break;
    case B_57600:
            r = 57600;
            break;
    case B_115200:
            r = 115200;
            break;
    case B_230400:
            r = 230400;
            break;
    case B_460800:
            r = 460800;
            break;
    case B_500000:
            r = 500000;
            break;
    case B_576000:
            r = 576000;
            break;
    case B_921600:
            r = 921600;
            break;
    case B_1000000:
            r = 1000000;
            break;
    case B_1152000:
            r = 1152000;
            break;
    case B_1500000:
            r = 1500000;
            break;
    case B_2000000:
            r = 2000000;
            break;
    case B_2500000:
            r = 2500000;
            break;
    case B_3000000:
            r = 3000000;
            break;
    case B_3500000:
            r = 3500000;
            break;
    case B_4000000:
            r = 4000000;
            break;
    default:
            r = (DWORD)rate;
            break;
    }
    return r;
}

void ESerialPort::setDtr(bool v)
{
    if (fd == ESP_INVALID_HANDLE) return;
    if (v) {
        EscapeCommFunction(fd, SETDTR);
    } else {
        EscapeCommFunction(fd, CLRDTR);
    }
}

qint64 ESerialPort::bytesAvailable() const
{
    if (fd == ESP_INVALID_HANDLE) return 0;
    DWORD Errors;
    COMSTAT Status;
    bool success=ClearCommError(fd, &Errors, &Status);
    if (success) {
        return Status.cbInQue + QIODevice::bytesAvailable();
    }
    return (unsigned int)-1;
}


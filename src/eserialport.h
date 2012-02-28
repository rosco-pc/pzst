#ifndef ESERIALPORT_H
#define ESERIALPORT_H

#include <QIODevice>

#ifdef Q_OS_WIN
#include <windows.h>
typedef HANDLE ESP_HANDLE;
#define ESP_INVALID_HANDLE INVALID_HANDLE_VALUE
#else
typedef int ESP_HANDLE;
#define ESP_INVALID_HANDLE -1
#endif

namespace PZST {
    class ESerialPort : public QIODevice
    {
        friend class ESerialPortManager;
        Q_OBJECT

    public:

        enum BaudRate {
            B_50 = 1,
            B_75,
            B_110,
            B_134,
            B_150,
            B_200,
            B_300,
            B_600,
            B_1200,
            B_1800,
            B_2400,
            B_4800,
            B_9600,
            B_19200,
            B_38400,
            B_57600,
            B_115200,
            B_230400,
            B_460800,
            B_500000,
            B_576000,
            B_921600,
            B_1000000,
            B_1152000,
            B_1500000,
            B_2000000,
            B_2500000,
            B_3000000,
            B_3500000,
            B_4000000,
        };

        virtual ~ESerialPort();

        static QStringList enumeratePorts();

        virtual qint64 bytesAvailable() const;
        virtual bool canReadLine () const;
        virtual void close();
        bool isOpen();
        virtual bool isSequential () const;
        virtual bool open(OpenMode mode);
        virtual qint64 pos() const {return 0;}
        virtual bool reset();
        virtual bool seek(qint64 pos);
        virtual qint64 readData(char *data, qint64 maxlen);
        virtual qint64 writeData(const char *data, qint64 len);

        void setDeviceName(QString const name);
        const QString &getDeviceName() {return deviceName;}
        void setTimeout(int milli);
        bool setBaudRate(BaudRate rate);
        void setDtr(bool v);

    private:
        ESerialPort();
        ESerialPort(QObject *parent);
        void initialize();
        bool nativeSetBaudRate(BaudRate rate);
        int nativeBaudRate(BaudRate rate);

        QString deviceName;
        int timeout;
        BaudRate baudRate;

        ESP_HANDLE fd;
    };
}
#endif // ESERIALPORT_H

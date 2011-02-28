#ifndef PROPELLERLOADER_H
#define PROPELLERLOADER_H

#include "eserialport.h"

#include <QIODevice>
#include <QProgressBar>
#include <QThread>
#include <QLabel>
#include <QTime>
#include <stdint.h>

namespace PZST {
    class PropellerLoader : public QThread
    {
        Q_OBJECT
    public:
        PropellerLoader();
        PropellerLoader(ESerialPort *device, QProgressBar* progress = 0, QLabel *label = 0);

        enum LoaderResult {
            LoaderOK = 0,
            LoaderInvalidFile,
            LoaderCommError,
            LoaderBusy
        };

        void updateFirmware(int command, QByteArray data);
        LoaderResult getResult() {return result;};

    signals:
        void setLabelText(const QString&);
        void setProgressMax(const int);
        void setProgressValue(const int);

    protected:
        virtual void run();

    private:
        ESerialPort *device;
        uint8_t LFSR;
        int command;
        QByteArray data;
        LoaderResult result;
        QTime t;


        void sendCalibration();
        uint8_t iterateLFSR();
        void sendByte(uint8_t);
        void sendLong(uint32_t);
        void sendBytes(char*, uint16_t);
        bool getReply(char*c, int timeout);
        void advance(int);

        QProgressBar* progress;
        QLabel* label;
        int progressMax;
        int progressValue;

        static const int programMilli = 5000;
        static const int verifyMilli = 2000;

    };

    class PropellerPacker
    {
    public:
        PropellerPacker();
        void pack(uint8_t bit);
        QByteArray& finalize();
    private:
        uint16_t bits;
        int bitsUsed;
        QByteArray bytes;
    };
}

#endif // PROPELLERLOADER_H

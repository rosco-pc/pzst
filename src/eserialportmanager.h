#ifndef ESERIALPORTMANAGER_H
#define ESERIALPORTMANAGER_H

#include <QString>
#include <QQueue>
#include <QHash>
#include <QMap>
#include "eserialportproxy.h"

namespace PZST {

class ESerialPortManager
{
public:
    static ESerialPortProxy* obtain(QString name);
    static bool grab(ESerialPortProxy *proxy);
    static bool release(ESerialPortProxy *proxy);
private:
    ESerialPortManager();
    static QMap<QString, ESerialPort*> ports;
    static QHash<ESerialPort*, QQueue<ESerialPortProxy*> > owners;
};

};

#endif // ESERIALPORTMANAGER_H

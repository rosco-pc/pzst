#include "eserialportmanager.h"

namespace PZST {

QMap<QString, ESerialPort*> ESerialPortManager::ports;
QHash<ESerialPort*, QQueue<ESerialPortProxy*> > ESerialPortManager::owners;

ESerialPortManager::ESerialPortManager()
{
}

ESerialPortProxy* ESerialPortManager::obtain(QString name)
{
    ESerialPort* port = ports[name];
    if (!port) {
        port = new ESerialPort();
        port->setDeviceName(name);
        ports[name] = port;
    }
    return new ESerialPortProxy(port);
}

bool ESerialPortManager::grab(ESerialPortProxy *proxy)
{
    QQueue<ESerialPortProxy*> & q = owners[proxy->getPort()];
    q.removeAll(proxy);
    if (!q.isEmpty()) {
        ESerialPortProxy* last = q.dequeue();
        last->isOwned = false;
        q.enqueue(last);
    }
    proxy->isOwned = true;
    q.enqueue(proxy);
    return true;
}

bool ESerialPortManager::release(ESerialPortProxy *proxy)
{
    QQueue<ESerialPortProxy*> & q = owners[proxy->getPort()];
    q.removeAll(proxy);
    proxy->isOwned = false;
    if (!q.isEmpty()) {
        ESerialPortProxy* last = q.dequeue();
        last->isOwned = true;
        last->restoreState();
        q.enqueue(last);
    }
    return true;
}

};

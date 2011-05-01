#include "eserialport.h"
#include <QStringList>
#include <QFile>

using namespace PZST;

QStringList ESerialPort::enumeratePorts()
{
    QStringList ports;
    QStringList templates("/dev/ttyS%1");
    templates << "/dev/ttyUSB%1";
    for (int i = 0; i < templates.size(); i++) {
        QString tpl = templates[i];
        for (int j = 0; j < 30; j++) {
            QString fName = tpl.arg(j);
            if (QFile::exists(fName)) ports << fName;
        }
    }
    return ports;
}


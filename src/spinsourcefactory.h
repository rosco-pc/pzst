#ifndef SPINSOURCEFACTORY_H
#define SPINSOURCEFACTORY_H

#include <QObject>
#include <QMap>
#include "spincodeparser.h"

namespace PZST {
class SpinSourceFactory : public QObject
{
Q_OBJECT
public:
    static SpinSourceFactory* instance();
    QString getSource(QString fileName);
    SpinCodeParser* getParser(QString fileName);
private:
    explicit SpinSourceFactory(QObject *parent = 0);
    QMap<QString, QString> sourceTexts;
    QMap<QString, SpinCodeParser*> parsers;
signals:
public slots:
    void addSource(QString fileName, QString text);
    void removeSource(QString fileName);
};

}
#endif // SPINSOURCEFACTORY_H

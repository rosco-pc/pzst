#ifndef SPINSOURCEFACTORY_H
#define SPINSOURCEFACTORY_H

#include <QObject>
#include <QMap>
#include <QFileSystemWatcher>
#include "spincodeparser.h"

namespace PZST {
class SpinSourceFactory : public QObject
{
Q_OBJECT
public:
    static SpinSourceFactory* instance();
    static void shutdown();
    QString getSource(QString fileName);
    SpinCodeParser* getParser(QString fileName);
private:
    explicit SpinSourceFactory(QObject *parent = 0);
    QString reload(QString fileName);
    QMap<QString, QString> sourceTexts;
    QMap<QString, SpinCodeParser*> parsers;
    QStringList dirty;
    QFileSystemWatcher *watcher;
signals:
    void extrnallyModified(QString fileName);
public slots:
    void addSource(QString fileName, QString text);
    void removeSource(QString fileName);
private slots:
    void fileChanged(QString fileName);
};

}
#endif // SPINSOURCEFACTORY_H

#include "spinsourcefactory.h"
#include "filenameresolver.h"
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QFileInfo>

using namespace PZST;

SpinSourceFactory::SpinSourceFactory(QObject *parent) :
    QObject(parent)
{
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
}

void SpinSourceFactory::addSource(QString fileName, QString text)
{
    fileName = FilenameResolver::resolve(fileName, "spin");
    sourceTexts[fileName] = text;
    if (!parsers.contains(fileName)) {
        parsers[fileName] = new SpinCodeParser;
        if (watcher) {
            QFileInfo info(fileName);
            if (info.exists()) {
                watcher->addPath(fileName);
            }
        }
    }
    parsers[fileName]->invalidate();
}

void SpinSourceFactory::removeSource(QString fileName)
{
    fileName = FilenameResolver::resolve(fileName, "spin");
    if (watcher) watcher->removePath(fileName);
    sourceTexts.remove(fileName);
    if (parsers.contains(fileName)) {
        delete parsers[fileName];
        parsers.remove(fileName);
    }
}

SpinSourceFactory* SpinSourceFactory::instance()
{
    static SpinSourceFactory instance;
    return &instance;
}

QString SpinSourceFactory::getSource(QString fileName)
{
    fileName = FilenameResolver::resolve(fileName, "spin");
    if (sourceTexts.contains(fileName)) return sourceTexts[fileName];
    QString ret = reload(fileName);
    if (watcher) {
        QFileInfo info(fileName);
        if (info.exists()) {
            watcher->addPath(fileName);
        }
    }
    return ret;
}

void SpinSourceFactory::shutdown()
{
    delete instance()->watcher;
    instance()->watcher = 0;
}

SpinCodeParser* SpinSourceFactory::getParser(QString fileName)
{
    fileName = FilenameResolver::resolve(fileName, "spin");
    if (!parsers.contains(fileName)) {
        SpinCodeParser *parser = new SpinCodeParser();
        QString source = SpinSourceFactory::getSource(fileName);
        parser->parseCode(source);
        parsers[fileName] = parser;
    }
    return parsers[fileName];
}

QString SpinSourceFactory::reload(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return "";
    QTextStream in(&file);
    QTextCodec *codec = QTextCodec::codecForName("UTF8");
    in.setCodec(codec);
    QString ret = in.readAll().replace("\r\n", "\n").replace("\r", "\n");
    sourceTexts[fileName] = ret;
    return ret;
}

void SpinSourceFactory::fileChanged(QString fileName)
{
    emit extrnallyModified(fileName);
    if (watcher) {
        watcher->removePath(fileName);
        watcher->addPath(fileName);
    }
}


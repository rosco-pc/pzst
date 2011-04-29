#include "spinsourcefactory.h"
#include "filenameresolver.h"
#include <QFile>
#include <QTextCodec>
#include <QTextStream>

using namespace PZST;

SpinSourceFactory::SpinSourceFactory(QObject *parent) :
    QObject(parent)
{
}

void SpinSourceFactory::addSource(QString fileName, QString text)
{
    fileName = FilenameResolver::resolve(fileName, "spin");
    sourceTexts[fileName] = text;
    if (!parsers.contains(fileName)) parsers[fileName] = new SpinCodeParser;
    parsers[fileName]->parseCode(text);
}

void SpinSourceFactory::removeSource(QString fileName)
{
    fileName = FilenameResolver::resolve(fileName, "spin");
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
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return "";
    QTextStream in(&file);
    QTextCodec *codec = QTextCodec::codecForName("UTF8");
    in.setCodec(codec);
    QString ret = in.readAll().replace("\r\n", "\n").replace("\r", "\n");
    sourceTexts[fileName] = ret;
    return ret;
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

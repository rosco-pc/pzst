#include "spinpreprocessor.h"
#include "spincodelexer.h"
#include "spinsourcefactory.h"
#include "filenameresolver.h"
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QRegExp>
#include <QQueue>
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>

using namespace PZST;

SpinPreprocessor::SpinPreprocessor(QString topFile, QString encoding) :
    topFile(topFile), encoding(encoding)
{
    QFileInfo info(topFile);
    topDir = info.absolutePath();
}
void SpinPreprocessor::setSearchPath(QStringList path)
{
    searchPath = path;
}
PreprocessedFiles SpinPreprocessor::findFiles()
{
    QStringList newObjects, allObjects, dataFiles;
    PreprocessedFiles files;
    QQueue<QString> queue;
    queue.enqueue(topFile);
    while (!queue.isEmpty()) {
        QString fileName = queue.dequeue();
        newObjects.clear();
        dataFiles.clear();
        PreprocessedFile p;
        p.fileName = fileName;
        p.data = findObjects(fileName, allObjects, newObjects, dataFiles);
        for (int i = 0; i < dataFiles.size(); i++) {
            PreprocessedFile p;
            QString fileName = FilenameResolver::resolve(dataFiles[i], "", topDir);
            p.fileName = dataFiles[i];
            QFile f(fileName);
            if (f.open(QIODevice::ReadOnly)) {
                p.data = f.readAll();
                files.append(p);
            }
        }
        if (!p.data.isNull()) {
            files.append(p);
        }
        for (int i = 0; i < newObjects.size(); i++) {
            QString newFile = FilenameResolver::resolve(newObjects[i], "spin", topDir);
            queue.enqueue(newFile);
        }
        allObjects << newObjects;
    }
    return files;
}
#define GET_POS(pattern, var, delta) {\
if (pos > var) { \
        char *c = ::strstr(source + pos, pattern);\
        if (!c) var = max + delta;\
        else var = c - source; \
   } \
}
QByteArray SpinPreprocessor::findObjects(QString fileName, QStringList existing, QStringList &newObjects, QStringList &dataFiles)
{
    enum State {
        Initial = 0,
        InObj,
        InFile,
    };
    State state = Initial;
    QByteArray ret;
    QString source = SpinSourceFactory::instance()->getSource(fileName);
    if (source.isEmpty()) return ret;
    QByteArray bytes = source.toAscii();
    bytes.append((char)0);
    char *start = bytes.data();
    char *firstChar = start;
    char *end = start + bytes.size() - 1;
    while (start < end) {
        char *next;
        SpinCodeLexer::Retval token = SpinCodeLexer::scan(start, end, &next);
        if (token == SpinCodeLexer::EOI) break;
        if (state == Initial) {
            switch (token) {
            case SpinCodeLexer::OBJ:
                state = InObj;
                ret.append(start, next - start);
                break;
            case SpinCodeLexer::FILE:
                state = InFile;
                ret.append(start, next - start);
                break;
            case SpinCodeLexer::STRING: {
                QString str = encodeChars(source.mid(start - firstChar + 1, next - start -2));
                ret.append(str);
                break;
            }
            default:
                ret.append(start, next - start);
                break;
            }
        } else if (state == InObj) {
            switch (token) {
            case SpinCodeLexer::DAT:
            case SpinCodeLexer::PUB:
            case SpinCodeLexer::PRI:
            case SpinCodeLexer::CON:
            case SpinCodeLexer::VAR:
                state = Initial;
                ret.append(start, next - start);
                break;
            case SpinCodeLexer::STRING: {
                QString name = source.mid(start - firstChar + 1, next - start - 2).toLower();
                if (!existing.contains(name) && !newObjects.contains(name)) newObjects.append(name);
                ret.append(start, next - start);
                }
                break;
            default:
                ret.append(start, next - start);
                break;
            }
        } else if (state == InFile) {
            switch (token) {
            case SpinCodeLexer::WHITESPACE:
            case SpinCodeLexer::COMMENT:
            case SpinCodeLexer::CHAR:
                ret.append(start, next - start);
                break;
            case SpinCodeLexer::STRING: {
                QString name = source.mid(start - firstChar + 1, next - start -2);
                if (!existing.contains(name) && !dataFiles.contains(name)) dataFiles.append(name);
                ret.append(start, next - start);
                break;
            }
            default:
                ret.append(start, next - start);
                state = Initial;
                break;
            }
        }
        start = next;
    }
    return ret;
}

QString SpinPreprocessor::encodeChars(QString str)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.toAscii().data());
    QString ret;
    QString pattern("$%1");

    QByteArray bytes = codec->fromUnicode(str);
    for (int i = 0; i < bytes.size(); i++) {
        if (!ret.isEmpty()) ret += ", ";
        ret += pattern.arg((unsigned char)bytes.at(i), 0, 16);
    }
    return ret;
}

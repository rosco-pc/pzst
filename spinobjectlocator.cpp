#include "spinobjectlocator.h"
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QRegExp>
#include <QQueue>
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>

using namespace PZST;

SpinObjectLocator::SpinObjectLocator(QString topFile, QString encoding) :
    topFile(topFile), encoding(encoding)
{
    QFileInfo info(topFile);
    topDir = info.absolutePath();
}
void SpinObjectLocator::setSearchPath(QStringList path)
{
    searchPath = path;
}
PreprocessedFiles SpinObjectLocator::findFiles()
{
    QStringList newObjects, allObjects;
    PreprocessedFiles files;
    QQueue<QString> queue;
    queue.enqueue(topFile);
    while (!queue.isEmpty()) {
        QString fileName = queue.dequeue();
        newObjects.clear();
        PreprocessedFile p;
        p.fileName = fileName;
        p.data = findObjects(fileName, allObjects, newObjects);
        if (!p.data.isNull()) {
            files.append(p);
        }
        for (int i = 0; i < newObjects.size(); i++) {
            QString newFile = objNameToFileName(newObjects[i]);
            queue.enqueue(newFile);
        }
        allObjects << newObjects;
    }
    return files;
}
QString SpinObjectLocator::objNameToFileName(QString name)
{
    QStringList paths;
    paths << topDir;
    paths << searchPath;
    for (int i = 0; i < paths.size(); i++) {
        QString path = paths[i];
        QDir dir(path, "*.spin");
        QStringList entries = dir.entryList();
        for (int j = 0; j < entries.size(); j++) {
            QString dirEntry = entries.at(j);
            if (dirEntry.toLower() == name.toLower() + ".spin") {
                return path + QDir::separator() + dirEntry;
            }
        }
    }
    return name + ".spin";;
}
#define GET_POS(pattern, var, delta) {\
if (pos > var) { \
        char *c = ::strstr(source + pos, pattern);\
        if (!c) var = max + delta;\
        else var = c - source; \
   } \
}
QByteArray SpinObjectLocator::findObjects(QString fileName, QStringList existing, QStringList &newObjects)
{
    QByteArray ret;
    char NL[] = "\n";
    char QUOTE[] = "\"";
    char COMMENT[] = "'";
    char MLCOMMENT[] = "{";
    char MLCOMMENT2[] = "{{";
    char MLCOMMENTEND[] = "}";
    char MLCOMMENT2END[] = "}}";
    char OBJ[] = "\nobj";
    char PUB[] = "\npub";
    char PRI[] = "\npri";
    char DAT[] = "\ndat";
    char VAR[] = "\nvar";
    char CON[] = "\ncon";

    QString original;
    QByteArray originalASCII;
    char *source;
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) return ret;
        QTextStream in(&file);
        QTextCodec *codec = QTextCodec::codecForName("UTF8");
        in.setCodec(codec);
        original = QString("\n").append(in.readAll());
        originalASCII = original.toLower().toAscii();
        source = originalASCII.data();
    }
    ret.reserve(originalASCII.size());
    int max = ::strlen(source);
    int pos = 0;
    int state = Initial;
    int appendFrom = 0;
    int posOBJ = -1;
    int posQuote = -1;
    int posPUB = -1;
    int posPRI = -1;
    int posVAR = -1;
    int posDAT = -1;
    int posCON = -1;
    int posComment = -1;
    int posMLComment = -1;
    int posMLComment2 = -1;
    int posNL = -1;
    int posMLCOMMENTEND = -1;
    int posMLCOMMENT2END = -1;
    while (pos < max) {
        switch(state & StateMask) {
        case Initial: {
            //if (state & InObj) {
                GET_POS(PUB, posPUB, 0);
                GET_POS(PRI, posPRI, 0);
                GET_POS(VAR, posVAR, 0);
                GET_POS(DAT, posDAT, 0);
                GET_POS(CON, posCON, 0);
            //} else {
                GET_POS(OBJ, posOBJ, 0);
            //}
            GET_POS(QUOTE, posQuote, 0);
            GET_POS(COMMENT, posComment, 0);
            GET_POS(MLCOMMENT, posMLComment, 0);
            GET_POS(MLCOMMENT2, posMLComment2, 0);
            int index = Min(posQuote, Min(posComment, Min(posMLComment, Min(posMLComment2, posOBJ))));
            index = Min(index, Min(posPUB, Min(posPRI, Min(posVAR, Min(posDAT, posCON)))));

            if (index >= max || index < 0) {
                pos = max;
                break;
            }

            if (posQuote == index) {
                state = (state & ~StateMask) | InString;
                pos = index + 1;
                posQuote = -1;
            } else if (posComment == index) {
                state = (state & ~StateMask) | InComment;
                pos = index + 1;
                posComment = -1;
            } else if (posMLComment2 == index) {
                state = (state & ~StateMask) | InMLComment2;
                pos = index + 2;
                posMLComment2 = -1;
            } else if (posMLComment == index) {
                state = (state & ~StateMask) | InMLComment;
                pos = index + 1;
                posMLComment = -1;
            } else if (posOBJ == index) {
                state = InObj;
                pos = index + 4;
                posOBJ = -1;
            } else if (
                       posPUB == index
                     || posPRI == index
                     || posVAR == index
                     || posDAT == index
                     || posCON == index
                    ) {
                state = 0;
                pos = index + 4;
                posPUB = posPRI = posVAR = posDAT = posCON = -1;
            }
            break;
        }
        case InComment: {
            GET_POS(NL, posNL, 0);
            state = state & ~StateMask;
            pos = posNL;
            break;
        }
        case InMLComment: {
            GET_POS(MLCOMMENTEND, posMLCOMMENTEND, -1);
            state = state & ~StateMask;
            pos = posMLCOMMENTEND + 1;
            break;
        }
        case InMLComment2: {
            GET_POS(MLCOMMENT2END, posMLCOMMENT2END, -2);
            state = state & ~StateMask;
            pos = posMLCOMMENT2END + 2;
            break;
        }
        case InString: {
            char *c = ::strstr(source + pos, QUOTE);
            posQuote = c - source;
            if (!c) {
                posQuote = max;
            } else if (state & InObj) {
                QString name = QString(QByteArray(source + pos, posQuote - pos)).toLower();
                if (!existing.contains(name) && !newObjects.contains(name)) newObjects.append(name);
            } else {
                ret.append(source + appendFrom, pos - appendFrom - 1);
                ret.append(encodeChars(original.mid(pos, posQuote- pos)));
                appendFrom = posQuote + 1;
            }
            state = state & ~StateMask;
            pos = posQuote + 1;
            break;
        }


        }
    }
    if (appendFrom < max) {
        ret.append(source + appendFrom, max - appendFrom);
    }
    return ret.right(ret.size()-1);
}
QString SpinObjectLocator::encodeChars(QString str)
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

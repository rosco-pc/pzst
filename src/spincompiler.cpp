#include <QtGlobal>
#include <QDir>
#include <QCoreApplication>
#include <QRegExp>
#include <QTextCodec>
#include <QTextStream>

#include "spincompiler.h"
#include "spinpreprocessor.h"
#include "pzstpreferences.h"
#include "filenameresolver.h"

using namespace PZST;

SpinCompiler::SpinCompiler() : QThread(), bstc(0)
{
}

SpinCompiler::~SpinCompiler()
{
    if (bstc) {
        bstc->terminate();
        delete bstc;
    }
}

CompilerTempFile::CompilerTempFile()
{
    ok = true;
    for (int i = 0; i < 1000; i ++) {
        tmpName = QDir::toNativeSeparators((QDir::tempPath() + "/pzst_%1_%2/").arg(QCoreApplication::applicationPid()).arg(i));
        QDir dir;
        if (!dir.exists(tmpName)) {
            if (!dir.mkpath(tmpName)) {
                ok = false;
                return;
            } else {
                break;
            }
        }
    }
}

CompilerTempFile::~CompilerTempFile()
{
    QDir dir(tmpName);
    QFileInfoList files = dir.entryInfoList();
    for (int i = 0; i < files.size(); i++) {
        QFileInfo info = files.at(i);
        if (info.isFile()) {
            QFile::remove(info.absoluteFilePath());
        }
    }
    dir.rmdir(tmpName);
}

void SpinCompiler::compile(QString srcFileName, CompileDestination dest)
{
    this->srcFileName = srcFileName;
    this->dest = dest;
    start();
}

void SpinCompiler::run()
{
    Preferences pref;
    errors.clear();
    QFile sf;
    QFile df;

    QString tmpName;
    CompilerTempFile tmpFile;
    if (!tmpFile.status()) {
        SpinError err(tr("Cannot create temp directory %1").arg(tmpName), "", -1, -1);
        err.severity = SpinError::Error;
        errors.append(err);
        status = CompileError;
        return;
    }
    tmpName = tmpFile.name();

    QString topFile;
    SpinPreprocessor prepro(srcFileName, pref.getEncoding());
    prepro.setSearchPath(pref.getSearchPath());
    PreprocessedFiles files = prepro.findFiles();
    for (int i = 0; i < files.size(); i++) {
        QString src = files.at(i).fileName;
        QFileInfo info(src);
        QString dst = tmpName + info.fileName();
        if (i == 0) topFile = dst;
        df.setFileName(dst);
        df.open(QIODevice::WriteOnly);
        df.write(files.at(i).data);
        sf.close();
        df.close();
    }

    binaryFileContent.clear();
    eepromFileContent.clear();
    listing = "";
    bstcOutput.clear();
    QString baseName;
    QStringList args;
    QFileInfo info(srcFileName);
    baseName = QDir::toNativeSeparators(tmpName + info.completeBaseName());

    if (pref.getInfo()) args << "-w2";
    else if (pref.getWarnings()) args << "-w1";
    else args << "-w0";

    QString opt("");
    if (pref.getFastConstants()) opt += "b";
    if (pref.getFoldConstants()) opt += "c";
    if (pref.getGenericSafe()) opt += "g";
    if (pref.getEliminateUnused()) opt += "r";
    if (pref.getFoldUnary()) opt += "u";
    if (pref.getNonParallaxExt()) opt += "x";

    if (!opt.isEmpty()) args << (QString("-O") + opt);

    QStringList paths = pref.getSearchPath();
    for (int i=0; i < paths.size(); i++) {
        args << "-L" << paths[i];
    }

    args << "-q";
    if (dest & EEPROM) args << "-e";
    args << "-b";
    args << "-lm";
    args << topFile;
    bstc = runCompiler(tmpName, args);
    if (!bstc) return;
    while (bstc->bytesAvailable()) bstcOutput.append(bstc->readLine());
    parseOutput();
    qSort(errors);
    if (bstc->exitCode()) {
        status = CompileError;
        return;
    }
    if (dest & EEPROM) {
        QFile f(baseName + ".eeprom");
        f.open(QIODevice::ReadOnly);
        eepromFileContent = f.readAll();
    }
    {
        QFile f(baseName + ".binary");
        f.open(QIODevice::ReadOnly);
        binaryFileContent = f.readAll();
    }
    QFile f(baseName + ".list");
    f.open(QIODevice::ReadOnly);
    listing = QString(f.readAll());
    f.close();
    parseListFile();
}

void SpinCompiler::parseOutput()
{
    QRegExp re("(.*):(\\d+):(\\d+): (Error|Warning|Information).*");

    status = OK;
    int max = 0;
    for (int i = 0; i < bstcOutput.size(); i++) {
        QString s = bstcOutput.at(i);
        if (s.startsWith("An unhandled exception occurred")) {
            SpinError err(tr("bstc compiler bug"), "", -1, -1);
            err.severity = SpinError::Error;
            errors.append(err);
            status = CompileError;
            return;
        } else if (re.indexIn(s) == 0) {
            SpinError err(s.trimmed(),
                objFileName(re.capturedTexts().at(1)),
                re.capturedTexts().at(2).toUInt(),
                re.capturedTexts().at(3).toUInt()
            );
            if (re.capturedTexts().at(4) == "Error") {
                max = 3;
                err.severity = SpinError::Error;
            } else if (re.capturedTexts().at(4) == "Warning") {
                if (max < 2) max = 2;
                err.severity = SpinError::Warning;
            } else if (re.capturedTexts().at(4) == "Information") {
                if (max < 1) max = 1;
                err.severity = SpinError::Info;
            }
            errors.append(err);
        } else {
            SpinError err(s.trimmed(), "", -1, -1);
            err.severity = SpinError::Error;
            errors.append(err);
            status = CompileError;
            return;
        }
        switch (max) {
        case 0: status = OK; break;
        case 1: status = CompileInfo; break;
        case 2: status = CompileWarning; break;
        case 3: status = CompileError; break;
        }
    }
}

QString SpinCompiler::objFileName(QString f)
{
    if (f.isEmpty()) return f;
    QFileInfo info(srcFileName);
    QString path = info.canonicalPath();

    if (!path.isEmpty()) path = path + "/";
    path = QDir::toNativeSeparators(path + f);
    if (QFile::exists(path)) return path;

    Preferences pref;
    QStringList paths = pref.getSearchPath();
    for (int i=0; i < paths.size(); i++) {
        path = QDir::toNativeSeparators(paths[i] + "/" + f);
        if (QFile::exists(path)) return path;
    }
    return QDir::toNativeSeparators(f);
}

const SpinErrors &SpinCompiler::getErrors()
{
    return errors;
}

const QByteArray& SpinCompiler::eeprom()
{
    return eepromFileContent;
}

const QByteArray& SpinCompiler::binary()
{
    return binaryFileContent;
}

SpinCodeInfo SpinCompiler::parseListFile()
{
    SpinCodeInfo ret;
    QRegExp partPattern("Binary Image Information.*(PBASE.*DCURR : ....)");
    if (partPattern.indexIn(listing) > 0) {
        QRegExp pattern("PBASE : (....).*VBASE : (....).*DBASE : (....).*PCURR : (....).*DCURR : (....).*");
        QString data = partPattern.capturedTexts().at(1);
        if (pattern.indexIn(data) >= 0) {
            int pBase = pattern.capturedTexts().at(1).toInt(0, 16);
            int vBase = pattern.capturedTexts().at(2).toInt(0, 16);
            int dBase = pattern.capturedTexts().at(3).toInt(0, 16);

            int codeSize = vBase - pBase;
            int varSize = dBase - vBase - 8;
            int stackSize = 32768 - dBase;

            int clkFreq = (quint8)binaryFileContent.at(0)
                          + ((quint8)binaryFileContent.at(1) << 8)
                          + ((quint8)binaryFileContent.at(2) << 16)
                          + ((quint8)binaryFileContent.at(3) << 24)
                          ;

            QString clkModeStr;
            int clkMode = (quint8)binaryFileContent.at(4)
                          + ((quint8)binaryFileContent.at(5) << 8)
                          + ((quint8)binaryFileContent.at(6) << 16)
                          + ((quint8)binaryFileContent.at(7) << 24)
                          ;
            if ((clkMode & (3 << 5)) == 0) {
                if ((clkMode & 7) == 0) {
                    clkModeStr = "RCFAST";
                } else {
                    clkModeStr = "RCSLOW";
                }
            } else {
                switch (((clkMode >> 3) & 3)) {
                case 0:
                    clkModeStr = "XINPUT";
                    break;
                case 1:
                    clkModeStr = "XTAL1";
                    break;
                case 2:
                    clkModeStr = "XTAL2";
                    break;
                case 3:
                    clkModeStr = "XTAL3";
                    break;
                }
                switch (clkMode & 7) {
                case 3:
                    clkModeStr += " + PLL1X";
                    break;
                case 4:
                    clkModeStr += " + PLL2X";
                    break;
                case 5:
                    clkModeStr += " + PLL4X";
                    break;
                case 6:
                    clkModeStr += " + PLL8X";
                    break;
                case 7:
                    clkModeStr += " + PLL16X";
                    break;
                }

            }
            ret.clkFreq = clkFreq;
            ret.codeSize = codeSize;
            ret.stackSize = stackSize;
            ret.varSize = varSize;
            ret.clkMode = clkModeStr;
        }
    }
    return ret;
}

QProcess* SpinCompiler::runCompiler(const QString &workingDir, const QStringList &args)
{
    QProcess* compiler = new QProcess();
    compiler->setWorkingDirectory(workingDir);
    compiler->start("bstc", args);
    if (!compiler->waitForStarted(5000)) {
#ifndef Q_OS_WIN32
#ifndef Q_OS_MAC
        QString altExe = "bstc.linux";
#else
        QString altExe = "bstc.osx";
#endif
        compiler->terminate();
        delete compiler;
        compiler = new QProcess();
        compiler->setWorkingDirectory(workingDir);
        compiler->start(altExe, args);
        if (!compiler->waitForStarted(5000)) {
#endif
        compiler->terminate();
        delete compiler;
        status = StartError;
        return 0;
#ifndef Q_OS_WIN32
    }
#endif
    }
    if (!compiler->waitForFinished(10000)) {
        compiler->terminate();
        delete compiler;
        status = StartError;
        return 0;
    }
    return compiler;
}

#ifndef SPINCOMPILER_H
#define SPINCOMPILER_H

#include <QThread>
#include <QStringList>
#include <QList>
#include <QProcess>

namespace PZST {
    struct SpinError
    {
        SpinError(QString msg, QString f, int l, int c);
        SpinError();
        QString message;
        QString filename;
        int line, col;
    };

    struct SpinCodeInfo
    {
        int codeSize;
        int varSize;
        int stackSize;
        int clkFreq;
        QString clkMode;
    };

    typedef QList<SpinError> SpinErrors;

    class CompilerTempFile
    {
    public:
        CompilerTempFile();
        ~CompilerTempFile();
        bool status() {return ok;};
        QString name() {return tmpName;};
    private:
        bool ok;
        QString tmpName;
    };

    class SpinCompiler : public QThread
    {
        Q_OBJECT
    public:
        enum CompileDestination {
            BINARY  = 1,
            EEPROM  = 2,
            LIST    = 4,
            TEMP    = 8,
        };

        enum Status {
            OK,
            StartError,
            CompileError,
            CompileWarning,
            CompileInfo,
        };

        SpinCompiler();
        virtual ~SpinCompiler();

        void compile(QString srcFileName, CompileDestination dest);
        Status getStatus() {return status;}
        const SpinErrors &getErrors();
        const QByteArray &binary();
        const QByteArray &eeprom();
        SpinCodeInfo parseListFile();

    private:
        Status status;
        QString srcFileName;
        QString listing;
        CompileDestination dest;
        QByteArray binaryFileContent;
        QByteArray eepromFileContent;
        QStringList bstcOutput;
        SpinErrors errors;
        QProcess *runCompiler(const QString &workingDir, const QStringList &args);
        void parseOutput();
        QString objFileName(QString f);
        QProcess *bstc;

    protected:
        virtual void run();

    };
}
#endif // SPINCOMPILER_H

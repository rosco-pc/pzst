#ifndef SPINOBJECTLOCATOR_H
#define SPINOBJECTLOCATOR_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QTime>



namespace PZST {
    struct PreprocessedFile
    {
        QString fileName;
        QByteArray data;
    };

    typedef QList<PreprocessedFile> PreprocessedFiles;

    class SpinPreprocessor
    {
    public:
        SpinPreprocessor(QString, QString);
        void setSearchPath(QStringList);
        PreprocessedFiles findFiles();
    private:

        static const int Initial        = 0;
        static const int InComment      = 1;
        static const int InMLComment    = 2;
        static const int InMLComment2   = 3;
        static const int InString       = 4;
        static const int StateMask      = 511;
        static const int InObj          = 512;

        QStringList searchPath;
        QString topFile;
        QString topDir;

        QByteArray findObjects(QString fileName, QStringList existing, QStringList &newObjects);
        QString encodeChars(QString str);
        QString encoding;

        static inline int Min(int a, int b) {if (a <0) return b; if (b <0) return a; if (a < b) return a; return b;};
    };
}

#endif // SPINOBJECTLOCATOR_H

#include <QtGui/QApplication>
#include <QTextCodec>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QFileInfo>
#include <QDir>
#include <QIcon>
#include "mainwindow.h"
#include "pzstpreferences.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Files/icon.ico"));

    QCoreApplication::setOrganizationName("Propeller Zone");
    QCoreApplication::setOrganizationDomain("propellerzone.ru");
    QCoreApplication::setApplicationName("PZST");

    PZST::Preferences pref;

    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QString locale = "C";
    switch (pref.getLanguage()) {
        case 1:
            locale = "ru_RU";
            break;
    }

    if (locale != "C") {
        QTranslator *systemTranslator, *appTranslator;
        systemTranslator = new QTranslator;
        appTranslator = new QTranslator;
        QString baseName = "pzst_" + locale;
        QFileInfo fileInfo(QApplication::applicationFilePath());
        QStringList paths;
        paths << QDir::homePath() + QDir::separator() + ".pzst" + QDir::separator() + "lang";
        paths << fileInfo.dir().absolutePath();
        paths << "/usr/share/pzst/lang";
        foreach (QString path, paths) {
            QString fileName = path + QDir::separator() + baseName;
            if (appTranslator->load(fileName)) {
                break;
            }
        }
        a.installTranslator(appTranslator);
        systemTranslator->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        a.installTranslator(systemTranslator);
    }


    PZST::MainWindow w;
    w.show();
    return a.exec();
}

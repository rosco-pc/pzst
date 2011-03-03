#include "filenameresolver.h"
#include "pzstpreferences.h"
#include <QDir>

namespace PZST {

QString FilenameResolver::resolve(QString filename, QString ext, QString curDir)
{
    return getInstance()->pResolve(filename, ext, curDir);
}

void FilenameResolver::setAdditionalSearchPaths(QStringList paths)
{
    getInstance()->additionalPaths = paths;
}

FilenameResolver * FilenameResolver::getInstance()
{
    static FilenameResolver instance;
    return &instance;
}

QString FilenameResolver::pResolve(QString filename, QString ext, QString curDir)
{
    Preferences pref;
    QString search = filename;
    if (!search.toLower().endsWith(ext.toLower().prepend("."))) search +=  "." + ext;
    QStringList paths;
    if (!curDir.isEmpty()) paths << curDir;
    paths << additionalPaths;
    paths << pref.getSearchPath();
    for (int i = 0; i < paths.size(); i++) {
        QString path = paths[i];
        QDir dir(path, "*", QDir::Name | QDir::IgnoreCase, QDir::Files);
        QStringList entries = dir.entryList();
        for (int j = 0; j < entries.size(); j++) {
            QString dirEntry = entries.at(j);
            if (dirEntry.toLower() == search.toLower()) {
                return QDir::toNativeSeparators(path + QDir::separator() + dirEntry);
            }
        }
    }
    return QDir::toNativeSeparators(search);
}


} // namespace PZST

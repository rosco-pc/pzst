#ifndef FILENAMERESOLVER_H
#define FILENAMERESOLVER_H

#include "pzstpreferences.h"
#include <QStringList>
#include <QMap>

namespace PZST {

class FilenameResolver
{
public:
    static QString resolve(const QString filename, const QString ext, const QString curDir = QString());
    static void setAdditionalSearchPaths(const QStringList paths = QStringList());
private:
    static FilenameResolver * getInstance();
    QStringList additionalPaths;
    QString pResolve(const QString filename, const QString ext, const QString curDir = QString());
    QStringList lastPaths;
    QMap<QString, QString> cache;
};

} // namespace PZST

#endif // FILENAMERESOLVER_H

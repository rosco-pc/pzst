#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QKeySequence>
#include <QMap>
#include <QList>
#include <QObject>
#include <QStringList>

namespace PZST {

class Shortcuts : public QObject
{
    Q_OBJECT
public:
    static QList<QKeySequence> defaultSequence(QString);
    static QString  title(QString);
    static QStringList  allNames();
private:
    Shortcuts();
    static void initShortcuts(QMap<QString, QList<QKeySequence> >&);
    static void initTitles(QMap<QString, QString>&);
    static QMap<QString, QString>& titles();
};

} // namespace PZST

#endif // SHORTCUTS_H


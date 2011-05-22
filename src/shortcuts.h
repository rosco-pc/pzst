#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QKeySequence>
#include <QMap>
#include <QList>
#include <QObject>
#include <QStringList>

namespace PZST {


    typedef struct {
        const char *name;
        const char *title;
        int std;
        int key;
        int sciMsg;
    } COMMAND;

class Shortcuts : public QObject
{
    Q_OBJECT
public:
    static QList<QKeySequence> defaultSequence(QString);
    static QString  title(QString);
    static QStringList  allNames();
    static int qsciCommand(QString name);
private:
    Shortcuts();
    static void initMap(QMap<QString, const COMMAND*>&);
    static const QMap<QString, const COMMAND*> &map();
};

} // namespace PZST

#endif // SHORTCUTS_H


#ifndef SEARCHABLE_H
#define SEARCHABLE_H

#include <QObject>
#include <QStringList>

namespace PZST {

    class Searchable
    {
    public:
        Searchable();
        virtual QStringList getSearchTargets() = 0;
        virtual QString getSearchTargetText(QString &target) = 0;
        virtual int getSearchTargetStartPosition(QString &target, bool backward) = 0;
        virtual void replaceInTarget(QString &target, int start, int len, QString &text) = 0;
        virtual bool supportsReplace(QString &target) = 0;
    };

} // namespace PZST

#endif // SEARCHABLE_H

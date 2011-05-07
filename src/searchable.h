#ifndef SEARCHABLE_H
#define SEARCHABLE_H

#include <QString>

namespace PZST {

    class Searchable
    {
    public:
        Searchable();
        virtual QString searchTargetId() const = 0;
        virtual QString searchTargetText() const = 0;
        virtual bool supportsReplace() const = 0;
        virtual int getStartPosition() const = 0;
        virtual void replaceInTarget(QString) {};
    };

} // namespace PZST

#endif // SEARCHABLE_H

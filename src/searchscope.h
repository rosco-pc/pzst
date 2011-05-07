#ifndef SEARCHSCOPE_H
#define SEARCHSCOPE_H

#include <QObject>
#include <QList>

#include "searchable.h"

namespace PZST {

    class SearchScope
    {
    public:
        SearchScope();
        void addSearchable(Searchable*);
        void removeSearchable(Searchable*);
        void clearSearchableList();
        const QList<Searchable*> &getTargets() const;
        virtual QString searchScopeName() const = 0;
    private:
        QList<Searchable*> targets;
    };

} // namespace PZST

#endif // SEARCHSCOPE_H

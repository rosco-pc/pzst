#include "searchscope.h"

using namespace PZST;

SearchScope::SearchScope()
{
}

void SearchScope::addSearchable(Searchable *s)
{
    if (!targets.contains(s)) targets.append(s);
}

void SearchScope::removeSearchable(Searchable *s)
{
    targets.removeAll(s);
}

const QList<Searchable*> &SearchScope::getTargets() const
{
    return targets;
}

void SearchScope::clearSearchableList()
{
    targets.clear();
}

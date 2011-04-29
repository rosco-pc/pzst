#include "searchengine.h"

#include <QRegExp>
#include "searchable.h"

namespace PZST {

SearchEngine::SearchEngine()
{
    currentTarget = NULL;
}

void SearchEngine::registerSearchable(Searchable *s)
{
    if (!s) return;
    if (registeredTargets.contains(s)) return;
    registeredTargets.append(s);
}

void SearchEngine::unregisterSearchable(Searchable *s)
{
    registeredTargets.removeAll(s);
}

void SearchEngine::setCurrentSearchTarget(Searchable *s)
{
    currentTarget = s;
}


void SearchEngine::find(QString search, SearchOptions options)
{
    QList<Searchable*> list;
    QStringList usedTargets;
    if (options & AllFiles) {
        list = registeredTargets;
    } else {
        if (currentTarget) list << currentTarget;
    }
    emit searchStarted((options & AllFiles) || (options & AllEntries));
    for (int i = 0; i < list.size(); i++) {
        findInTarget(list.at(i), search, options, usedTargets);
    }
    emit searchFinished((options & AllFiles) || (options & AllEntries));
}

void SearchEngine::findInTarget(Searchable *target, QString &search, SearchOptions options, QStringList &usedTargets)
{
    int max = 1;
    if (options & AllFiles) {
        max = target->getSearchTargets().size();
    }
    for (int i = 0; i < max; i++) {
        QString targetName = target->getSearchTargets().at(i);
        if (usedTargets.contains(targetName)) continue;
        usedTargets.append(targetName);
        int startPos = target->getSearchTargetStartPosition(targetName, (options & Backward) ? true : false);
        QString text = target->getSearchTargetText(targetName);
        if (options & AllEntries) {
            startPos = 0;
            options &= ~Wrap;
        }
        Result result;
        do {
            result = findInText(text, startPos, search, options);
            if (result.pos >= 0) {
                result.context = getContext(target, targetName, result.pos);
                emit foundInTarget(targetName, result, (options & AllFiles) || (options & AllEntries));
                if (options & AllEntries) {
                    startPos = result.pos + result.match.size();
                } else {
                    result.pos = -1;
                }
            }
        } while (result.pos >= 0);
    }
}


SearchEngine::Result SearchEngine::findInText(QString &text, int startPos, QString search, SearchOptions options)
{
    QRegExp re;
    Result res;
    res.pos = -1;
    if (!(options & RegExp)) {
        search = QRegExp::escape(search);
    }
    if (options & WholeWord) {
        search = "\\b(" + search + ")\\b";
    }
    re = QRegExp(search);
    if (!re.isValid()) return res;
    re.setCaseSensitivity(options & CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    if (options & Backward) {
        startPos--;
        if (startPos < 0 && !(options & Wrap)) {
            startPos = 0;
        }
        res.pos = text.lastIndexOf(re, startPos);
        if (res.pos < 0 && (options & Wrap)) {
            res.pos = text.lastIndexOf(re);
        }
    } else {
        res.pos = text.indexOf(re, startPos);
        if (res.pos < 0 && (options & Wrap)) {
            res.pos = text.indexOf(re);
        }
    }
    if (res.pos >= 0) {
        res.match = re.capturedTexts().at(0);
    }
    return res;
}

QString SearchEngine::getContext(Searchable *target, QString targetName, int pos)
{
    QString text = target->getSearchTargetText(targetName);
    int start = text.lastIndexOf('\n', pos);
    if (start < 0) start = 0;
    int end = text.indexOf('\n', pos);
    if (end < 0) end = text.size();
    return text.mid(start, end - start).trimmed();
}

} // namespace PZST


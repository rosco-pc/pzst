#include "searchengine.h"
#include <QStringList>
#include <QRegExp>

using namespace PZST;

SearchRequest::SearchRequest(QString q) :
        query(q), options(0)
{
}

SearchEngine::SearchEngine(QObject *parent) :
    QObject(parent)
{
}

SearchEngine & SearchEngine::instance()
{
    static SearchEngine instance;
    return instance;
}

void SearchEngine::search(const SearchRequest &request)
{
    instance().p_search(request);
}


void SearchEngine::connectInstance(const char *sig, const QObject *obj, const char * slot)
{
    connect(&instance(), sig, obj, slot);
}


void SearchEngine::findIn(Searchable *target, const SearchRequest &request)
{
    SearchRequest::Options opts = request.options;
    QString search = request.query;
    if (!(opts & SearchRequest::RegExp)) {
        search = QRegExp::escape(search);
    }
    if (opts & SearchRequest::WholeWords) {
        search = "\\b" + search + "\\b";
    }
    QRegExp re(search);
    if (!(opts & SearchRequest::CaseSensitive)) {
        re.setCaseSensitivity(opts & SearchRequest::CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    }
    int startPos = target->getStartPosition();
    bool wrap = (opts & SearchRequest::Wrap);
    bool all = (opts & SearchRequest::All);
    bool back = (opts & SearchRequest::Backwards) && !all;
    if (all) startPos = 0;
    QString text = target->searchTargetText();
    for (;back ? startPos : startPos < text.size();) {
        if (opts & SearchRequest::Replace) {
            target->replaceInTarget(request.replacement);
            text = target->searchTargetText();
            startPos = target->getStartPosition();
        }
        int pos;
        if (back) pos = text.lastIndexOf(re, startPos-1);
        else pos = text.indexOf(re, startPos);
        if (pos >= 0) {
            emit found(target, pos, re.capturedTexts()[0].size(), &request);
            if (!all) break;
            if (back) startPos =  pos;
            else startPos =  pos + re.capturedTexts()[0].size();
        } else {
            if (wrap && !all) {
                if (back)  {
                    startPos = text.size();
                } else {
                    startPos = 0;
                }
            } else {
                break;
            }
        }
    }
}

void SearchEngine::p_search(const SearchRequest &request)
{
    QStringList seen;
    emit searchStarted(&request);
    foreach (SearchScope *scope, request.scopes) {
        foreach (Searchable * s, scope->getTargets()) {
            QString targetId = s->searchTargetId();
            if (seen.contains(targetId)) continue;
            seen.append(targetId);
            findIn(s, request);
        }
    }
}

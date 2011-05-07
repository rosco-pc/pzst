#include "searchengine.h"
#include <QStringList>
#include <QRegExp>

using namespace PZST;

SearchRequest::SearchRequest(QString q) :
        query(q), options(0)
{
}

void SearchRequest::addScope(SearchScope *scope)
{
    if (!scopes.contains(scope)) scopes.append(scope);
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


int SearchEngine::findIn(Searchable *target, const SearchRequest &request)
{
    int count = 0;
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
    bool wrapped = false;
    bool all = (opts & SearchRequest::All);
    bool back = (opts & SearchRequest::Backwards) && !all;
    if (all) startPos = 0;
    QString text = target->searchTargetText();
    for (;back ? startPos || (wrap && !wrapped) : startPos <= text.size();) {
        if (opts & SearchRequest::Replace) {
            target->replaceInTarget(request.replacement);
            text = target->searchTargetText();
            if (!wrapped) startPos = target->getStartPosition();
        }
        int pos;
        if (back) pos = text.lastIndexOf(re, startPos-1);
        else pos = text.indexOf(re, startPos);
        if (pos >= 0) {
            emit found(target, pos, re.capturedTexts()[0].size(), &request);
            count ++;
            if (!all) break;
            if (back) startPos =  pos;
            else startPos =  pos + re.capturedTexts()[0].size();
        } else {
            if (wrap && !all) {
                if (wrapped) break;
                if (back)  {
                    startPos = text.size();
                } else {
                    startPos = 0;
                }
                wrapped = true;
            } else {
                break;
            }
        }
    }
    return count;
}

void SearchEngine::p_search(const SearchRequest &request)
{
    QStringList seen;
    int count = 0;
    emit searchStarted();
    emit searchStarted(&request);
    foreach (SearchScope *scope, request.scopes) {
        foreach (Searchable * s, scope->getTargets()) {
            QString targetId = s->searchTargetId();
            if (seen.contains(targetId)) continue;
            seen.append(targetId);
            count += findIn(s, request);
        }
    }
    if (!count) emit noResults();
    emit searchFinished();
}

bool SearchEngine::parseQuickSearch(QString query, SearchRequest &request)
{
    enum State {
        Initial = 0,
        Search,
        ReplaceOrOptions,
        Options
    };

    QString search, replace;
    State state = Initial;
    bool valid = true;
    bool back = false;
    bool all = false;
    bool caseSensitive = true;
    QString optionsStr;
    int i;
    for (i = 0; i < query.size(); i++) {
        QChar ch = query[i];
        switch (state) {
        case Initial:
            if (ch == '?') {
                back = true;
            } else if (ch != '/') {
                i--;
            }
            state = Search;
            break;
        case Search:
            if (ch == '\\') {
                if (i == query.size() - 1) valid = false;
                else {
                    search += '\\';
                    search += query[i+1];
                    i++;
                }
            } else if (ch == '/') {
                state = ReplaceOrOptions;
            } else {
                search += ch;
            }
            break;
        case ReplaceOrOptions:
            if (ch == '\\') {
                if (i == query.size() - 1) valid = false;
                else {
                    optionsStr += '\\';
                    optionsStr += query[i+1];
                    i++;
                }
            } else if (ch == '/') {
                replace = optionsStr;
                if (replace.isNull()) replace = "";
                optionsStr = "";
                state = Options;
            } else {
                optionsStr += ch;
            }
            break;
        case Options:
            optionsStr += ch;
            break;
        }
    }


    for (i = 0; i < optionsStr.size(); i++) {
        QChar ch = optionsStr[i];
        if (ch == 'i') {
            caseSensitive = false;
        } else if (ch == 'a') {
            all = true;
        } else {
            valid = false;
            break;
        }
    }

    if (valid) {
        QRegExp searchRE(search);
        valid = searchRE.isValid();
    }
    if (valid) {
        request.setQuery(search);
        if (!replace.isNull()) {
            request.setReplacement(replace);
            request.addOptions(SearchRequest::Replace);
        }
        if (all) request.addOptions(SearchRequest::All);
        if (back) request.addOptions(SearchRequest::Backwards);
        if (caseSensitive) request.addOptions(SearchRequest::CaseSensitive);
        request.addOptions(SearchRequest::RegExp);
        request.addOptions(SearchRequest::Wrap);
    }
    return valid;
}

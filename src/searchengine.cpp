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


int SearchEngine::findIn(Searchable *target, const SearchRequest &request, const QStringList &replacements)
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
    QRegExp re;
    if (!(opts & SearchRequest::CaseSensitive)) {
        re.setCaseSensitivity(opts & SearchRequest::CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    }
    if ((opts & SearchRequest::RegExp) && (opts & SearchRequest::Ungreedy)) {
        re.setPatternSyntax(QRegExp::RegExp);
        re.setMinimal(true);
    } else {
        re.setPatternSyntax(QRegExp::RegExp2);
        re.setMinimal(false);
    }
    int startPos = target->getStartPosition();
    bool wrap = (opts & SearchRequest::Wrap);
    bool wrapped = false;
    bool all = (opts & SearchRequest::All);
    bool back = (opts & SearchRequest::Backwards) && !all;
    if (all) startPos = 0;
    re.setPattern(search);
    QString text = target->searchTargetText();
    for (;back ? startPos || (wrap && !wrapped) : startPos <= text.size();) {
        int pos;
        if (back) pos = text.lastIndexOf(re, startPos-1);
        else pos = text.indexOf(re, startPos);
        if (pos >= 0) {
            emit found(target, pos, re.capturedTexts()[0].size(), &request);
            if (opts & SearchRequest::Replace) {
                if (request.options & SearchRequest::RegExp) {
                    target->replaceInTarget(buildReplacement(re, replacements));
                } else {
                   target->replaceInTarget(request.replacement);
                }
                text = target->searchTargetText();
                if (!wrapped) startPos = target->getStartPosition();
            }
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
    QStringList replacements;
    int count = 0;
    if ((request.options & (SearchRequest::RegExp | SearchRequest::Replace)) == (SearchRequest::RegExp | SearchRequest::Replace))
        splitReplacement(request.replacement, &replacements);
    emit searchStarted();
    emit searchStarted(&request);
    foreach (SearchScope *scope, request.scopes) {
        foreach (Searchable * s, scope->getTargets()) {
            QString targetId = s->searchTargetId();
            if (seen.contains(targetId)) continue;
            seen.append(targetId);
            count += findIn(s, request, replacements);
        }
    }
    if (!count) emit noResults();
    emit searchFinished(&request);
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
    bool greedy = true;
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
        } else if (ch == 'g') {
            all = true;
        } else if (ch == 'U') {
            greedy = false;
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
        if (!greedy) request.addOptions(SearchRequest::Ungreedy);
        request.addOptions(SearchRequest::RegExp);
        request.addOptions(SearchRequest::Wrap);
    }
    return valid;
}

QString SearchEngine::buildReplacement(const QRegExp &re, const QStringList &parts)
{
    QString ret;
    int nMatches = re.captureCount();
    // 0 - no change
    // 1 - next lower
    // 2 - lower
    // 3 - next upper
    // 4 - upper
    QString caseChars = "lLuUeE";
    int caseChange = 0;
    for (int i = 0; i < parts.size(); i++) {
        QString part = parts[i];
        if (part.size() > 1 && part[0] == '\\' && part[1].isDigit()) {
            int idx = part.right(part.size()-1).toInt();
            if (idx <= nMatches) {
                part = re.capturedTexts()[idx];
                if (part.isNull()) part = "";
            }
            else part = QString();
        } else if (part.size() == 2 && caseChars.indexOf(part[1]) >= 0) {
            if (part[1] == 'l') {
                caseChange = 1;
            } else if (part[1] == 'L') {
                caseChange = 2;
            } else if (part[1] == 'u') {
                caseChange = 3;
            } else if (part[1] == 'U') {
                caseChange = 4;
            } else if (part[1] == 'e' || part[1] == 'E') {
                caseChange = 0;
            }
            part = QString();
        }
        if (!part.isNull()) {
            switch (caseChange) {
            case 1:
                if (part.size()) {
                    part[0] = part[0].toLower();
                    caseChange = 0;
                }
                break;
            case 2:
                part = part.toLower();
                break;
            case 3:
                if (part.size()) {
                    part[0] = part[0].toUpper();
                    caseChange = 0;
                }
                break;
            case 4:
                part = part.toUpper();
                break;
            }
            ret += part;
        }
    }
    return ret;
}

bool SearchEngine::splitReplacement(const QString &pattern, QStringList *parts)
{
    QString part;
    for (int i = 0; i < pattern.size(); i++) {
        QChar c = pattern[i];
        if (c == '\\') {
            if (i >= pattern.size() - 1) return false;
            c = pattern[++i];
            if (c.isDigit()) {
                if (!part.isEmpty()) {
                    if (parts) *parts << part;
                    part = "";
                }
                part += '\\';
                while (c.isDigit()) {
                    part += c;
                    if (++i >= pattern.size()) break;
                    c = pattern[i];
                }
                --i;
                if (parts) *parts << part;
                part = "";
            } else {
                if (c == 'n') {
                    part += '\n';
                } else if (c == 't') {
                    part += '\t';
                } else if (QString("uUlLeE").indexOf(c) >= 0) {
                    if (!part.isEmpty()) {
                        if (parts) *parts << part;
                        part = "";
                    }
                    *parts << (QString("\\")  + c);
                } else {
                    part += c;
                }
            }
        } else {
            part += c;
        }
    }
    if (parts && !part.isEmpty()) *parts << part;
    return true;
}

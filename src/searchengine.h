#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include <QString>
#include "searchable.h"
#include "searchscope.h"

namespace PZST {

    class SearchRequest
    {
        friend class SearchEngine;
    public:
        enum Option {
            RegExp          = 1,
            CaseSensitive   = 2,
            Replace         = 4,
            Backwards       = 8,
            WholeWords      = 16,
            All             = 32,
            Wrap            = 64,
        };

        SearchRequest(QString q = QString());
        Q_DECLARE_FLAGS (Options, Option);

        void setOptions(Options o) {options = o;};
        void addOptions(Options o)  {options |= o;};
        void removeOptions(Options o) {options &= ~o;};
        void setQuery(QString q) {query = q;}
        void setReplacement(QString r) {replacement = r;}
        void addScope(SearchScope* scope);
        void setScopes(QList<SearchScope*> s) {scopes = s;};
        Options getOptions() const {return options;}
        const QString &getQuery() {return query;}
        const QString &getReplacement() {return replacement;}

    private:
        QString query;
        QString replacement;
        Options options;
        QList<SearchScope*> scopes;

    };
    //Q_DECLARE_OPERATORS_FOR_FLAGS (SearchRequest::Options);

    class SearchEngine : public QObject
    {
    Q_OBJECT
    public:
        explicit SearchEngine(QObject *parent = 0);
        static void search(const SearchRequest &request);
        static void connectInstance(const char*, const QObject*, const char*);
        static bool parseQuickSearch(const QString query, SearchRequest &request);

    signals:
        void searchStarted(const SearchRequest*);
        void found(Searchable*, int pos, int len, const SearchRequest*);
        void noResults();

    public slots:
    private:
        static SearchEngine &instance();
        void p_search(const SearchRequest &request);
        int findIn(Searchable *target, const SearchRequest &request);
    };

} // namespace PZST

#endif // SEARCHENGINE_H

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include <QList>


namespace PZST {

    class Searchable;

    class SearchEngine : public QObject
    {
        Q_OBJECT
    public:
        SearchEngine();
        enum SearchOption {
            CaseSensitive       = 1,
            RegExp              = 2,
            WholeWord           = 4,
            Wrap                = 8,
            Backward            = 16,
            AllEntries          = 32,
            AllFiles            = 32,
        };
        Q_DECLARE_FLAGS (SearchOptions, SearchOption);

        void setCurrentSearchTarget(Searchable*);
        void registerSearchable(Searchable*);
        void unregisterSearchable(Searchable*);
        struct Result {
            QString match;
            QString context;
            int pos;
        };
    signals:
        void searchStarted(bool allTargets);
        void foundInTarget(QString target, SearchEngine::Result res, bool allTargets);
        void searchFinished(bool allTargets);
    public slots:
        void find(QString search, SearchOptions options);
    private:

        Searchable *currentTarget;
        QList<Searchable*> registeredTargets;

        void findInTarget(Searchable *target, QString &search, SearchOptions options, QStringList &usedTargets);
        Result findInText(QString &text, int startPos, QString search, SearchOptions options);
        QString getContext(Searchable *target, QString targetName, int pos);
    };

    //Q_DECLARE_OPERATORS_FOR_FLAGS(SearchEngine::SearchOptions);
} // namespace PZST

#endif // SEARCHENGINE_H

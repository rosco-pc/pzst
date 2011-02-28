#ifndef SPINPARSER_H
#define SPINPARSER_H

#include <QString>
#include <QList>

namespace PZST {
    struct SpinMethodInfo
    {
        SpinMethodInfo(QString n, bool p, int l) {name = n; pri = p; line = l;};
        QString name;
        bool pri;
        int line;
        int lines;
    };

    inline bool operator<(SpinMethodInfo a, SpinMethodInfo b) {return a.name.toLower() < b.name.toLower();};

    typedef QList<SpinMethodInfo>  SpinMethodInfoList;

    class SpinParser
    {
    public:
        SpinParser();
        void parse(QString text);
        const SpinMethodInfoList &getMethods() {return methods;}
    private:

        enum State {
            Initial = 0,
            Comment,
            MLComment2,
            MLComment,
            Quote,
        };

        SpinMethodInfoList methods;
        int methodName(char* str);
    };
}
#endif // SPINPARSER_H

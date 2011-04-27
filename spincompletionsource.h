#ifndef SPINCOMPLETIONSOURCE_H
#define SPINCOMPLETIONSOURCE_H

#include "Qsci/qsciapis.h"


namespace PZST {
    class SpinEditor;

    class SpinCompletionSource : public QsciAbstractAPIs
{
public:
    SpinCompletionSource(QsciLexer*, SpinEditor*);
    virtual QStringList callTips(const QStringList &context, int commas,
            QsciScintilla::CallTipsStyle style,
            QList<int> &shifts);
    virtual void updateAutoCompletionList(const QStringList &context,
            QStringList &list);
private:
    SpinEditor *editor;
    void updateInMethod(const QStringList &context, QStringList &list, QStringList locals);
    void updateInCon(const QStringList &context, QStringList &list);
    void updateInDat(const QStringList &context, QStringList &list);
    void updateInObj(const QStringList &context, QStringList &list);
    void updateInVar(const QStringList &context, QStringList &list);

    void addConstants(const QStringList &context, QStringList &list, bool allowPrefix);
    void addObjects(const QStringList &context, QStringList &list, bool allowPrefix);
    void addLabels(const QStringList &context, QStringList &list, bool allowPrefix);
    void addGlobals(const QStringList &context, QStringList &list);
    void addMethods(const QStringList &context, QStringList &list);
    void addLocals(const QStringList &context, QStringList &list, QStringList locals);
    void addObjectMethods(const QStringList &context, QStringList &list);
    void addObjectConstants(const QStringList &context, QStringList &list, bool allowPrefix);
};

} // namespace PZST

#endif // SPINCOMPLETIONSOURCE_H

#ifndef SPINCODEPARSER_H
#define SPINCODEPARSER_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QVarLengthArray>

#include "spincodelexer.h"

namespace PZST {

    struct SpinVar
    {
        enum Type {
            Unknown = 0,
            Byte,
            Word,
            Long,
        };
        Type type;
        QString name;
    };

    inline bool operator<(SpinVar a, SpinVar b) {return a.name.toLower() < b.name.toLower();};
    typedef QList<SpinVar> SpinVarsList;

    struct SpinCodeSection
    {
        enum Type {
            None = 0,

        };
        Type type;
        QString name;
    };



    struct SpinCodeContext {
        enum Context {
            Unknown = 0,
            Pub,
            Pri,
            Con,
            Obj,
            Dat,
            Var,
        };
        Context ctx;
        int start;
        int end;
        QString name;
        QStringList locals;
        QStringList params;
    };
    inline bool operator<(SpinCodeContext a, SpinCodeContext b) {return a.name.toLower() < b.name.toLower();};

    class SpinObjectsList : public QMap<QString, QString> {
        public :
            QString getFileName(QString objName);
    };

    typedef QList<SpinCodeContext>  SpinContextList;

    typedef struct {
        int len;
        SpinCodeLexer::Retval style;
    } SpinHighlightInfo;

    class SpinHighlightList
    {
    public:
        SpinHighlightList();
        ~SpinHighlightList();
        int size() const {return sz;}
        void append(int len, SpinCodeLexer::Retval style);
        void clear();
        const SpinHighlightInfo *get(int idx) const;
        const SpinHighlightInfo * operator[](int idx) const {return get(idx);};
    private:
        int capacity;
        int sz;
        SpinHighlightInfo* data;
    };

    class SpinCodeParser
    {
    public:
        SpinCodeParser();
        void parseCode(const QString code);
        const SpinContextList getMethods(SpinCodeContext::Context = SpinCodeContext::Unknown);
        const SpinVarsList &getGlobals() {return spinGlobals;}
        const QStringList &getConstants() {return constants;}
        const QStringList &getAsmGlobals() {return asmGlobals;}
        const SpinObjectsList &getObjects() {return objects;}
        const  SpinHighlightList &getHighlighting() {return highlighting;}
        SpinCodeContext getContext(int pos);
        bool isValid() {return valid;}
        void invalidate() {valid = false;}
    private:
        enum State {
            Initial = 0,
            Con,
            ConValue,
            Pub,
            Pri,
            ParamsStart,
            Param,
            ParamNext,
            Local,
            LocalNext,
            Var,
            VarName,
            VarSeparator,
            Dat,
            DatCode,
            Obj,
            ObjFile,
            SkipToEOL,
        };

        QStringList constants;
        SpinVarsList spinGlobals;
        char *textStart;
        char *textEnd;
        State state;
        SpinVar curVar;
        SpinCodeContext curContext;
        SpinContextList contexts;
        SpinObjectsList objects;
        QStringList asmGlobals;
        SpinHighlightList highlighting;
        int lastSectionIndex;
        QString objName;
        State skipState;
        bool valid;

        void processToken(SpinCodeLexer::Retval token, char *text, int len);
        bool checkSectionSwitch(SpinCodeLexer::Retval token, char *text);

        void stateInitial(SpinCodeLexer::Retval token, char *text, int len);
        void stateCon(SpinCodeLexer::Retval token, char *text, int len);
        void stateConValue(SpinCodeLexer::Retval token, char *text, int len);
        void stateVar(SpinCodeLexer::Retval token, char *text, int len);
        void stateVarName(SpinCodeLexer::Retval token, char *text, int len);
        void stateVarSeparator(SpinCodeLexer::Retval token, char *text, int len);
        void stateObj(SpinCodeLexer::Retval token, char *text, int len);
        void stateObjFile(SpinCodeLexer::Retval token, char *text, int len);
        void statePub(SpinCodeLexer::Retval token, char *text, int len);
        void statePri(SpinCodeLexer::Retval token, char *text, int len);
        void stateDat(SpinCodeLexer::Retval token, char *text, int len);
        void stateDatCode(SpinCodeLexer::Retval token, char *text, int len);
        void stateParamsStart(SpinCodeLexer::Retval token, char *text, int len);
        void stateParam(SpinCodeLexer::Retval token, char *text, int len);
        void stateParamNext(SpinCodeLexer::Retval token, char *text, int len);
        void stateLocal(SpinCodeLexer::Retval token, char *text, int len);
        void stateLocalNext(SpinCodeLexer::Retval token, char *text, int len);
        void stateSkipToEOL(SpinCodeLexer::Retval token, char *text, int len);

        void statePubPri(SpinCodeLexer::Retval token, char *text, int len, bool pri);
    };

} // namespace PZST

#endif // SPINCODEPARSER_H

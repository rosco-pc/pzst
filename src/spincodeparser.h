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
        int style;
        int token;
    } SpinHighlightInfo;

    class SpinHighlightList
    {
    public:
        SpinHighlightList();
        ~SpinHighlightList();
        int size() const {return sz;}
        void append(int len, int style, int token);
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

        void processToken(int token, char *text, int len);
        bool checkSectionSwitch(int token, char *text);

        void stateInitial(int token, char *text, int len);
        void stateCon(int token, char *text, int len);
        void stateConValue(int token, char *text, int len);
        void stateVar(int token, char *text, int len);
        void stateVarName(int token, char *text, int len);
        void stateVarSeparator(int token, char *text, int len);
        void stateObj(int token, char *text, int len);
        void stateObjFile(int token, char *text, int len);
        void statePub(int token, char *text, int len);
        void statePri(int token, char *text, int len);
        void stateDat(int token, char *text, int len);
        void stateDatCode(int token, char *text, int len);
        void stateParamsStart(int token, char *text, int len);
        void stateParam(int token, char *text, int len);
        void stateParamNext(int token, char *text, int len);
        void stateLocal(int token, char *text, int len);
        void stateLocalNext(int token, char *text, int len);
        void stateSkipToEOL(int token, char *text, int len);

        void statePubPri(int token, char *text, int len, bool pri);
    };

} // namespace PZST

#endif // SPINCODEPARSER_H

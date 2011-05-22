#include "spincodeparser.h"
#include "spincodelexer.h"

#include <QByteArray>
#include <QString>

namespace PZST {

SpinCodeParser::SpinCodeParser()
{
}

void SpinCodeParser::parseCode(QString code)
{
    char *textNext;
    constants.clear();
    spinGlobals.clear();
    contexts.clear();
    asmGlobals.clear();
    highlighting.clear();
    objects.clear();
    lastSectionIndex= -1;
    QByteArray bytes(code.toUtf8());
    bytes.append((char)0);
    textStart = bytes.data();
    char * start = textStart;
    textEnd = textStart + bytes.size();
    state = Initial;
    for (;start < textEnd;) {
        SpinCodeLexer::Retval token = SpinCodeLexer::scan(start, textEnd, &textNext);
        if (token == SpinCodeLexer::EOI) break;
        highlighting.append(textNext - start, token);
        processToken(token, start, textNext - start);
        start = textNext;
    }
    if (lastSectionIndex>= 0) {
        contexts[lastSectionIndex].end = textEnd - textStart;
        lastSectionIndex = -1;
    }
    valid = true;
}

void SpinCodeParser::processToken(SpinCodeLexer::Retval token, char *text, int len)
{
    if (token == SpinCodeLexer::PREPRO) {
        skipState = state;
        state = SkipToEOL;
        return;
    }
    switch (state) {
        case Initial:
            stateInitial(token, text, len); break;
        case Con:
            stateCon(token, text, len); break;
        case ConValue:
            stateConValue(token, text, len); break;
        case Var:
            stateVar(token, text, len); break;
        case VarName:
            stateVarName(token, text, len); break;
        case VarSeparator:
            stateVarSeparator(token, text, len); break;
        case Obj:
            stateObj(token, text, len); break;
        case ObjFile:
            stateObjFile(token, text, len); break;
        case Pub:
            statePub(token, text, len); break;
        case Pri:
            statePri(token, text, len); break;
        case Dat:
            stateDat(token, text, len); break;
        case DatCode:
            stateDatCode(token, text, len); break;
        case ParamsStart:
            stateParamsStart(token, text, len); break;
        case Param:
            stateParam(token, text, len); break;
        case ParamNext:
            stateParamNext(token, text, len); break;
        case Local:
            stateLocal(token, text, len); break;
        case LocalNext:
            stateLocalNext(token, text, len); break;
        case SkipToEOL:
            stateSkipToEOL(token, text, len); break;
    }
}

void SpinCodeParser::stateInitial(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    checkSectionSwitch(token, text);
}

void SpinCodeParser::stateCon(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
        case SpinCodeLexer::IDENTIFIER: {
            QString name = QString::fromUtf8(text, len);
            if (!constants.contains(name,  Qt::CaseInsensitive)) {
                constants.append(name);
            }
            state = ConValue;
            break;
        }
        case SpinCodeLexer::RESERVED: {
            state = ConValue;
        }
        case SpinCodeLexer::CHAR: {
            if (*text == '#') state = ConValue;
        }
        default: break;
    }
}

void SpinCodeParser::stateConValue(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
        case SpinCodeLexer::CHAR: {
            if (*text == ',') state = Con;
        }
        case SpinCodeLexer::NL: {
            state = Con;
        }
        default: break;
    }
}

void SpinCodeParser::stateVar(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
        case SpinCodeLexer::TYPE: {
            QString name = QString::fromUtf8(text, len).toLower();
            if (name == "byte") curVar.type = SpinVar::Byte;
            else if (name == "word") curVar.type = SpinVar::Word;
            else if (name == "long") curVar.type = SpinVar::Long;
            else curVar.type = SpinVar::Unknown;

            state = VarName;
            break;
        }
        default: break;
    }
}

void SpinCodeParser::stateVarName(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
        case SpinCodeLexer::IDENTIFIER: {
            curVar.name = QString::fromUtf8(text, len);
            spinGlobals.append(curVar);
            state = VarSeparator;
            break;
        }
        default: break;
    }
}

void SpinCodeParser::stateVarSeparator(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
        case SpinCodeLexer::CHAR: {
            if (*text == ',') {
                state = VarName;
            }
            break;
        }
        case SpinCodeLexer::NL: {
            state = Var;
        }
        default: break;
    }
}

void SpinCodeParser::stateObj(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::IDENTIFIER: {
            objName = QString::fromUtf8(text, len);
            state = ObjFile;
            break;
        }
    default: ;
    }
}

void SpinCodeParser::stateObjFile(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::STRING: {
            QString fileName = QString::fromUtf8(text+1, len-2);
            objects[objName] = fileName;
            state = Obj;
            break;
        }
    default: ;
    }
}

void SpinCodeParser::statePub(SpinCodeLexer::Retval token, char *text, int len)
{
    statePubPri(token, text, len, false);
}

void SpinCodeParser::statePri(SpinCodeLexer::Retval token, char *text, int len)
{
    statePubPri(token, text, len, true);
}

void SpinCodeParser::statePubPri(SpinCodeLexer::Retval token, char *text, int len, bool pri)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
        case SpinCodeLexer::IDENTIFIER : {
            lastSectionIndex = contexts.size();
            curContext.name = QString::fromUtf8(text, len);
            curContext.ctx = pri ? SpinCodeContext::Pri : SpinCodeContext::Pub;
            curContext.locals.clear();
            curContext.params.clear();
            curContext.locals << "Result";
            contexts.append(curContext);
            state = ParamsStart;
        }
        default: ;
    }

}

void SpinCodeParser::stateDat(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::WHITESPACE : break;
    case SpinCodeLexer::COMMENT : break;
    case SpinCodeLexer::NL: break;
    case SpinCodeLexer::IDENTIFIER: {
            asmGlobals << QString::fromUtf8(text, len);
            state = DatCode;
            break;
        }
    default: state = DatCode;
    }
}

void SpinCodeParser::stateDatCode(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::NL: {
            state = Dat;
            break;
        }
    default: ;
    }
}

void SpinCodeParser::stateParamsStart(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::CHAR : {
            if (*text == '(') state = Param;
            if (*text == ':') state = Local;
            if (*text == '|') state = Local;
            break;
        }
    case SpinCodeLexer::NL : {
            state = Initial;
            break;
        }
    default: break;
    }
}

void SpinCodeParser::stateParam(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::IDENTIFIER: {
            contexts[lastSectionIndex].params.append(QString::fromUtf8(text, len));
            state = ParamNext;
            break;
        }
    case SpinCodeLexer::CHAR: {
            if (*text == ':') state = Local;
            if (*text == '|') state = Local;
        }
    case SpinCodeLexer::NL: {
            state = Initial;
        }
    default:break;
    }
}

void SpinCodeParser::stateParamNext(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::CHAR: {
            if (*text == ',') state = Param;
            if (*text == ':') state = Local;
            if (*text == '|') state = Local;
            break;
        }
    case SpinCodeLexer::NL: {
            state = Initial;
        }
    default:break;
    }
}
void SpinCodeParser::stateLocal(SpinCodeLexer::Retval token, char *text, int len)
{
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::IDENTIFIER: {
            contexts[lastSectionIndex].locals.append(QString::fromUtf8(text, len));
            state = LocalNext;
            break;
        }
    case SpinCodeLexer::CHAR: {
            if (*text == ':') state = Local;
            if (*text == '|') state = Local;
        }
    case SpinCodeLexer::NL: {
            state = Initial;
        }
    default:break;
    }
}

void SpinCodeParser::stateLocalNext(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::CHAR: {
            if (*text == ',') state = Local;
            if (*text == ':') state = Local;
            if (*text == '|') state = Local;
            break;
        }
    case SpinCodeLexer::NL: {
            state = Initial;
        }
    default:break;
    }
}

void SpinCodeParser::stateSkipToEOL(SpinCodeLexer::Retval token, char *text, int len)
{
    Q_UNUSED(len);
    if (checkSectionSwitch(token, text)) return;
    switch (token) {
    case SpinCodeLexer::NL: {
            state = skipState;
            break;
        }
    default:break;
    }
}

bool SpinCodeParser::checkSectionSwitch(SpinCodeLexer::Retval token, char *text)
{
    bool ret = false;
    switch (token) {
        case SpinCodeLexer::CON :
            state = Con;
            ret = true;
            break;
        case SpinCodeLexer::DAT : {state = Dat; ret = true; break;}
        case SpinCodeLexer::PUB : {state = Pub; ret = true; break;}
        case SpinCodeLexer::PRI : {state = Pri; ret = true; break;}
        case SpinCodeLexer::OBJ : {state = Obj; ret = true; break;}
        case SpinCodeLexer::VAR : {state = Var; curVar.type = SpinVar::Unknown; ret = true; break;}
        default: break;
    }
    if (ret) {
        if (lastSectionIndex >= 0) {
            contexts[lastSectionIndex].end = text - textStart;
            lastSectionIndex = -1;
        }
        curContext.start = text - textStart;
    }
    switch (token) {
        case SpinCodeLexer::CON :
            curContext.ctx = SpinCodeContext::Con;
            lastSectionIndex = contexts.size();
            contexts << curContext;
            break;
        case SpinCodeLexer::OBJ :
            curContext.ctx = SpinCodeContext::Obj;
            lastSectionIndex = contexts.size();
            contexts << curContext;
            break;
        case SpinCodeLexer::VAR :
            curContext.ctx = SpinCodeContext::Var;
            lastSectionIndex = contexts.size();
            contexts << curContext;
            break;
        case SpinCodeLexer::DAT :
            curContext.ctx = SpinCodeContext::Dat;
            lastSectionIndex = contexts.size();
            contexts << curContext;
            break;
        default: ;
    }
    return ret;
}


SpinCodeContext SpinCodeParser::getContext(int pos)
{
    SpinCodeContext ret;
    ret.ctx = SpinCodeContext::Unknown;
    for (int i = 0; i < contexts.size(); i++) {
        SpinCodeContext ctx = contexts[i];
        if (ctx.start <= pos && ctx.end >= pos) {
            return ctx;
        }
    }
    return ret;
}

const SpinContextList SpinCodeParser::getMethods(SpinCodeContext::Context filter)
{
    SpinContextList ret;
    for (int i = 0; i < contexts.size(); i++) {
        SpinCodeContext ctx = contexts[i];
        if (ctx.ctx == SpinCodeContext::Pri || ctx.ctx == SpinCodeContext::Pub) {
            if (ctx.ctx == filter || filter == SpinCodeContext::Unknown)
                ret << ctx;
        }
    }
    return ret;
}


QString SpinObjectsList::getFileName(QString objName)
{
    QMapIterator<QString, QString> i(*this);
    while (i.hasNext()) {
        i.next();
        QString c = i.key();
        if (c.toLower() == objName) {
            return i.value();
        }
    }
    return QString();
}

SpinHighlightList::SpinHighlightList():
        capacity(0), sz(0), data(0)
{

}

SpinHighlightList::~SpinHighlightList()
{
    delete[] data;
}

const SpinHighlightInfo *SpinHighlightList::get(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < sz);
    return data + idx;
}

void SpinHighlightList::append(int len, SpinCodeLexer::Retval style)
{
    if (capacity <= sz) {
        SpinHighlightInfo* newData = new SpinHighlightInfo[capacity + 1024];
        memcpy(newData, data,  sizeof(SpinHighlightInfo) * sz);
        capacity += 1024;
        delete[] data;
        data = newData;
    }
    data[sz].len = len;
    data[sz].style = style;
    sz++;
}

void SpinHighlightList::clear()
{
    sz = 0;
}

} // namespace PZST

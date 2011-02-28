#include "spinparser.h"

using namespace PZST;

SpinParser::SpinParser()
{
}

#define GET_POS(pattern, var) {\
if (pos > var) { \
        char *c = ::strstr(str + pos, pattern);\
        if (!c) var = max;\
        else var = c - str; \
   } \
}


void SpinParser::parse(QString text)
{
    methods.clear();
    QByteArray original = text.toAscii();
    QByteArray bytes = text.toLower().toAscii();
    char *str = (char*)bytes.constData();
    char *originalStr = (char*)original.constData();
    int max = ::strlen(str);
    int pos = 0;
    State state = Initial;
    int line = 0;
    bool bol = true;
    int lastBlockIndex = -1;

    char pComment[] = "'";
    char pMLCommentStart[] = "{";
    char pMLComment2Start[] = "{{";
    char pMLCommentEnd[] = "}";
    char pMLComment2End[] = "}}";
    char pQuote[] = "\"";
    char pNL[] = "\n";
    char pPub[] = "pub";
    char pPri[] = "pri";
    char pCon[] = "con";
    char pVar[] = "var";
    char pDat[] = "dat";
    char pObj[] = "obj";

    int posComment = -1;
    int posMLComment = -1;
    int posMLComment2 = -1;
    int posMLCommentEnd = -1;
    int posMLComment2End = -1;
    int posQuote = -1;
    int posPub = -1;
    int posPri = -1;
    int posVar = -1;
    int posDat = -1;
    int posCon = -1;
    int posObj = -1;
    int posNL  = -1;

    while (pos < max) {
        GET_POS(pComment, posComment);
        GET_POS(pMLCommentStart, posMLComment);
        GET_POS(pMLComment2Start, posMLComment2);
        GET_POS(pMLCommentEnd, posMLCommentEnd);
        GET_POS(pMLComment2End, posMLComment2End);
        GET_POS(pQuote, posQuote);
        GET_POS(pPub, posPub);
        GET_POS(pPri, posPri);
        GET_POS(pPri, posPri);
        GET_POS(pCon, posCon);
        GET_POS(pVar, posVar);
        GET_POS(pDat, posDat);
        GET_POS(pObj, posObj);
        GET_POS(pNL, posNL);

        int index = qMin(posComment, posMLComment);
        index = qMin(index, posMLComment2);
        index = qMin(index, posMLCommentEnd);
        index = qMin(index, posMLComment2End);
        index = qMin(index, posNL);
        index = qMin(index, posPri);
        index = qMin(index, posPub);
        index = qMin(index, posVar);
        index = qMin(index, posDat);
        index = qMin(index, posCon);
        index = qMin(index, posObj);
        index = qMin(index, posQuote);

        if (state == Comment) {
            index = posNL;
        }

        if (index >= max) {
            break;
        }
        if (index == posNL) {
            bol = true;
            line ++;
            pos = index + 1;
            if (state == Comment)
                state = Initial;
            continue;
        }

        switch (state) {
        case Initial:
            if (index == posComment) {
                state = Comment;
                pos = index + 1;
                if (str[index+1] == '\'') pos++;
            } else if (index == posMLComment2) {
                state = MLComment2;
                pos = index + 2;
            } else if (index == posMLComment) {
                state = MLComment;
                pos = index + 1;
            } else if (index == posQuote) {
                state = Quote;
                pos = index + 1;
            } else if (index == posPub || index == posPri) {
                int len = 0;
                if (bol && pos == index) {
                    len = methodName(str + index + 3);
                    if (len) {
                        methods.append(SpinMethodInfo(QString(QByteArray(originalStr + index + 3, len)), index == posPri, line+1));
                        if (lastBlockIndex >= 0) {
                            SpinMethodInfo &info = methods[lastBlockIndex];
                            info.lines = line - info.line + 1;
                        }
                        lastBlockIndex = methods.size()-1;
                    }
                }
                pos = index + len + 3;
            } else if (index == posVar || index == posObj || index == posDat || index == posCon) {
                char c = str[index+3];
                if (bol && pos == index && (c==' ' || c == '\t' || c == 0 || c == '\r' || c == '\n')) {
                    if (lastBlockIndex >= 0) {
                        SpinMethodInfo &info = methods[lastBlockIndex];
                        info.lines = line - info.line + 1;
                        lastBlockIndex = -1;
                    }
                }
                pos = index + 3;
            } else {
                pos = max;
            }
            break;
        case Comment:
            pos = max;
            break;
        case Quote:
            pos = posQuote + 1;
            state = Initial;
            break;
        case MLComment:
            if (posNL < posMLCommentEnd) {
                line++;
                pos = posNL + 1;
            } else {
                pos = posMLCommentEnd + 1;
                state = Initial;
            }
            break;
        case MLComment2:
            if (posNL < posMLComment2End) {
                line++;
                pos = posNL + 1;
            } else {
                pos = posMLComment2End + 2;
                state = Initial;
            }
            break;
        }
        bol = false;
    }
    if (lastBlockIndex >= 0) {
        SpinMethodInfo &info = methods[lastBlockIndex];
        info.lines = line - info.line + 2;
        lastBlockIndex = -1;
    }
}

int SpinParser::methodName(char *str)
{
    int len = 0;
    if (*str != ' ' && *str != '\t') return 0;
    while (*str == ' ' || *str == '\t') {
        len++;
        str++;
    }
    if (!*str) return 0;
    if ((*str < 'a' || *str > 'z') && (*str != '_')) return 0;
    while ((*str >= 'a' && *str <= 'z') || (*str >= '0' && *str <= '9') || (*str == '_')) {
        str++;
        len++;
    }
    return len;
}

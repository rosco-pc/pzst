#include "spincompletionsource.h"
#include "spincodeparser.h"
#include "spineditor.h"
#include "spinsourcefactory.h"
#include "filenameresolver.h"

#include <QFileInfo>
namespace PZST {

SpinCompletionSource::SpinCompletionSource(QsciLexer *l, SpinEditor *e)
        : QsciAbstractAPIs(l), editor(e)
{
}

QStringList SpinCompletionSource::callTips(const QStringList &context, int commas, QsciScintilla::CallTipsStyle style, QList<int> &shifts)
{
    Q_UNUSED(commas);
    Q_UNUSED(style);
    Q_UNUSED(shifts);
    QStringList ret;
    int line, col;
    editor->getCursorPosition(&line, &col);
    int pos = editor->positionFromLineIndex(line, col);
    SpinCodeContext ctx = editor->getParser()->getContext(pos);
    if (ctx.ctx != SpinCodeContext::Pri && ctx.ctx != SpinCodeContext::Pub) return ret;

    if (context.size() != 2 && context.size() != 4) return ret;
    QString objName;
    QString methodName;
    SpinContextList methods = editor->getParser()->getMethods();
    if (context.size() == 2) {
        methodName = context[0];
    } else {
        methodName = context[2];
        objName = context[0];
    }
    if (objName.isEmpty()) {
        methods = editor->getParser()->getMethods();
    } else {
        QFileInfo info(editor->getFileName());
        QString topDir = info.absolutePath();
        SpinObjectsList objects = editor->getParser()->getObjects();
        QString fileName = FilenameResolver::resolve(objects.getFileName(objName), "spin", topDir);
        SpinCodeParser *parser = SpinSourceFactory::instance()->getParser(fileName);
        if (!parser->isValid()) parser->parseCode(SpinSourceFactory::instance()->getSource(fileName));
        methods = parser->getMethods(SpinCodeContext::Pub);
    }
    for (int i = 0; i < methods.size(); i++) {
        SpinCodeContext method = methods.at(i);
        if (method.name.toLower() == methodName.toLower()) {
            QString signature;
            for (int j = 0; j < method.params.size(); j++) {
                if (!signature.isEmpty()) signature += ", ";
                signature += method.params[j];
            }
            signature += ")";
            signature.prepend(method.name + "(");
            ret << signature;
        }
    }
    return ret;
}

void SpinCompletionSource::updateAutoCompletionList(const QStringList &context, QStringList &list)
{
    int line, index;
    editor->getCursorPosition(&line, &index);
    int pos = editor->positionFromLineIndex(line, index);
    SpinCodeContext ctx = editor->getParser()->getContext(pos);
    switch (ctx.ctx) {
        case SpinCodeContext::Pub :
        case SpinCodeContext::Pri : {
            QStringList locals = ctx.locals;
            locals << ctx.params;
            updateInMethod(context, list, locals);
            break;
        }
        case SpinCodeContext::Con:
            updateInCon(context, list);
            break;
        case SpinCodeContext::Obj:
            updateInObj(context, list);
            break;
        case SpinCodeContext::Var:
            updateInVar(context, list);
            break;
        case SpinCodeContext::Dat:
            updateInDat(context, list);
            break;
        default: ;
    }
}

void SpinCompletionSource::updateInMethod(const QStringList &context, QStringList &list, QStringList locals)
{
    addMethods(context, list);
    addConstants(context, list, false);
    addObjects(context, list, false);
    addLabels(context, list, false);
    addGlobals(context, list);
    addLocals(context, list, locals);
    addObjectMethods(context, list);
    addObjectConstants(context, list, false);
}

void SpinCompletionSource::updateInCon(const QStringList &context, QStringList &list)
{
    addConstants(context, list, false);
    addObjects(context, list, false);
    addObjectConstants(context, list, false);
}

void SpinCompletionSource::updateInObj(const QStringList &context, QStringList &list)
{
    addConstants(context, list, false);
    addObjects(context, list, false);
    addObjectConstants(context, list, false);
}

void SpinCompletionSource::updateInVar(const QStringList &context, QStringList &list)
{
    addConstants(context, list, false);
    addObjects(context, list, false);
    addObjectConstants(context, list, false);
}

void SpinCompletionSource::updateInDat(const QStringList &context, QStringList &list)
{
    addLabels(context, list, true);
    addConstants(context, list, true);
    addObjects(context, list, true);
    addObjectConstants(context, list, true);
}

void SpinCompletionSource::addConstants(const QStringList &context, QStringList &list, bool allowPrefix)
{
    if (context.size() > 2) return;
    QString part;
    if (!allowPrefix && context.size() > 1) return;
    part = context.last();
    QStringList consts = editor->getParser()->getConstants();
    for (int i = 0;  i < consts.size(); i++) {
        QString c= consts.at(i);
        if (c.toLower().startsWith(part.toLower())) {
           list <<  QString("%1?3").arg(c);
        }
    }
}

void SpinCompletionSource::addObjects(const QStringList &context, QStringList &list, bool allowPrefix)
{
    if (context.size() > 2) return;
    QString part;
    if (!allowPrefix && context.size() > 1) return;
    part = context.last();
    SpinObjectsList objects = editor->getParser()->getObjects();
    QMapIterator<QString, QString> i(objects);
    while (i.hasNext()) {
        i.next();
        QString c = i.key();
        if (c.toLower().startsWith(part.toLower())) {
           list <<  c + "?9";
        }
    }
}

void SpinCompletionSource::addLabels(const QStringList &context, QStringList &list, bool allowPrefix)
{
    if (context.size() > 2) return;
    QString part;
    if (!allowPrefix && context.size() > 1) return;
    part = context.last();
    QStringList labels = editor->getParser()->getAsmGlobals();
    for (int i = 0;  i < labels.size(); i++) {
        QString label = labels.at(i);
        if (label.toLower().startsWith(part.toLower())) {
           list << label + "?6";
        }
    }
}

void SpinCompletionSource::addGlobals(const QStringList &context, QStringList &list)
{
    if (context.size() > 1) return;
    QString part = context[0];
    SpinVarsList vars = editor->getParser()->getGlobals();
    for (int i = 0;  i < vars.size(); i++) {
        SpinVar var= vars.at(i);
        if (var.name.toLower().startsWith(part.toLower())) {
            QString str = var.name;
            switch (var.type) {
                case SpinVar::Byte : str += "?8"; break;
                case SpinVar::Long : str += "?4"; break;
                case SpinVar::Word : str += "?7"; break;
                default: break;
            }

            list << str;
        }
    }
}

void SpinCompletionSource::addMethods(const QStringList &context, QStringList &list)
{
    if (context.size() != 1) return;
    QString part = context[0];
    SpinContextList methods = editor->getParser()->getMethods();
    for (int i = 0;  i < methods.size(); i++) {
        SpinCodeContext method = methods.at(i);
        if (method.name.toLower().startsWith(part.toLower())) {
            int iconIdx = method.ctx == SpinCodeContext::Pri ? 2 : 1;
            list << QString("%1?%2").arg(method.name).arg(iconIdx);
        }
    }
}

void SpinCompletionSource::addLocals(const QStringList &context, QStringList &list, QStringList locals)
{
    if (context.size() != 1) return;
    QString part = context[0];
    for (int i = 0;  i < locals.size(); i++) {
        QString name = locals.at(i);
        if (name.toLower().startsWith(part.toLower())) {
            list << name + "?5";
        }
    }
}

void SpinCompletionSource::addObjectMethods(const QStringList &context, QStringList &list)
{
    if (context.size() != 3) return;
    if (context[1] != ".") return;
    QFileInfo info(editor->getFileName());
    QString topDir = info.absolutePath();
    QString part = context[2];
    QString objName = context[0];
    SpinObjectsList objects = editor->getParser()->getObjects();
    QString fileName = FilenameResolver::resolve(objects.getFileName(objName), "spin", topDir);
    SpinCodeParser *parser = SpinSourceFactory::instance()->getParser(fileName);
    if (!parser->isValid()) parser->parseCode(SpinSourceFactory::instance()->getSource(fileName));
    SpinContextList methods = parser->getMethods(SpinCodeContext::Pub);
    for (int i = 0; i < methods.size(); i++) {
        QString name = methods[i].name  +"?1";
        if (name.toLower().startsWith(part.toLower())) {
            list << name;
        }
    }
}

void SpinCompletionSource::addObjectConstants(const QStringList &context, QStringList &list, bool allowPrefix)
{
    if (context.size() != 3 && context.size() != 4) return;
    QString objName;
    QString part;
    if (context.size() == 4) {
        if (!allowPrefix) return;
        if (context[0] != "#") return;
        if (context[2] != "#") return;
        objName = context[1];
        part = context[3];
    } else {
        if (context[1] != "#") return;
        objName = context[0];
        part = context[2];
    }
    QFileInfo info(editor->getFileName());
    QString topDir = info.absolutePath();
    SpinObjectsList objects = editor->getParser()->getObjects();
    QString fileName = FilenameResolver::resolve(objects.getFileName(objName), "spin", topDir);
    SpinCodeParser *parser = SpinSourceFactory::instance()->getParser(fileName);
    if (!parser->isValid()) parser->parseCode(SpinSourceFactory::instance()->getSource(fileName));
    QStringList constants= parser->getConstants();
    for (int i = 0; i < constants.size(); i++) {
        if (constants[i].toLower().startsWith(part.toLower())) {
            list << constants[i] + "?3";
        }
    }
}

} // namespace PZST

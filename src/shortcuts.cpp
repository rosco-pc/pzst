#include "shortcuts.h"
#include <QObject>
#include <Qsci/qsciscintillabase.h>

namespace PZST {


static const COMMAND allCommands[] = {
    {
        "File.Close",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Close"),
        QKeySequence::Close,
        0,
        0
    },
    {
        "File.CloseAll",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Close all"),
        0,
        0,
        0
    },
    {
        "File.Open",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Open ..."),
        QKeySequence::Open,
        0,
        0
    },
    {
        "File.New",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "New"),
        QKeySequence::New,
        0,
        0
    },
    {
        "File.Save",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Save"),
        QKeySequence::Save,
        0,
        0
    },
    {
        "File.SaveAs",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Save as ..."),
        QKeySequence::SaveAs,
        0,
        0
    },
    {
        "File.Quit",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Quit"),
        QKeySequence::Quit,
        0,
        0
    },
    {
        "Editor.Cut",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Cut"),
        QKeySequence::Cut,
        0,
        0
    },
    {
        "Editor.Copy",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Copy"),
        QKeySequence::Copy,
        0,
        0
    },
    {
        "Editor.Paste",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Paste"),
        QKeySequence::Paste,
        0,
        0
    },
    {
        "Compile.Detect",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Detect Propeller"),
        0,
        Qt::Key_F7,
        0
    },
    {
        "Compile.Compile",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Compile"),
        0,
        Qt::Key_F9,
        0
    },
    {
        "Compile.LoadRAM",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Compile and load RAM"),
        0,
        Qt::Key_F10,
        0
    },
    {
        "Compile.LoadROM",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Compile and load ROM"),
        0,
        Qt::Key_F11,
        0
    },
    {
        "Compile.WriteEEPROM",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Compile and save EEPROM file"),
        0,
        0,
        0
    },
    {
        "Compile.WriteBINARY",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Compile and save BINARY file"),
        0,
        0,
        0
    },
    {
        "Help.About",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "About PZST ..."),
        0,
        0,
        0
    },
    {
        "Environment.Preferences",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Preferences ..."),
        QKeySequence::Preferences,
        0,
        0
    },
    {
        "Editor.Undo",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Undo"),
        QKeySequence::Undo,
        0,
        0
    },
    {
        "Editor.Redo",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Redo"),
        QKeySequence::Redo,
        0,
        0
    },
    {
        "Editor.Find",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Search/Replace"),
        QKeySequence::Find,
        0,
        0
    },
    {
        "Editor.FindNext",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Find next"),
        QKeySequence::FindNext,
        0,
        0
    },
    {
        "Editor.QuickSearch",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Quick Search"),
        0,
        Qt::Key_Slash + Qt::ControlModifier,
        0
    },
    {
        "Editor.Replace",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Replace and find next"),
        QKeySequence::Replace,
        0,
        0
    },
    {
        "Editor.Autocomplete",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Autocomplete"),
        0,
        Qt::Key_Period + Qt::ControlModifier,
        0
    },
    {
        "Editor.CallTip",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Call tip"),
        0,
        Qt::Key_Period + Qt::ControlModifier + Qt::ShiftModifier,
        0
    },
    {
        "Editor.Fold",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Fold"),
        0,
        0,
        0
    },
    {
        "Editor.Unfold",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Unfold"),
        0,
        0,
        0
    },
    {
        "File.Print",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Print ..."),
        QKeySequence::Print,
        0,
        0
    },
    {
        "Editor.ZoomIn",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Increase font size"),
        QKeySequence::ZoomIn,
        0,
        0
    },
    {
        "Editor.ZoomOut",
        QT_TRANSLATE_NOOP("PZST::Shortcuts", "Decrease font size"),
        QKeySequence::ZoomOut,
        0,
        0
    },

    {
        "Editor.LineDn",
        QT_TRANSLATE_NOOP("QsciCommand", "Move down one line"),
        0,
        Qt::Key_Down,
        QsciScintillaBase::SCI_LINEDOWN,
    },
    {
        "Editor.LineDnExtend",
        QT_TRANSLATE_NOOP("QsciCommand", "Extend selection down one line"),
        0,
        Qt::Key_Down | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEDOWNEXTEND,
    },
    {
        "Editor.LineDnScroll",
        QT_TRANSLATE_NOOP("QsciCommand", "Scroll view down one line"),
        0,
        Qt::Key_Down | Qt::CTRL,
        QsciScintillaBase::SCI_LINESCROLLDOWN,
    },
    {
        "Editor.LineDnExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand", "Extend rectangular selection down one line"),
        0,
        Qt::Key_Down | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEDOWNRECTEXTEND,
    },
    {
        "Editor.LineUp",
        QT_TRANSLATE_NOOP("QsciCommand", "Move up one line"),
        0,
        Qt::Key_Up,
        QsciScintillaBase::SCI_LINEUP,
    },
    {
        "Editor.LineUpExtend",
        QT_TRANSLATE_NOOP("QsciCommand", "Extend selection up one line"),
        0,
        Qt::Key_Up | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEUPEXTEND,
    },
    {
        "Editor.LineUpScroll",
        QT_TRANSLATE_NOOP("QsciCommand", "Scroll view up one line"),
        0,
        Qt::Key_Up | Qt::CTRL,
        QsciScintillaBase::SCI_LINESCROLLUP,
    },
    {
        "Editor.LineUpExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand", "Extend rectangular selection up one line"),
        0,
        Qt::Key_Up | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEUPRECTEXTEND,
    },

    {
        "Editor.ParUp",
        QT_TRANSLATE_NOOP("QsciCommand","Move up one paragraph"),
        0,
        Qt::Key_BracketLeft | Qt::CTRL,
        QsciScintillaBase::SCI_PARAUP,
    },
    {
        "Editor.ParUpExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection up one paragraph"),
        0,
        Qt::Key_BraceLeft | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_PARAUPEXTEND,
    },
    {
        "Editor.ParDn",
        QT_TRANSLATE_NOOP("QsciCommand","Move down one paragraph"),
        0,
        Qt::Key_BracketRight | Qt::CTRL,
        QsciScintillaBase::SCI_PARADOWN,
    },
    {
        "Editor.ParDnExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection down one paragraph"),
        0,
        Qt::Key_BraceRight | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_PARADOWNEXTEND,
    },
    {
        "Editor.CharLt",
        QT_TRANSLATE_NOOP("QsciCommand","Move left one character"),
        0,
        Qt::Key_Left,
        QsciScintillaBase::SCI_CHARLEFT,
    },
    {
        "Editor.CharLtExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection left one character"),
        0,
        Qt::Key_Left | Qt::SHIFT,
        QsciScintillaBase::SCI_CHARLEFTEXTEND,
    },
    {
        "Editor.WordLt",
        QT_TRANSLATE_NOOP("QsciCommand","Move left one word"),
        0,
        Qt::Key_Left | Qt::CTRL,
        QsciScintillaBase::SCI_WORDLEFT,
    },
    {
        "Editor.WordLtExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection left one word"),
        0,
        Qt::Key_Left | Qt::SHIFT | Qt::CTRL,
        QsciScintillaBase::SCI_WORDLEFTEXTEND,
    },
    {
        "Editor.CharLtExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand","Extend rectangular selection left one character"),
        0,
        Qt::Key_Left | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_CHARLEFTRECTEXTEND,
    },
    {
        "Editor.CharRt",
        QT_TRANSLATE_NOOP("QsciCommand","Move right one character"),
        0,
        Qt::Key_Right,
        QsciScintillaBase::SCI_CHARRIGHT,
    },
    {
        "Editor.CharRtExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection right one character"),
        0,
        Qt::Key_Right | Qt::SHIFT,
        QsciScintillaBase::SCI_CHARRIGHTEXTEND,
    },
    {
        "Editor.WordRt",
        QT_TRANSLATE_NOOP("QsciCommand","Move right one word"),
        0,
        Qt::Key_Right | Qt::CTRL,
        QsciScintillaBase::SCI_WORDRIGHT,
    },
    {
        "Editor.WordRtExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection right one word"),
        0,
        Qt::Key_Right | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_WORDRIGHTEXTEND,
    },
    {
        "Editor.CharRtExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand","Extend rectangular selection right one character"),
        0,
        Qt::Key_Right | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_CHARRIGHTRECTEXTEND,
    },
    {
        "Editor.VisHome",
        QT_TRANSLATE_NOOP("QsciCommand","Move to first visible character in line"),
        0,
        Qt::Key_Home,
        QsciScintillaBase::SCI_VCHOME,
    },
    {
        "Editor.VisHomeExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection to first visible character in line"),
        0,
        Qt::Key_Home | Qt::SHIFT,
        QsciScintillaBase::SCI_VCHOMEEXTEND,
    },
    {
        "Editor.TextStart",
        QT_TRANSLATE_NOOP("QsciCommand","Move to start of text"),
        0,
        Qt::Key_Home | Qt::CTRL,
        QsciScintillaBase::SCI_DOCUMENTSTART,
    },
    {
        "Editor.TextStartExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection to start of text"),
        0,
        Qt::Key_Home | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_DOCUMENTSTARTEXTEND,
    },
    {
        "Editor.DisplayHome",
        QT_TRANSLATE_NOOP("QsciCommand","Move to start of displayed line"),
        0,
        Qt::Key_Home | Qt::ALT,
        QsciScintillaBase::SCI_HOMEDISPLAY,
    },
    {
        "Editor.DisplayHomeExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection to start of line"),
        0,
        0,
        QsciScintillaBase::SCI_HOMEDISPLAYEXTEND,
    },
    {
        "Editor.VisHomeExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand","Extend rectangular selection to first visible character in line"),
        0,
        Qt::Key_Home | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_VCHOMERECTEXTEND,
    },
    {
        "Editor.LineEnd",
        QT_TRANSLATE_NOOP("QsciCommand","Move to end of line"),
        0,
        Qt::Key_End,
        QsciScintillaBase::SCI_LINEEND,
    },
    {
        "Editor.LineEndExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection to end of line"),
        0,
        Qt::Key_End | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEENDEXTEND,
    },
    {
        "Editor.TextEnd",
        QT_TRANSLATE_NOOP("QsciCommand","Move to end of text"),
        0,
        Qt::Key_End | Qt::CTRL,
        QsciScintillaBase::SCI_DOCUMENTEND,
    },
    {
        "Editor.TextEndExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection to end of text"),
        0,
        Qt::Key_End | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_DOCUMENTENDEXTEND,
    },
    {
        "Editor.DisplayEnd",
        QT_TRANSLATE_NOOP("QsciCommand","Move to end of displayed line"),
        0,
        Qt::Key_End | Qt::ALT,
        QsciScintillaBase::SCI_LINEENDDISPLAY,
    },
    {
        "Editor.DisplayEndExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection to end of displayed line"),
        0,
        0,
        QsciScintillaBase::SCI_LINEENDDISPLAYEXTEND,
    },
    {
        "Editor.LineEndExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand","Extend rectangular selection to end of line"),
        0,
        Qt::Key_End | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEENDRECTEXTEND,
    },
    {
        "Editor.PgUp",
        QT_TRANSLATE_NOOP("QsciCommand","Move up one page"),
        0,
        Qt::Key_PageUp,
        QsciScintillaBase::SCI_PAGEUP,
    },
    {
        "Editor.PgUpExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection up one page"),
        0,
        Qt::Key_PageUp | Qt::SHIFT,
        QsciScintillaBase::SCI_PAGEUPEXTEND,
    },
    {
        "Editor.PgUpExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand","Extend rectangular selection up one page"),
        0,
        Qt::Key_PageUp | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_PAGEUPRECTEXTEND,
    },
    {
        "Editor.PgDn",
        QT_TRANSLATE_NOOP("QsciCommand","Move down one page"),
        0,
        Qt::Key_PageDown,
        QsciScintillaBase::SCI_PAGEDOWN,
    },
    {
        "Editor.PgDnExtend",
        QT_TRANSLATE_NOOP("QsciCommand","Extend selection down one page"),
        0,
        Qt::Key_PageDown | Qt::SHIFT,
        QsciScintillaBase::SCI_PAGEDOWNEXTEND,
    },
    {
        "Editor.PgDnExtendRect",
        QT_TRANSLATE_NOOP("QsciCommand","Extend rectangular selection down one page"),
        0,
        Qt::Key_PageDown | Qt::ALT | Qt::SHIFT,
        QsciScintillaBase::SCI_PAGEDOWNRECTEXTEND,
    },
    {
        "Editor.Clear",
        QT_TRANSLATE_NOOP("QsciCommand","Delete current character"),
        0,
        Qt::Key_Delete,
        QsciScintillaBase::SCI_CLEAR,
    },
    {
        "Editor.DelWordRt",
        QT_TRANSLATE_NOOP("QsciCommand","Delete word to right"),
        0,
        Qt::Key_Delete | Qt::CTRL,
        QsciScintillaBase::SCI_DELWORDRIGHT,
    },
    {
        "Editor.DelLineRt",
        QT_TRANSLATE_NOOP("QsciCommand","Delete line to right"),
        0,
        Qt::Key_Delete | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_DELLINERIGHT,
    },
    {
        "Editor.ToggleIns",
        QT_TRANSLATE_NOOP("QsciCommand","Toggle insert/overtype"),
        0,
        Qt::Key_Insert,
        QsciScintillaBase::SCI_EDITTOGGLEOVERTYPE,
    },
    {
        "Editor.DelBack",
        QT_TRANSLATE_NOOP("QsciCommand","Delete previous character"),
        0,
        Qt::Key_Backspace,
        QsciScintillaBase::SCI_DELETEBACK,
    },
    {
        "Editor.DelWordLt",
        QT_TRANSLATE_NOOP("QsciCommand","Delete word to left"),
        0,
        Qt::Key_Backspace | Qt::CTRL,
        QsciScintillaBase::SCI_DELWORDLEFT,
    },
    {
        "Editor.DelLineLt",
        QT_TRANSLATE_NOOP("QsciCommand","Delete line to left"),
        0,
        Qt::Key_Backspace | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_DELLINELEFT,
    },
    {
        "Editor.SelectAll",
        QT_TRANSLATE_NOOP("QsciCommand","Select all text"),
        0,
        Qt::Key_A | Qt::CTRL,
        QsciScintillaBase::SCI_SELECTALL,
    },
    {
        "Editor.CutLine",
        QT_TRANSLATE_NOOP("QsciCommand","Cut current line"),
        0,
        Qt::Key_L | Qt::CTRL,
        QsciScintillaBase::SCI_LINECUT,
    },
    {
        "Editor.DelLine",
        QT_TRANSLATE_NOOP("QsciCommand","Delete current line"),
        0,
        Qt::Key_L | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_LINEDELETE,
    },
    {
        "Editor.LineCopy",
        QT_TRANSLATE_NOOP("QsciCommand","Copy current line"),
        0,
        Qt::Key_T | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_LINECOPY,
    },
    {
        "Editor.LineSwap",
        QT_TRANSLATE_NOOP("QsciCommand","Swap current and previous lines"),
        0,
        Qt::Key_T | Qt::CTRL,
        QsciScintillaBase::SCI_LINETRANSPOSE,
    },
    {
        "Editor.DupSel",
        QT_TRANSLATE_NOOP("QsciCommand","Duplicate selection"),
        0,
        Qt::Key_D | Qt::CTRL,
        QsciScintillaBase::SCI_SELECTIONDUPLICATE,
    },
    {
        "Editor.Lower",
        QT_TRANSLATE_NOOP("QsciCommand","Convert selection to lower case"),
        0,
        Qt::Key_U | Qt::CTRL,
        QsciScintillaBase::SCI_LOWERCASE,
    },
    {
        "Editor.Upper",
        QT_TRANSLATE_NOOP("QsciCommand","Convert selection to upper case"),
        0,
        Qt::Key_U | Qt::CTRL | Qt::SHIFT,
        QsciScintillaBase::SCI_UPPERCASE,
    },
    {
        "Editor.Indent",
        QT_TRANSLATE_NOOP("QsciCommand","Indent"),
        0,
        Qt::Key_Tab,
        QsciScintillaBase::SCI_TAB,
    },
    {
        "Editor.Unindent",
        QT_TRANSLATE_NOOP("QsciCommand", "Unindent"),
        0,
        Qt::Key_Tab | Qt::SHIFT,
        QsciScintillaBase::SCI_BACKTAB,
    },

    {0, 0, 0, 0, 0},
};

Shortcuts::Shortcuts()
{
}

QList<QKeySequence> Shortcuts::defaultSequence(QString name)
{
    const QMap<QString, const COMMAND*> & map = Shortcuts::map();
    if (map.contains(name)) {
        const COMMAND* cmd = map[name];
        if (cmd->std) return  QKeySequence::keyBindings((QKeySequence::StandardKey)cmd->std);
        else if (cmd->key) return  QList<QKeySequence>() << QKeySequence(cmd->key);
    }
    return QList<QKeySequence>();
}

QString Shortcuts::title(QString name)
{
    const QMap<QString, const COMMAND*> & map = Shortcuts::map();
    if (map.contains(name)) return map[name]->title;
    return QString();
}

QStringList Shortcuts::allNames()
{
    const QMap<QString, const COMMAND*> & map = Shortcuts::map();
    return map.keys();
}

int Shortcuts::qsciCommand(QString name)
{
    const QMap<QString, const COMMAND*> & map = Shortcuts::map();
    if (map.contains(name)) {
        return map[name]->sciMsg;
    }
    return 0;
}

const QMap<QString, const COMMAND*> & Shortcuts::map()
{
    static QMap<QString, const COMMAND*> map;
    if (map.isEmpty()) {
        const COMMAND * cmd = allCommands;
        while(cmd->name) {
            map[cmd->name] = cmd;
            cmd++;
        }
    }
    return map;
}


} // namespace PZST

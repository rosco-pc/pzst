#include "shortcuts.h"
#include <QObject>

namespace PZST {


    typedef struct {
        const char *name;
        const char *title;
        int std;
        const char *key;
    } COMMAND;

static const COMMAND allCommands[] = {
    {"File.Close",              "Close",                        QKeySequence::Close,        0},
    {"File.CloseAll",           "Close all",                    0,                          0},
    {"File.Open",               "Open ...",                     QKeySequence::Open,         0},
    {"File.New",                "New",                          QKeySequence::New,          0},
    {"File.Save",               "Save",                         QKeySequence::Save,         0},
    {"File.SaveAs",             "Save as ...",                  QKeySequence::SaveAs,       0},
    {"File.Quit",               "Quit",                         QKeySequence::Quit,         0},
    {"Editor.Cut",              "Cut",                          QKeySequence::Cut,          0},
    {"Editor.Copy",             "Copy",                         QKeySequence::Copy,         0},
    {"Editor.Paste",            "Paste",                        QKeySequence::Paste,        0},
    {"Compile.Detect",          "Detect Propeller",             0,                          "F7"},
    {"Compile.Compile",         "Compile",                      0,                          "F9"},
    {"Compile.LoadRAM",         "Compile and load RAM",         0,                          "F10"},
    {"Compile.LoadROM",         "Compile and load ROM",         0,                          "F11"},
    {"Compile.WriteEEPROM",	"Compile and save EEPROM file",	0,                          0},
    {"Compile.WriteBINARY",	"Compile and save BINARY file", 0,                          0},
    {"Help.About",              "About PZST ...",               0,                          0},
    {"Environment.Preferences",	"Preferences ...",              QKeySequence::Preferences,  0},
    {"Editor.Undo",             "Undo",                         QKeySequence::Undo,         0},
    {"Editor.Redo",             "Redo",                         QKeySequence::Redo,         0},
    {"Editor.Find",             "Search/Replace",               QKeySequence::Find,         0},
    {"Editor.FindNext",         "Find next",                    QKeySequence::FindNext,     0},
    {"Editor.QuickSearch",	"Quick Search",                 0,                          "Ctrl+/"},
    {"Editor.Replace",          "Replace and find next",        QKeySequence::Replace,      0},
    {"Editor.Autocomplete",	"Autocomplete",                 0,                          "Ctrl+."},
    {"Editor.CallTip",          "Call tip",                     0,                          "Ctrl+Shift+."},
    {"Editor.Fold",             "Fold",                         0,                          0},
    {"Editor.Unfold",           "Unfold",                       0,                          0},
    {"File.Print",              "Print ...",                    QKeySequence::Print,        0},
    {"Editor.ZoomIn",           "Increase font size",           QKeySequence::ZoomIn,       0},
    {"Editor.ZoomOut",          "Decrease font size",           QKeySequence::ZoomOut,      0},
    {0, 0, 0, 0},
};

Shortcuts::Shortcuts()
{
}

QList<QKeySequence> Shortcuts::defaultSequence(QString name)
{
    static QMap<QString, QList<QKeySequence> >  shortcuts;
    if (shortcuts.empty()) initShortcuts(shortcuts);
    if (shortcuts.contains(name)) return shortcuts[name];
    return QList<QKeySequence>();
}

QString Shortcuts::title(QString name)
{
    static QMap<QString, QString>  &titles = Shortcuts::titles();
    if (titles.contains(name)) return titles[name];
    return QString();
}

void Shortcuts::initShortcuts(QMap<QString, QList<QKeySequence> > & shortcuts)
{
    const COMMAND *cmd ;
    for (cmd = allCommands; cmd->name; cmd++) {
        QList<QKeySequence> list;
        if (cmd->std) list = QKeySequence::keyBindings((QKeySequence::StandardKey)cmd->std);
        else if (cmd->key) list << QKeySequence(cmd->key);
        shortcuts[cmd->name] = list;
    }
}

void Shortcuts::initTitles(QMap<QString, QString> &titles)
{
    const COMMAND *cmd ;
    for (cmd = allCommands; cmd->name; cmd++) {
        titles[cmd->name] = tr(cmd->title);
    }
}

QMap<QString, QString>& Shortcuts::titles()
{
    static QMap<QString, QString>  titles;
    if (titles.empty()) initTitles(titles);
    return titles;
}

QStringList Shortcuts::allNames()
{
    return titles().keys();
}

} // namespace PZST



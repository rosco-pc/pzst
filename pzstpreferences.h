#ifndef PZSTPREFERENCES_H
#define PZSTPREFERENCES_H
#include <QSettings>
#include <QStringList>

#define SETTING(group, name, type, defaultValue, getter) \
type get ## name() {\
    settings->beginGroup(#group);\
    type ret = settings->value(#name, defaultValue).getter(); \
    settings->endGroup(); \
    return ret; \
}\
void set ## name(type value) {\
    settings->beginGroup(#group);\
    settings->setValue(#name, value); \
    settings->endGroup(); \
}

#define SETTING_BOOL(group, name, defaultValue) SETTING(group, name, bool, defaultValue, toBool)
#define SETTING_INT(group, name, defaultValue) SETTING(group, name, int, defaultValue, toInt)
#define SETTING_STRING(group, name, defaultValue) SETTING(group, name, QString, defaultValue, toString)
#define SETTING_STRINGLIST(group, name) SETTING(group, name, QStringList, QStringList(), toStringList)


namespace PZST {
    class Preferences
    {
    public:
        Preferences();
        ~Preferences();

        SETTING_STRING      (Editor, FontName,  "Parallax");
        SETTING_INT         (Editor, FontSize,  12);
        SETTING_INT         (Editor, TabSize,  4);
        SETTING_BOOL        (Editor, TabsToSpaces, false);
        SETTING_BOOL        (Editor, TabsVisible, false);
        SETTING_INT         (Editor, Language,  0);
        SETTING_BOOL        (Editor, UTF16,  false);

        SETTING_STRINGLIST  (Compiler, SearchPath);
        SETTING_BOOL        (Compiler, FoldConstants, true);
        SETTING_BOOL        (Compiler, EliminateUnused, true);
        SETTING_BOOL        (Compiler, FoldUnary, true);
        SETTING_BOOL        (Compiler, GenericSafe, true);
        SETTING_BOOL        (Compiler, FastConstants, true);
        SETTING_BOOL        (Compiler, NonParallaxExt, false);
        SETTING_BOOL        (Compiler, Warnings, false);
        SETTING_BOOL        (Compiler, Info, false);

        SETTING_STRING      (Port, PortName,  "");
        SETTING_BOOL        (Port, DoubleSpeed,  true);

    private:
        QSettings *settings;
    };
}

#endif // PZSTPREFERENCES_H

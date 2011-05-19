#ifndef PZSTPREFERENCES_H
#define PZSTPREFERENCES_H
#include <QSettings>
#include <QStringList>
#include <QVariant>

#define SETTING(group, name, type, defaultValue, getter) \
type get ## name() {\
    QSettings &s = instance();\
    s.beginGroup(#group);\
    type ret = s.value(#name, defaultValue).getter(); \
    s.endGroup(); \
    return ret; \
}\
void set ## name(type value_) {\
    QSettings &s = instance();\
    s.beginGroup(#group);\
    if (s.value(#name, defaultValue) != value_) {\
        s.setValue(#name, value_); \
        emit valueChanged(#group, #name, QVariant(value_));\
    }\
    s.endGroup(); \
}

#define SETTING_BOOL(group, name, defaultValue) SETTING(group, name, bool, defaultValue, toBool)
#define SETTING_INT(group, name, defaultValue) SETTING(group, name, int, defaultValue, toInt)
#define SETTING_STRING(group, name, defaultValue) SETTING(group, name, QString, defaultValue, toString)
#define SETTING_STRINGLIST(group, name) SETTING(group, name, QStringList, QStringList(), toStringList)


namespace PZST {
    class Preferences : public QObject
    {
        Q_OBJECT
    private:
        static QSettings &instance();
    public:
        Preferences();
        ~Preferences();
        static void connectInstance(const char *sig, const QObject *obj, const char * slot);

        SETTING_STRING      (Editor, FontName,  "Parallax");
        SETTING_INT         (Editor, FontSize,  12);
        SETTING_INT         (Editor, TabSize,  4);
        SETTING_BOOL        (Editor, TabsToSpaces, false);
        SETTING_BOOL        (Editor, TabsVisible, false);
        SETTING_INT         (Editor, Language,  0);
        SETTING_BOOL        (Editor, UTF16,  false);
        SETTING_BOOL        (Editor, LineNumbers,  false);
        SETTING_BOOL        (Editor, CurLineMarker,  false);

        SETTING_STRINGLIST  (Compiler, SearchPath);
        SETTING_BOOL        (Compiler, FoldConstants, true);
        SETTING_BOOL        (Compiler, EliminateUnused, true);
        SETTING_BOOL        (Compiler, FoldUnary, true);
        SETTING_BOOL        (Compiler, GenericSafe, true);
        SETTING_BOOL        (Compiler, FastConstants, true);
        SETTING_BOOL        (Compiler, NonParallaxExt, false);
        SETTING_BOOL        (Compiler, Warnings, false);
        SETTING_BOOL        (Compiler, Info, false);
        SETTING_STRING      (Compiler, Encoding,  "ISO-8859-1");

        SETTING_STRING      (Port, PortName,  "");
        SETTING_BOOL        (Port, DoubleSpeed,  true);
    signals:
        void valueChanged(QString, QString, QVariant);
    };
}

#endif // PZSTPREFERENCES_H

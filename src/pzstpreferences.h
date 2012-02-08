#ifndef PZSTPREFERENCES_H
#define PZSTPREFERENCES_H
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <QKeySequence>
#include <QColor>

#define SETTING(group, name, type, defaultValue, getter) \
void set ## name(type value_) {\
    QSettings &s = settings;\
    s.beginGroup(#group);\
    if (s.value(#name, defaultValue) != value_) {\
        s.setValue(#name, value_); \
        s.sync();\
        signaller().emitValueChanged(#group, #name, QVariant(value_));\
    }\
    s.endGroup(); \
}\
type get ## name() {\
    QSettings &s = settings;\
    s.beginGroup(#group);\
    type ret = s.value(#name, defaultValue).getter(); \
    s.endGroup(); \
    return ret; \
}

#define SETTING_BOOL(group, name, defaultValue) SETTING(group, name, bool, defaultValue, toBool)
#define SETTING_INT(group, name, defaultValue) SETTING(group, name, int, defaultValue, toInt)
#define SETTING_STRING(group, name, defaultValue) SETTING(group, name, QString, defaultValue, toString)
#define SETTING_STRINGLIST(group, name) SETTING(group, name, QStringList, QStringList(), toStringList)
#define SETTING_COLOR(group, name, defaultValue) SETTING(group, name, QColor, defaultValue, value<QColor>)


namespace PZST {
    class Preferences : public QObject
    {
        Q_OBJECT
    private:
        static QSettings &instance();
        QSettings settings;
        static Preferences &signaller();
        void emitValueChanged(QString, QString, QVariant);
        void emitShortcutChanged(QString, QString);
        void emitShortcutChanged(int, int, int, int);
    public:
        Preferences();
        ~Preferences();
        static void connectInstance(const char *sig, const QObject *obj, const char * slot);

        SETTING_STRING      (Editor, FontName,  "Parallax");
        SETTING_INT         (Editor, FontSize,  13);
        SETTING_INT         (Editor, TabSize,  4);
        SETTING_BOOL        (Editor, TabsToSpaces, false);
        SETTING_BOOL        (Editor, TabsVisible, false);
        SETTING_INT         (Editor, Language,  0);
        SETTING_BOOL        (Editor, UTF16,  false);
        SETTING_BOOL        (Editor, LineNumbers,  false);
        SETTING_BOOL        (Editor, CurLineMarker,  false);
        SETTING_BOOL        (Editor, Zebra,  false);

        SETTING_COLOR       (Editor, ColorCondition,  QColor(255, 0, 0));
        SETTING_COLOR       (Editor, ColorComment,  QColor(128, 128, 128));
        SETTING_COLOR       (Editor, ColorReserved,  QColor(0, 128, 0));
        SETTING_COLOR       (Editor, ColorIdentifier,  QColor(0, 0, 255));
        SETTING_COLOR       (Editor, ColorNumber,  QColor(255, 0, 255));
        SETTING_COLOR       (Editor, ColorType,  QColor(128, 128, 0));
        SETTING_COLOR       (Editor, ColorString,  QColor(128, 64, 0));
        SETTING_COLOR       (Editor, ColorPreprocessor,  QColor(128, 0, 128));
        SETTING_COLOR       (Editor, ColorOther,  QColor(0, 0, 0));
        SETTING_COLOR       (Editor, ColorSelection,  QColor(255, 255, 255));
        SETTING_COLOR       (Editor, ColorLineNumbers,  QColor(0, 0, 0));

        SETTING_COLOR       (Editor, PaperCurrentLine,  QColor(240, 240, 240));
        SETTING_COLOR       (Editor, PaperSelection,  QColor(117, 80, 123));
        SETTING_COLOR       (Editor, PaperLineNumbers,  QColor(240, 240, 240));

        SETTING_COLOR       (Editor, PaperPub,  QColor(0xbf, 0xdf, 0xff));
        SETTING_COLOR       (Editor, PaperPri,  QColor(0xbf, 0xf8, 0xff));
        SETTING_COLOR       (Editor, PaperCon,  QColor(0xfd, 0xf3, 0xa8));
        SETTING_COLOR       (Editor, PaperVar,  QColor(0xff, 0xdf, 0xbf));
        SETTING_COLOR       (Editor, PaperObj,  QColor(0xff, 0xbf, 0xbf));
        SETTING_COLOR       (Editor, PaperDat,  QColor(0xbf, 0xff, 0xc8));


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
        SETTING_STRING      (Compiler, Defines,  "");

        SETTING_STRING      (Port, PortName,  "");
        SETTING_BOOL        (Port, DoubleSpeed,  true);

        QList<QKeySequence> getShortcuts(QString name);
        void                setShortcut(QString name, QString value, QKeySequence::SequenceFormat = QKeySequence::NativeText);

    signals:
        void valueChanged(QString, QString, QVariant);
        void shortcutChanged(QString, QString);
        void shortcutChanged(int, int, int, int);
    };
}

#endif // PZSTPREFERENCES_H

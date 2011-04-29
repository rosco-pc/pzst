#ifndef TERMINALDISPLAY_H
#define TERMINALDISPLAY_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QVector>
#include <QTextCodec>

namespace PZST {
    struct TerminalData
    {
        uint timestamp;
        QByteArray *data;
        bool received;

        TerminalData();
        TerminalData(QByteArray &data, bool received);
        ~TerminalData();
    };

    class TerminalDisplay : public QWidget
    {
    Q_OBJECT
    public:
        enum DisplayMode {
            Text,
            Hex,
        };

    public:
        explicit TerminalDisplay(QWidget *parent = 0);
        ~TerminalDisplay();

    signals:

    public slots:
        void appendSent(QByteArray &data);
        void appendReceived(QByteArray &data);

    private:
        QPlainTextEdit *textView;
        QVector<TerminalData*> log;
        QTextCodec *codec;
        DisplayMode displayMode;

        void appendDataChunk(TerminalData*);
        void clear();
    };
}

#endif // TERMINALDISPLAY_H

#ifndef CHARTABLE_H
#define CHARTABLE_H

#include <QWidget>
#include <QVariant>

namespace PZST {

    class CharTable : public QWidget
    {
    Q_OBJECT
    public:
        explicit CharTable(QWidget *parent = 0);
        void changeFont(QFont &font);

    signals:
        void charSelected(QChar c);
        void charSelected(QString s);
        void charHighlighted(QChar c);

    public slots:
        void preferencesChanged(QString, QString, QVariant);

    protected:
        QString chars;
        virtual void paintEvent(QPaintEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);
        virtual void mousePressEvent(QMouseEvent *);
        virtual int heightForWidth (int w) const;

        int gridWidth, gridHeight, charIndex;

    };
}

#endif // CHARTABLE_H

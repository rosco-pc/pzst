#include "chartable.h"
#include "pzstpreferences.h"
#include <QPaintEvent>
#include <QPainter>

using namespace PZST;

CharTable::CharTable(QWidget *parent) :
    QWidget(parent)
{
    chars = QString::fromUtf8("←→↑↓◀▶‣•ΔπΣΩ≈√ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~─│┼╋┤├┴┬┫┣┻┳┘└┐┌¡£€¥°±²³µ¹¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ∞");
    Preferences pref;
    QFont f;
    f.setFamily(pref.getFontName());
    f.setPointSize(pref.getFontSize());
    setFont(f);
    QSizePolicy p =  sizePolicy();
    p.setHeightForWidth(true);
    setSizePolicy(p);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    charIndex = -1;
    gridHeight = gridWidth = 0;
}

void CharTable::paintEvent(QPaintEvent *e)
{
    bool needResize = !gridWidth;
    QFont f = font();
    f.setPointSize(20);
    QPainter painter(this);
    painter.setFont(f);
    QRect r = rect();
    int max = 0, height = 0;
    for (int i = 0; i < chars.size(); i++) {
        QRect bounds =  painter.boundingRect(r, Qt::AlignLeft, chars.at(i));
        max = qMax(max, bounds.width());
        height = qMax(height, bounds.height());
    }
    gridWidth = max;
    gridHeight = height;
    int x = 0, y = 0;
    QColor hlColor = QColor(255, 255, 255);
    for (int i = 0; i < chars.size(); i++) {
        if (x && (x + max >= r.width())) {
            x = 0; y+= height;
        }
        QRect charRect(x, y, max, height);
        if (i == charIndex) painter.fillRect(charRect, hlColor);
        painter.setPen(Qt::black);
        painter.drawRect(charRect);
        painter.drawText(charRect, Qt::AlignCenter, chars.at(i));
        x += max;
    }
    e->accept();
    if (needResize) resize(1,1);
}


int CharTable::heightForWidth(int w) const
{
    if (gridWidth) {
        int charsPerRow = w / gridWidth;
        if (charsPerRow >= 256) return gridHeight;
        int rows = 256 / charsPerRow;
        if (rows * charsPerRow < 256) rows++;
        return rows * gridHeight + 1;
    }
    return 1000;
}

void CharTable::mouseMoveEvent(QMouseEvent *e)
{
    int charsPerRow = width() / gridWidth;
    if (e->x() >= charsPerRow * gridWidth) {
        if (charIndex >= 0) {
            charIndex = -1;
            repaint();
        }
        return;
    }
    int newIndex = e->x() / gridWidth + e->y() / gridHeight * charsPerRow;
    if (newIndex >= chars.size()) {
        if (charIndex >= 0) {
            charIndex = -1;
            repaint();
        }
        return;
    }
    if (newIndex != charIndex) {
        charIndex = newIndex;
        repaint();
        if (charIndex >=0) {
            emit charHighlighted(chars.at(charIndex));
        }
    }
}

void CharTable::mousePressEvent(QMouseEvent *e)
{
    if (charIndex >= 0 && e->buttons() == Qt::LeftButton) {
        emit charSelected(chars.at(charIndex));
        emit charSelected(QString(chars.at(charIndex)));
    }
}

void CharTable::changeFont(QFont &font)
{
    setFont(font);
    repaint();
    resize(1,1);
}

void CharTable::preferencesChanged(QString section, QString name, QVariant value)
{
    if (section == "Editor" && name == "FontName") {
        QFont f(value.toString());
        changeFont(f);
    }
}

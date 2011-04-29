#include "terminaldisplay.h"
#include <QBoxLayout>
#include <QDateTime>

using namespace PZST;

TerminalData::TerminalData() :
    timestamp(0), data(0)
{

}

TerminalData::TerminalData(QByteArray &data, bool received) :
    received(received)
{
    QDateTime dt;
    timestamp = dt.toTime_t();
    this->data = new QByteArray(data);
}

TerminalData::~TerminalData()
{
    if (data) delete data;
}

TerminalDisplay::TerminalDisplay(QWidget *parent) :
    QWidget(parent)
{
    textView = new QPlainTextEdit(this);
    textView->setReadOnly(true);
    QHBoxLayout *hbl = new QHBoxLayout(0);
    hbl->addWidget(textView);
    setLayout(hbl);
    codec = QTextCodec::codecForName("Latin-1");
    log.append(new TerminalData());
}

TerminalDisplay::~TerminalDisplay()
{
    for (int i = 0; i < log.size(); i++) {
        delete log.at(i);
    }
}

void TerminalDisplay::appendReceived(QByteArray &data)
{
    TerminalData *chunk = new TerminalData(data, true);
    appendDataChunk(chunk);
}

void TerminalDisplay::appendSent(QByteArray &data)
{
    TerminalData *chunk = new TerminalData(data, false);
    appendDataChunk(chunk);
}

void TerminalDisplay::appendDataChunk(TerminalData *chunk)
{
    Q_ASSERT(chunk != 0);
    if (!chunk->data) return;
    if (!log.size() || log.at(log.size()-1)->received != chunk->received) {
        log.append(chunk);
    } else {
        log.at(log.size()-1)->data->append(*chunk->data);
        delete chunk;
    }
}

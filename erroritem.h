#ifndef ERRORITEM_H
#define ERRORITEM_H

#include <QListWidgetItem>
#include "spincompiler.h"
namespace PZST {
    class ErrorItem : public QListWidgetItem
    {
    public:
        ErrorItem(SpinError err, QListWidget *parent = 0);
        SpinError err;
    private:
    };
}

#endif // ERRORITEM_H

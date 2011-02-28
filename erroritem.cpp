#include "erroritem.h"

using namespace PZST;

ErrorItem::ErrorItem(SpinError err, QListWidget *parent)
    :QListWidgetItem(err.message, parent)
{
    this->err = err;
}

#include "erroritem.h"

using namespace PZST;

ErrorItem::ErrorItem(SpinError err, QListWidget *parent)
    :QListWidgetItem(err.message, parent)
{
    this->err = err;
    if (err.severity == SpinError::Error) setIcon(QIcon(":/Icons/error.png"));
    else if (err.severity == SpinError::Warning) setIcon(QIcon(":/Icons/warning.png"));
    else if (err.severity == SpinError::Info) setIcon(QIcon(":/Icons/info.png"));
}

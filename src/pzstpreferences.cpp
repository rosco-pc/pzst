#include "pzstpreferences.h"

using namespace PZST;

Preferences::Preferences()
{
    settings = new QSettings;
}

Preferences::~Preferences()
{
    delete settings;
}


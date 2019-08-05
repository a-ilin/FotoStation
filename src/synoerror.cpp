#include "synoerror.h"

QMetaEnum SynoErrorGadget::metaEnum()
{
    return QMetaEnum::fromType<SynoErrorGadget::SynoError>();
}

SynoErrorGadget::SynoErrorGadget(QObject *parent) : QObject(parent)
{
}

#ifndef MESSAGES_H_INCLUDED
#define MESSAGES_H_INCLUDED

#include <QString>
#include "machine.h"
#include "parser.h"

namespace Messages
{
    bool machineResult(QString &out, size_t res);
    bool parserError(QString &out, MachineIO::ErrorCode code);
}

#endif

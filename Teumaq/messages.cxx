#include "messages.h"

#define TR(x) QObject::tr((x))

/*****
 * TODO: improve method of storage and data
 * query.
 */

static QString machResTxt[] = {
    // XR_END
    TR("Completed successfully"),
    // XR_NOPROD
    TR("Bad configuration"),
    // XR_LIMIT
    TR("Time limit reached"),
    // XR_BADSTATE
    TR("Machine is not initialized")
};
static const size_t machResSize = 4;

static QString parErrTxt[] = {
    // OK
    QString(),
    // PROD_L_STATE
    TR("Bad state in the condition of command"),
    // PROD_L_SYMBOL
    TR("Bad symbol in the condition of command"),
    // PROD_OP
    TR("Missing '->' operator in command"),
    // PROD_R_SYMBOL
    TR("Bad symbol in the action of command"),
    // PROD_R_DIR
    TR("Wrong movement direction identifier"),
    // PROD_R_STATE
    TR("Bad state in the action of command"),
    // SYM_UNEXP
    TR("Unexpected symbol"),
    // SYM_BAD
    TR("Bad symbol"),
    // SCAL_MIS
    TR("Bad scalar literal"),
    // DIR_NOPARAM
    TR("Missing directive parameter"),
    // STREAM_ERROR
    TR("I/O failure"),
    // TAPE_EMPTY
    TR("Empty tape is not allowed"),
    // TAPE_SYM_ILLEGAL
    TR("Illegal symbol in tape specification"),
    // CUR_OFFSET
    TR("Illegal cursor offset specifier")
};
static const size_t parErrSize = 15;

bool Messages::machineResult(QString &out, size_t res)
{
    if (res < machResSize)
    {
        out = machResTxt[res];
        return true;
    }

    return false;
}

bool Messages::parserError(QString &out, MachineIO::ErrorCode code)
{
    if (static_cast<size_t>(code) < parErrSize)
    {
        out = parErrTxt[code];
        return true;
    }

    return false;
}

#undef TR

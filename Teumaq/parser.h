#ifndef PARSER_H
#define PARSER_H

#include <QTextStream>
#include <QStringList>
#include <QDataStream>
#include "lexer.h"
#include "machine.h"

/***** PARSER *****/
class MachineIO
{
public:
    enum ErrorCode
    {
        OK = 0, PROD_L_STATE, PROD_L_SYMBOL, PROD_OP,
        PROD_R_SYMBOL, PROD_R_DIR, PROD_R_STATE, SYM_UNEXP,
        SYM_BAD, SCAL_MIS, DIR_NOPARM, STREAM_ERROR,
        TAPE_EMPTY, TAPE_SYM_ILLEGAL, CUR_OFFSET
    };
    struct ParserMessage
    {
        unsigned line;
        ErrorCode error;
    };

    static const char DIR_LETTERS[3];

    static QString interpret(HeadMoveDir hmd)
    {
        switch (hmd)
        {
            case HMD_LEFT:
                return QObject::tr("Left (L)");
            case HMD_RIGHT:
                return QObject::tr("Right (R)");
            case HMD_STILL:
                return QObject::tr("Still (S)");
        }

        return QString();
    }

    MachineIO(Machine &m):
        _m(m)
    {
        _mesg.error = OK;
        _mesg.line = 0;
        setupLexer();
    }
    bool load(QTextStream &in);
    bool save(QTextStream &out, bool comments = false);
    bool loadBinary(QDataStream &in);
    bool saveBinary(QDataStream &out);
    const ParserMessage &message()
    { return _mesg; }
    ErrorCode parseProduction(const QString &src, Production &dest);
    static ErrorCode productionToString(const Machine &m, const Production &src, QString &dest);
    void tape2text(QStringList &dest);

protected:
    /*! \brief Ключевые слова. В лексический анализатор должны
     * помещаться в порядке от наиболее сложных к наиболее
     * простым.
     */
    enum
    {
        SPACE, SEPARATOR, COMMENT, PRODUCTION, SCAL_CONST,
        DEMPTY, DSTATE0, DSTATE1, DOFFSET, DALPHABET,
        DSTATES, DTAPE
    };

    void setupLexer();

private:
    /// Обёртка для чтения лексемы с обработкой ошибок
    bool lexical();

    /// Обёртка для чтения массива параметров
    bool readParams(QStringList &out);

    ParserMessage _mesg;
    QString _tokMesg;
    Lexer _lexer;
    Machine &_m;
};


#endif // PARSER_H

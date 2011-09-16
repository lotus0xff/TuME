#include "parser.h"
#include <QRegExp>
#include <QByteArray>
#include <QMap>
#include <QDebug>

const char MachineIO::DIR_LETTERS[3] = { 'S', 'R', 'L' };

MachineIO::ErrorCode MachineIO::parseProduction(const QString &src, Production &dest)
{
    int pb, pe;
    int ndx;
    QString curr;
    state_t st;
    symbol_t sym;

    // Condition: state. States are trimmed.
    pb = 0;
    pe = src.indexOf('(', pb);
    if (pe < 0 || pe == pb)
        return PROD_L_STATE;
    curr = src.mid(pb, pe - pb).trimmed();
    if (curr.size() == 0)
        return PROD_L_STATE;
    st = _m.stateByName(curr);
    if (st == Machine::STATE_ILLEGAL)
        return PROD_L_STATE;
    dest.inState = st;

    // Condition: symbol. Symbols are treated 'as is'.
    pb = pe + 1;
    pe = src.indexOf(')', pb);
    if (pe < 0 || pe == pb)
        return PROD_L_SYMBOL;
    curr = src.mid(pb, pe - pb);
    sym = _m.symbolByName(curr);
    if (sym == Machine::SYMBOL_ILLEGAL)
        return PROD_L_SYMBOL;
    dest.inSymbol = sym;

    // Find and skip arrow (left/right prod. part delimiter)
    pb = src.indexOf("->", pe + 1);
    if (pb < 0)
        return PROD_OP;
    pe = pb + 2;

    // Action: symbol
    pb = src.indexOf('(', pe);
    pe = src.indexOf(')', pb+1);
    if (pb < 0 || pe < 0)
        return PROD_R_SYMBOL;
    pb++;
    curr = src.mid(pb, pe - pb);
    sym = _m.symbolByName(curr);
    if (sym == Machine::SYMBOL_ILLEGAL)
        return PROD_R_SYMBOL;
    dest.outSymbol = sym;

    // Action: where to move
    pb = pe + 1;
    pe = src.indexOf('.', pb);
    if (pe < 0 || pe - pb < 1)
        return PROD_R_DIR;
    curr = src.mid(pb, pe - pb);
    char d = curr[0].toUpper().toAscii();
    for (ndx = 0; ndx < 3; ndx++)
        if (d == DIR_LETTERS[ndx])
            break;
    if (ndx > 2)
        return PROD_R_DIR;
    dest.moveDir = static_cast<HeadMoveDir>(ndx);

    // Action: target state
    pb = pe + 1;
    if (pb >= src.size())
        return PROD_R_STATE;
    curr = src.mid(pb).trimmed();
    st = _m.stateByName(curr);
    if (st == Machine::STATE_ILLEGAL)
        return PROD_R_STATE;
    dest.outState = st;

    return OK;
}

MachineIO::ErrorCode MachineIO::productionToString(const Machine &m, const Production &src, QString &dest)
{
    dest.clear();
    int nStates = m.states().size(),
        nSyms = m.symbols().size();
    if (src.inState < 0 || src.inState >= nStates)
        return PROD_L_STATE;
    dest.append(m.states().at(src.inState));
    if (src.inSymbol < 0 || src.inSymbol >= nSyms)
        return PROD_L_SYMBOL;
    dest.append('(').append(m.symbols().at(src.inSymbol)).append(")->(");
    if (src.outSymbol < 0 || src.outSymbol >= nSyms)
        return PROD_R_SYMBOL;
    dest.append(m.symbols().at(src.outSymbol)).append(')');
    if (src.moveDir < 0 || src.moveDir > 2)
        return PROD_R_DIR;
    dest.append(DIR_LETTERS[src.moveDir]).append('.');
    if (src.outState < 0 || src.outState >= nStates)
        return PROD_R_STATE;
    dest.append(m.states().at(src.outState));

    return OK;
}

void MachineIO::tape2text(QStringList &dest)
{
    Machine::Cursor_RO itr;
    for (_m.setToTapeFirst(itr); _m.validCursor(itr); ++itr)
        dest.append(_m.symbolName(*itr));
}

bool MachineIO::load(QTextStream &in)
{
    _m.reset(Machine::RM_ERASE);

    Production prod;
    _mesg.error = OK;
    _mesg.line = 0;
    _lexer.setInputStream(&in);

    // Параметры машины
    QMap<size_t, QString> productions;
    QStringList temp, tape, states, alphabet;
    QString empty, state0, state1;
    int offset = 0;

    // Начать синтаксический анализ
    bool go = lexical();
    while (go)
    {
        switch (_lexer.tokenID())
        {
        case PRODUCTION:
            productions.insert(_lexer.lineNo(), _lexer.tokenText());
        case COMMENT:
            go = lexical();
            continue;
        case DEMPTY:
            temp.clear();
            if (readParams(temp))
            {
                empty = temp.first();
                continue;
            }
            break;
        case DSTATE0:
            temp.clear();
            if (readParams(temp))
            {
                state0 = temp.first();
                continue;
            }
            break;
        case DSTATE1:
            temp.clear();
            if (readParams(temp))
            {
                state1 = temp.first();
                continue;
            }
            break;
        case DOFFSET:
            temp.clear();
            if (readParams(temp))
            {
                bool ok;
                offset = temp.first().toInt(&ok);
                if (ok)
                    continue;
            }
            _mesg.error = SYM_BAD;
            _mesg.line = _lexer.lineNo();
            return false;
        case DALPHABET:
            temp.clear();
            if (readParams(temp))
            {
                alphabet = temp;
                continue;
            }
            break;
        case DSTATES:
            temp.clear();
            if (readParams(temp))
            {
                states = temp;
                continue;
            }
            break;
        case DTAPE:
            temp.clear();
            if (readParams(temp))
            {
                tape = temp;
                continue;
            }
            break;
        }
        _mesg.error = DIR_NOPARM;
        _mesg.line = _lexer.lineNo();
        return false;
    }

    // Установить параметры машины и программу
    if (_mesg.error == OK)
    {
        // Опциональные
        if (!state1.isEmpty())
            _m.setInitialState(state1);
        if (!state0.isEmpty())
            _m.setFinalState(state0);
        if (!empty.isEmpty())
            _m.setEmptySymbol(empty);
        if (!states.isEmpty())
            _m.setStateSet(states, true);
        if (!alphabet.isEmpty())
            _m.setSymbolSet(alphabet, true);

        // Обязательный - текущая лента
        if (!tape.isEmpty())
        {
            if (_m.setTape(tape))
            {
                if (offset != 0)
                    if (!_m.moveStaticCursor(offset))
                        _mesg.error = CUR_OFFSET;
            }
            else
                _mesg.error = TAPE_SYM_ILLEGAL;
        }
        else
            _mesg.error = TAPE_EMPTY;

        // Программа
        foreach (size_t line, productions.keys())
        {
            _mesg.error = parseProduction(productions.value(line), prod);
            if (_mesg.error == OK)
                // Добавить продукцию в машину
                _m.addProduction(prod);
            else
            {
                // Формирование сообщения об ошибке и останов
                _mesg.line = line;
                break;
            }
        }
    }


    // Возвращает true при достижении конца потока (см. "lexical()")
    return _mesg.error == OK;
}

/*! \brief Save machine in current configuration to file.
 *
 * File format is same TML as any regular input file to parser.
 * "Current configuration" means that it may override initial state and
 * offset of the original machine so that its current configuration
 * will become initial configuration when loaded from that data
 * storage next time.
 * \param rout Output stream to write data to. Data will be written in plain
 * text, consumable by Parser::load().
 * \param comments Shall the procedure add some few comments about meaning of specific
 * fields or not.
 *\return Indicator of successfull operation carry.
 */
bool MachineIO::save(QTextStream &rout, bool comments)
{
    QStringList strlst;
    QStringList::ConstIterator itr;

    // States
    _m.getArbitraryStates(strlst);
    itr = strlst.begin();
    if (itr != strlst.end())
    {
        if (comments)
            rout << "# Valid states set" << endl;
        rout << "states:" << endl << *itr;
        while (++itr != strlst.end())
            rout << ", " << *itr;
        rout << endl << endl;
    }

    // Alphabet
    strlst.clear();
    _m.getArbitrarySymbols(strlst);
    itr = strlst.begin();
    if (itr != strlst.end())
    {
        if (comments)
            rout << "# Valid alphabet" << endl;
        rout << "alphabet:" << endl << *itr;
        while (++itr != strlst.end())
            rout << ", " << *itr;
        rout << endl << endl;
    }

    // Special states
    if (comments)
        rout << "# Mandatory states" << endl;
    rout << "state0: " << _m.stateName(Machine::STATE_FINAL) << endl
         << "state1: " << _m.stateName(Machine::STATE_INITIAL) << endl
         << endl;

    // Empty symbol
    if (comments)
        rout << "# Empty symbol" << endl;
    rout << "empty: " << _m.symbolName(Machine::SYMBOL_EMPTY) << endl
         << endl;

    // Current tape
    strlst.clear();
    tape2text(strlst);
    itr = strlst.begin();
    if (itr != strlst.end())
    {
        if (comments)
            rout << "# Tape contents" << endl;
        rout << "tape:" << endl << *itr;
        while (++itr != strlst.end())
            rout << ", " << *itr;
        rout << endl << endl;
    }

    // Offset from the beginning of the tape (if needed)
    qint64 off = _m.offset();
    if (off > 0)
    {
        if (comments)
            rout << "# Initial cursor offset from the beginning of the tape" << endl;
        rout << "offset: " << off << endl
             << endl;
    }

    // Put productions
    Machine::Program::ConstIterator icmd;
    QString str;
    if (comments)
        rout << "# Machine commands" << endl;
    for (icmd = _m.program().begin(); icmd != _m.program().end(); ++icmd)
    {
        _mesg.error = productionToString(_m, icmd.value(), str);
        if (_mesg.error != OK)
            break;
        rout << str << endl;
    }

    return _mesg.error == OK;
}

// Feed keywords/REs to lexer
void MachineIO::setupLexer()
{
    // Наиболее часто встречаемые элементы
    _lexer.addToken(PRODUCTION, "\\w+\\(\\w+(?:\\s+\\w+)*\\)\\s*->\\s*\\(\\w+(?:\\s+\\w+)*\\)[rRlLsS]\\.\\w+");
    _lexer.addToken(COMMENT, "#\\s*.*\\s*$");

    // Ключевые слова
    _lexer.addToken(DEMPTY, "[eE][mM][pP][tT][yY]\\s*:");
    _lexer.addToken(DSTATE0, "[sS][tT][aA][tT][eE]0\\s*:");
    _lexer.addToken(DSTATE1, "[sS][tT][aA][tT][eE]1\\s*:");
    _lexer.addToken(DOFFSET, "[oO][fF]{2}[sS][eE][tT]\\s*:");
    _lexer.addToken(DALPHABET, "[aA][lL][pP][hH][aA][bB][eE][tT]\\s*:");
    _lexer.addToken(DSTATES, "[sS][tT][aA][tT][eE][sS]\\s*:");
    _lexer.addToken(DTAPE, "[tT][aA][pP][eE]\\s*:");

    // Прочие элементы языка
    _lexer.addToken(SCAL_CONST, "\\w+(?!\\s*\\:)");
    _lexer.addToken(SEPARATOR, "\\s*[,;]\\s*");
    _lexer.addToken(SPACE, "\\s+", true);
}

/*! \brief Read parameter via lexical analyser.
 * \param out List that will accept parameters.
 * \return true if there were 1+ parameters recognized,
 * false otherwise.
 */
bool MachineIO::readParams(QStringList &out)
{
    Lexer::tokid_t prid = SEPARATOR;
    while (lexical())
    {
        switch (_lexer.tokenID())
        {
        case SCAL_CONST:
            if (prid == SEPARATOR)
            {
                out.append(_lexer.tokenText());
                prid = SCAL_CONST;
                continue;
            }
            break;
        case SEPARATOR:
            if (prid == SCAL_CONST)
            {
                prid = SEPARATOR;
                continue;
            }
        default:
            if (prid == SCAL_CONST)
                return true;
        }
        _mesg.error = SYM_UNEXP;
        _mesg.line = _lexer.lineNo();

        return false;
    }

    // Since nothing was read
    return false;
}

/*!
 * Wrapper for Lexer calls that
 */
bool MachineIO::lexical()
{
    _mesg.error = OK;
    switch (_lexer.lex())
    {
    case Lexer::OK:
        return true;
    case Lexer::BAD_TOKEN:
        _mesg.error = SYM_BAD;
        _mesg.line = _lexer.lineNo();
        break;
    case Lexer::BAD_STREAM:
        _mesg.error = STREAM_ERROR;
        break;
    default:
        ;
    }

    return false;
}

// To be implemented later
bool MachineIO::loadBinary(QDataStream &)
{
    return false;
}

// To be implemented later
bool MachineIO::saveBinary(QDataStream &)
{
    return false;
}


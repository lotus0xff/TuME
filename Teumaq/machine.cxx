#include "machine.h"

const state_t Machine::STATE_INITIAL = 1;
const state_t Machine::STATE_FINAL = 0;
const state_t Machine::STATE_ILLEGAL = -1;
const symbol_t Machine::SYMBOL_ILLEGAL = -1;
const symbol_t Machine::SYMBOL_EMPTY = 0;
const QString Machine::SYMBOL_EMPTY_STR = "_";
const QString Machine::STATE_INITIAL_STR = "q1";
const QString Machine::STATE_FINAL_STR = "q0";
const quint64 Machine::MAX_TIME = ~Q_INT64_C(0);

Machine::Machine(bool createTape):
    _stricStates(false), _stricAlphbt(false),
    _state(STATE_INITIAL), _time(Q_UINT64_C(0)),
    _log(NULL)
{
    _symbolSet.insert(SYMBOL_EMPTY, SYMBOL_EMPTY_STR);
    _stateSet.insert(STATE_FINAL, STATE_FINAL_STR);
    _stateSet.insert(STATE_INITIAL, STATE_INITIAL_STR);
    if (createTape)
    {
        _tape.append(SYMBOL_EMPTY);
        _cursor0 = _tape.begin();
    }
    else
        _cursor0 = _tape.end();
    _cursor = _cursor0;
}

Machine::Machine(const Machine &other):
    _log(NULL)
{
    // Simply call assignment
    *this = other;
}

const Machine &Machine::operator=(const Machine &other)
{
    Machine::Cursor_RO oiter;
    _program = other._program;
    _symbolSet = other._symbolSet;
    _stateSet = other._stateSet;
    _tape = other._tape;
    _state = other._state;
    _time = other._time;
    _stricStates = other._stricStates;
    _stricAlphbt = other._stricAlphbt;

    if (other.validCursor(other._cursor0))
    {
        _cursor0 = _tape.begin();
        oiter = other._tape.begin();
        while (oiter != other._cursor0)
        {
            ++oiter;
            ++_cursor0;
        }
    }
    else
        _cursor0 = _tape.end();
    if (other.validCursor(other._cursor))
    {
        _cursor = _tape.begin();
        oiter = other._tape.begin();
        while (oiter != other._cursor)
        {
            ++oiter;
            ++_cursor;
        }
    }
    else
        _cursor = _tape.end();

    /* Log is an external reference and thus is NOT
     * copied.
     */
    return *this;
}

state_t Machine::stateByName(const QString &name)
{
    int idx = _stateSet.indexOf(name);
    if (idx == -1)
    {
        if (_stricStates)
            return STATE_ILLEGAL;
        idx = _stateSet.size();
        _stateSet.insert(idx, name);
    }
    return static_cast<state_t>(idx);
}

symbol_t Machine::symbolByName(const QString &name)
{
    int idx = _symbolSet.indexOf(name);
    if (idx == -1)
    {
        if (_stricAlphbt)
            return SYMBOL_ILLEGAL;
        idx = _symbolSet.size();
        _symbolSet.insert(idx, name);
    }
    return static_cast<symbol_t>(idx);
}

void Machine::reset(ResetMethod method)
{
    if (method != RM_SOFT)
    {
        // RM_ERASE
        _symbolSet.clear();
        _symbolSet.insert(SYMBOL_EMPTY, SYMBOL_EMPTY_STR);
        _stateSet.clear();
        _stateSet.insert(STATE_FINAL, STATE_FINAL_STR);
        _stateSet.insert(STATE_INITIAL, STATE_INITIAL_STR);
        _tape.clear();
        _program.clear();

        // RM_ERASE_DEF?
        if (method == RM_ERASE_DEF)
        {
            _tape.append(SYMBOL_EMPTY);
            _cursor0 = _tape.begin();
        }
        else
            _cursor0 = _tape.end();
        _stricStates = _stricAlphbt = false;
    }

    // This is for RM_SOFT
    _state = STATE_INITIAL;
    _cursor = _cursor0;
    _time = Q_UINT64_C(0);
    if (_log)
        _log->clear();
}

Machine::RunResult Machine::exec(quint64 steps)
{
    RunResult ret;
    LogItem logItem;
    quint64 t = _time;
    steps += _time;
    if (_cursor == _tape.end())
    {
        // Return error: tape empty / machine not initialized
        ret.reason = XR_BADSTATE;
    }
    else
    {
        quint64 id;

        // Main execution loop
        ret.reason = XR_LIMIT;
        for (; t < steps && ret.reason == XR_LIMIT; ++t)
        {
            id = Production::identity(_state, *_cursor);
            if (_program.contains(id))
            {
                const Production &p = _program.value(id);

                // Perform logging (don't be confused, this
                // logic doesn't relate to machine operation.
                if (_log && !(_log->size() > 1 &&
                              Machine::isSpecial(_log->last().cmd) &&
                              _log->at(_log->size()-2).cmd == p))
                {
                    logItem.time = t;
                    logItem.cmd = p;
                    if (!_log->isEmpty() && _log->last().cmd == logItem.cmd)
                        Machine::setSpecial(logItem.cmd);
                    _log->append(logItem);
                }

                *_cursor = p.outSymbol;
                switch (p.moveDir)
                {
                case HMD_RIGHT:
                    if (_cursor + 1 == _tape.end())
                        _tape.append(SYMBOL_EMPTY);
                    ++_cursor;
                    break;
                case HMD_LEFT:
                    if (_cursor - 1 == _tape.end())
                        _tape.prepend(SYMBOL_EMPTY);
                    --_cursor;
                    break;
                case HMD_STILL:
                    break;
                }
                _state = p.outState;
                if (_state == STATE_FINAL)
                {
                    ret.reason = XR_END;
                    // break;
                }
            }
            else
            {
                // No productions for such case found... bad program
                ret.reason = XR_NOPROD;
                // break;
            }
        }
    }

    // Time of the last operation
    ret.elapsed = t - _time;
    _time = t;

    return ret;
}

/*! \brief Прогон машины в автоматическом режиме.
 *
 * Выполняет установку внутреннего состояния машины в
 * начальное, перемещает курсор на исходную позицию.
 * Выполняет прогон машины вплоть до указанного числа шагов.
 * Адаптирована для вызова в отдельном потоке.
 * \param timeLimit Максимально допустимое число шагов.
 * \return Дескриптор останова машины, в котором указана его причина.
 */
Machine::RunResult Machine::run(quint64 timeLimit)
{
    _state = STATE_INITIAL;
    _cursor = _cursor0;
    _time = Q_UINT64_C(0);
    if (_log)
        _log->clear();

    return exec(timeLimit);
}

/*! \brief Calculate the offset of current cursor position from the beginning of the tape.
 *
 */
qint64 Machine::offset() const
{
    qint64 pos;
    Cursor_RO curcur;
    for (pos = 0, curcur = _tape.begin(); curcur != _tape.end(); ++curcur, ++pos)
        if (curcur == _cursor)
            return pos;

    return -1;
}

bool Machine::currentProduction(Production &prd)
{
    if (_cursor != _tape.end())
    {
        quint64 id = Production::identity(_state, *_cursor);
        if (_program.contains(id))
        {
            prd = _program.value(id);
            return true;
        }
    }

    return false;
}

bool Machine::symbolInUse(symbol_t sym) const
{
    foreach (const Production &prd, _program.values())
        if (sym == prd.inSymbol || sym == prd.outSymbol)
            return true;
    foreach (symbol_t tsym, _tape)
        if (sym == tsym)
            return true;

    return false;
}

bool Machine::stateInUse(state_t state) const
{
    foreach (const Production &prd, _program.values())
        if (state == prd.inState || state == prd.outState)
            return true;

    return false;
}

void Machine::AsyncRunner::run()
{
    if (_reinit)
    {
        _m._state = STATE_INITIAL;
        _m._cursor = _m._cursor0;
        _m._time = Q_UINT64_C(0);
        if (_m._log)
            _m._log->clear();
    }
    _resOut.reason = XR_LIMIT;
    quint64 t = 0;
    while (!_halt &&
           t < _timeLimit &&
           _resOut.reason == XR_LIMIT)
    {
        _resOut = _m.exec(1);
        t++;
    }
    _halt = false;
}





#ifndef MACHINE_H
#define MACHINE_H

#include <QString>
#include <QVector>
#include <QLinkedList>
#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QMap>
#include <QtAlgorithms>
#include <QThread>

/***** BASIC TYPES *****/
// state_t, symbol_t are vectors index types
typedef int state_t;
typedef int symbol_t;

/// Machine cursor movement direction type
enum HeadMoveDir
{
    HMD_STILL = 0, HMD_RIGHT = 1, HMD_LEFT = 2
};

/// In-memory machine command representation
struct Production
{
    state_t inState;
    symbol_t inSymbol;
    symbol_t outSymbol;
    HeadMoveDir moveDir;
    state_t outState;

    bool operator ==(const Production &pr) const
    {
        return inState == pr.inState && inSymbol == pr.inSymbol &&
               outSymbol == pr.outSymbol && outState == pr.outState &&
               moveDir == pr.moveDir;
    }

    static quint64 identity(const Production &p)
    {
        quint64 res = static_cast<quint64>(p.inState) & Q_INT64_C(0xFFFFFFFF);
        res |= (static_cast<quint64>(p.inSymbol) & Q_INT64_C(0xFFFFFFFF)) << 32;
        return res;
    }

    static quint64 identity(state_t state, symbol_t symbol)
    {
        quint64 res = static_cast<quint64>(state) & Q_INT64_C(0xFFFFFFFF);
        res |= (static_cast<quint64>(symbol) & Q_INT64_C(0xFFFFFFFF)) << 32;
        return res;
    }
};

/***** TURING MACHINE ITSELF *****/

class Machine
{
public:
    /// In-memory program representation (bytecode ;-) )
    typedef QMap<quint64,Production> Program;

    /// Machine cursor (head)
    typedef QLinkedList<symbol_t>::Iterator Cursor;

    /// Read-only machine cursor
    typedef QLinkedList<symbol_t>::ConstIterator Cursor_RO;

    enum ExitReason
    {
        XR_END = 0, XR_NOPROD, XR_LIMIT, XR_BADSTATE
    };

    enum ResetMethod
    {
        RM_SOFT, RM_ERASE, RM_ERASE_DEF
    };

    struct RunResult
    {
        quint64 elapsed;
        ExitReason reason;
    };

    struct LogItem
    {
        quint64 time;
        Production cmd;
    };

    /// Allows to run the machine in asynchronous mode
    class AsyncRunner: public QThread
    {
        Machine &_m;
        volatile bool _halt;
        bool _reinit;
        RunResult _resOut;
        quint64 _timeLimit;

    public:
        AsyncRunner(Machine &m, quint64 timeLim = MAX_TIME,
                    bool fromInitialState = true, QObject *owner = NULL):
            QThread(owner),
            _m(m), _halt(false), _reinit(fromInitialState),
            _timeLimit(timeLim)
        {
            _resOut.elapsed = 0;
            _resOut.reason = XR_LIMIT;
        }
        void setTimeLimit(quint64 limit)
        {
            _timeLimit = limit;
        }
        void setToReinit(bool reinit)
        {
            _reinit = reinit;
        }
        virtual void run();
        bool stopped() const
        {
            return _halt;
        }
        const Machine::RunResult &result() const
        {
            return _resOut;
        }
        void stop()
        {
            _halt = true;
        }
    };

    static const state_t STATE_INITIAL;
    static const state_t STATE_FINAL;
    static const state_t STATE_ILLEGAL;
    static const symbol_t SYMBOL_ILLEGAL;
    static const symbol_t SYMBOL_EMPTY;
    static const QString SYMBOL_EMPTY_STR;
    static const QString STATE_INITIAL_STR;
    static const QString STATE_FINAL_STR;
    static const quint64 MAX_TIME;

    Machine(bool createTape = true);
    Machine(const Machine &other);
    const Machine &operator=(const Machine &other);
    state_t stateByName(const QString &name);
    symbol_t symbolByName(const QString &name);
    bool validState(state_t state) const
    {
        return state >= 0 &&
               state < _stateSet.size() &&
               state != STATE_ILLEGAL;
    }
    bool validSymbol(symbol_t symbol) const
    {
        return symbol >= 0 &&
               symbol < _symbolSet.size() &&
               symbol != SYMBOL_ILLEGAL;
    }
    const QVector<QString> &states() const
    {
        return _stateSet;
    }
    bool setState(state_t state, const QString &name)
    {
        if (state >= 0 && state < _stateSet.size() &&
            !name.isEmpty())
        {
            _stateSet[state] = name;
            return true;
        }
        else
            return false;
    }
    state_t addState(const QString &name)
    {
        bool old = _stricStates;
        _stricStates = false;
        state_t s = stateByName(name);
        _stricStates = old;
        return s;
    }
    bool delLastState()
    {
        state_t lstate = _stateSet.size() - 1;
        if (lstate >= 0 &&
            lstate != STATE_FINAL &&
            lstate != STATE_INITIAL)
        {
            _stateSet.pop_back();
            return true;
        }
        else
            return false;
    }
    state_t lastState() const
    {
        state_t s = _stateSet.size() - 1;
        return s >= 0 ? s : STATE_ILLEGAL;
    }
    bool symbolInUse(symbol_t sym) const;
    bool stateInUse(state_t state) const;
    void getArbitraryStates(QStringList &target)
    {
        for (int i = 0, ie = _stateSet.size(); i < ie; i++)
        {
            if (i != STATE_FINAL && i != STATE_INITIAL)
                target.append(_stateSet.at(i));
        }
    }
    const QVector<QString> &symbols() const
    {
        return _symbolSet;
    }
    bool setSymbol(symbol_t sym, const QString &name)
    {
        if (sym >= 0 && sym < _symbolSet.size() &&
            !name.isEmpty())
        {
            _symbolSet[sym] = name;
            return true;
        }
        else
            return false;
    }
    symbol_t addSymbol(const QString &name)
    {
        bool old = _stricAlphbt;
        _stricAlphbt = false;
        symbol_t s = symbolByName(name);
        _stricAlphbt = old;
        return s;
    }
    bool delLastSymbol()
    {
        symbol_t last = _symbolSet.size() - 1;
        if (last >= 0 && last != SYMBOL_EMPTY)
        {
            _symbolSet.pop_back();
            return true;
        }
        else
            return false;
    }
    symbol_t lastSymbol() const
    {
        symbol_t s = _symbolSet.size() - 1;
        return s >= 0 ? s : SYMBOL_ILLEGAL;
    }
    void getArbitrarySymbols(QStringList &target)
    {
        for (int i = 0, ie = _symbolSet.size(); i < ie; i++)
        {
            if (i != SYMBOL_EMPTY)
                target.append(_symbolSet.at(i));
        }
    }
    const Program &program() const
    {
        return _program;
    }
    void eraseProgram()
    {
        _program.clear();
    }
    void copyProgram(const Program &prg)
    {
        _program = prg;
    }
    const QString &symbolName(symbol_t sym) const
    {
        return _symbolSet.at(sym);
    }
    const QString &stateName(state_t st) const
    {
        return _stateSet.at(st);
    }
    const QLinkedList<symbol_t> &tape() const
    {
        return _tape;
    }
    bool appendToTape(symbol_t sym)
    {
        if (validSymbol(sym))
        {
            _tape.append(sym);
            return true;
        }
        else
            return false;
    }
    bool setTapeSymbol(size_t off, symbol_t sym)
    {
        if (validSymbol(sym) && static_cast<int>(off) < _tape.size())
        {
            *(_tape.begin() + off) = sym;
            return true;
        }
        return false;
    }
    bool delTapeSymbol(size_t off)
    {
        if (static_cast<int>(off) < _tape.size())
        {
            Cursor cur = _tape.begin() + off, tmp;
            tmp = _tape.erase(cur);
            if (tmp == _tape.end())
                setToTapeLast(tmp);
            if (cur == _cursor0)
                _cursor0 = tmp;
            if (cur == _cursor)
                _cursor = tmp;
            return true;
        }
        return false;
    }
    void clearTape()
    {
        _tape.clear();
    }
    void cropTapeRight(size_t syms = 1)
    {
        for (size_t i = 0; i < syms && !_tape.isEmpty(); i++)
            _tape.removeLast();
    }
    void cropTapeLeft(size_t syms = 1)
    {
        for (size_t i = 0; i < syms && !_tape.isEmpty(); i++)
            _tape.removeFirst();
    }
    bool setTape(const QStringList &symNames)
    {
        _tape.clear();
        symbol_t sym;
        foreach (const QString &symName, symNames)
        {
            sym = symbolByName(symName);
            if (sym == SYMBOL_ILLEGAL)
                return false;
            _tape.append(sym);
        }
        _cursor0 = _tape.begin();
        _cursor = _cursor0;

        return true;
    }
    Cursor_RO initialCursor() const
    {
        return _cursor0;
    }
    Cursor_RO cursor() const
    {
        return _cursor;
    }

    bool setToTapeFirst(Cursor_RO &cur) const
    {
        cur = _tape.begin();
        return cur != _tape.end();
    }
    bool setToTapeLast(Cursor_RO &cur) const
    {
        Cursor_RO tmp = _tape.begin();
        while (tmp != _tape.end())
            cur = tmp++;
        return cur != _tape.end();
    }
    bool setToTapeFirst(Cursor &cur)
    {
        cur = _tape.begin();
        return cur != _tape.end();
    }
    bool setToTapeLast(Cursor &cur)
    {
        Cursor tmp = _tape.begin();
        while (tmp != _tape.end())
            cur = tmp++;
        return cur != _tape.end();
    }
    bool validCursor(const Cursor &c)
    {
        return c != _tape.end() && validSymbol(*c);
    }
    bool validCursor(const Cursor_RO &c) const
    {
        return c != _tape.end() && validSymbol(*c);
    }
    state_t currentState() const
    {
        return _state;
    }
    bool changeCurrentState(state_t state)
    {
        if (state < _stateSet.size() && state != _state)
        {
            _state = state;
            return true;
        }

        return false;
    }
    bool changeCurrentSymbol(symbol_t symbol)
    {
        if (_cursor != _tape.end() && symbol != *_cursor)
        {
            *_cursor = symbol;
            return true;
        }

        return false;
    }

    quint64 currentTime() const
    {
        return _time;
    }
    bool strictAlphabet() const
    {
        return _stricAlphbt;
    }
    void setStrictAlphabet(bool strict)
    {
        _stricAlphbt = strict;
    }
    bool strictStateSet() const
    {
        return _stricStates;
    }
    void setStrictStateSet(bool strict)
    {
        _stricStates = strict;
    }
    qint64 offset() const;
    void setStateSet(const QStringList &statNames, bool strict = false)
    {
        _stricStates = false;
        QString sti = _stateSet.at(STATE_INITIAL),
                stf = _stateSet.at(STATE_FINAL);
        _stateSet.clear();
        _stateSet.reserve(statNames.size()+2);
        _stateSet.insert(STATE_FINAL, stf);
        _stateSet.insert(STATE_INITIAL, sti);
        QStringList::const_iterator itr;
        for (itr = statNames.begin(); itr != statNames.end(); ++itr)
            stateByName(*itr);
        _stricStates = strict;
    }
    void setSymbolSet(const QStringList &symNames, bool strict = false)
    {
        _stricAlphbt = false;
        QString syme = _symbolSet.at(SYMBOL_EMPTY);
        _symbolSet.clear();
        _symbolSet.reserve(symNames.size()+1);
        _symbolSet.insert(SYMBOL_EMPTY, syme);
        QStringList::const_iterator itr;
        for (itr = symNames.begin(); itr != symNames.end(); ++itr)
            symbolByName(*itr);
        _stricAlphbt = strict;
    }
    void setInitialState(const QString &stName)
    {
        _stateSet[STATE_INITIAL] = stName;
    }
    void setFinalState(const QString &stName)
    {
        _stateSet[STATE_FINAL] = stName;
    }
    void setEmptySymbol(const QString &symName)
    {
        _symbolSet[SYMBOL_EMPTY] = symName;
    }
    void addProduction(const Production &prd)
    {
        _program.insert(Production::identity(prd), prd);
    }
    void remProduction(quint64 id)
    {
        _program.remove(id);
    }
    void remProduction(state_t st, symbol_t sym)
    {
        _program.remove(Production::identity(st, sym));
    }

    /*! \brief Move static cursor, which points to the cell
     * being the current cell at the beginning of every execution.
     *
     * Current cursor is moved to the same position as static.
     * \param off Offset value, may be either positive or negative.
     */
    bool moveStaticCursor(int off)
    {
        _cursor0 = _tape.begin();
        bool fwd = off >= 0;
        off = qAbs(off);
        _cursor = _cursor0;
        for (int i = 0; i < off; i++)
        {
            if (fwd)
                ++_cursor;
            else
                --_cursor;
            if (_cursor == _tape.end())
            {
                _cursor = _cursor0;
                return false;
            }
        }
        _cursor0 = _cursor;
        return true;
    }
    void setLog(QList<LogItem> *log)
    {
        _log = log;
    }
    const QList<LogItem> *log() const
    {
        return _log;
    }
    bool currentProduction(Production &prd);

    /*! \brief Check whether production has special meaning or not.
     *
     * Productions with special meaning are not supposed to be ever
     * used in machine actions, but rather serve as a flag.
     * \param prd Production to check for special meaning.
     * \return true, if specified production has special meaning.
     */
    static bool isSpecial(const Production &prd)
    {
        return prd.inSymbol == SYMBOL_ILLEGAL &&
               prd.inState == STATE_ILLEGAL;
    }
    /*! \brief Set a production to the "special" state.
     *
     * After being passed to this function, production
     * becomes a special one and will never be activated.
     * \param prd Production to set to special state.
     */
    static void setSpecial(Production &prd)
    {
        prd.inState = prd.outState = STATE_ILLEGAL;
        prd.inSymbol = prd.outSymbol = SYMBOL_ILLEGAL;
    }
    void reset(ResetMethod method = RM_SOFT);

    /// Execute some instructions in current thread, allowing user
    /// to interact with machine ducring execution.
    RunResult exec(quint64 steps = 1);

    /// Reset the state of machine and run it.
    RunResult run(quint64 timeLimit = MAX_TIME);

private:
    /// Статическая составляющая машины - настройки
    QVector<QString> _stateSet;
    QVector<QString> _symbolSet;
    QLinkedList<symbol_t> _tape;
    Cursor _cursor0;
    Program _program;
    bool _stricStates,
         _stricAlphbt;

    /// Динамическая составляющая машины - её переменные состояния
    Cursor _cursor;
    state_t _state;
    quint64 _time;

    /// Журнал, в который записываются результаты работы машины
    QList<LogItem> *_log;

    friend class AsyncRunner;
};

#endif // MACHINE_H

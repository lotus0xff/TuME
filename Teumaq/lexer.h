#ifndef LEXER_H
#define LEXER_H

#include <QVector>
#include <QPair>
#include <QRegExp>
#include <QTextStream>
#include <QSet>


class Lexer
{
public:
    typedef unsigned tokid_t;

    enum StatusCode
    {
        OK, EOS, BAD_TOKEN, BAD_STREAM, BLOCK_MISMATCH
    };

    Lexer():
        m_in(NULL), m_lineno(0)
    { }

    Lexer(QTextStream *in):
        m_in(in), m_lineno(0)
    { }

    void setInputStream(QTextStream *in)
    {
        m_in = in;
        m_lineno = 0;
    }

    StatusCode lex();

    size_t addToken(tokid_t id, const QRegExp &re, bool ignore)
    {
        m_traits.append(TokenTrait(id, re));
        if (ignore)
            m_ignore.insert(id);
        return m_traits.size()-1;
    }

    size_t addToken(tokid_t id, const char *szRe, bool ignore = false)
    { return addToken(id, QRegExp(szRe), ignore); }

    size_t tokens()
    { return m_traits.size(); }

    void insertToken(size_t pos, tokid_t id, const QRegExp &re, bool ignore)
    {
        m_traits.insert(pos, TokenTrait(id, re));
        if (ignore)
            m_ignore.insert(id);
    }

    void insertToken(size_t pos, tokid_t id, const char *pszRe, bool ignore = false)
    { insertToken(pos, id, QRegExp(pszRe), ignore); }

    void removeByID(tokid_t id)
    {
        for (int i = 0, ie = m_traits.size(); i < ie; i++)
            if (m_traits[i].first == id)
                m_traits.remove(i);
        m_ignore.remove(id);
    }

    void removeByPos(size_t pos)
    {
        m_traits.remove(pos);
        m_ignore.remove(m_traits[pos].first);
    }

    void addBlockTrait(const QString &beg, const QString &end)
    {
        m_blockTraits.append(BlockTrait(beg, end));
    }

    void clear()
    {
        m_traits.clear();
        m_ignore.clear();
        m_blockTraits.clear();
    }

    size_t lineNo()
    { return m_lineno; }

    bool tokenValid() const
    { return m_lineno > 0; }

    const QString &tokenText() const
    { return m_lastText; }

    tokid_t tokenID() const
    { return m_lastId; }

private:
    typedef QPair<tokid_t, QRegExp> TokenTrait;
    typedef QPair<QString, QString> BlockTrait;

    /// Token traits
    QVector<TokenTrait> m_traits;

    /// A set of tokens to be skipped (not returned) by lexer
    QSet<tokid_t> m_ignore;

    /// Block traits (control the input)
    QVector<BlockTrait> m_blockTraits;

    /// Input buffer
    QString m_buf;

    /// Current input stream
    QTextStream *m_in;

    /// Current line number
    size_t m_lineno;

    /// Last read token ID
    tokid_t m_lastId;

    /// Last read token text
    QString m_lastText;
};

#endif // LEXER_H

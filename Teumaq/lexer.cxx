#include "lexer.h"

/** \brief Acquire next token.
 *
 * Reads one of previously specified tokens from the stream and
 * returns its ID, as well as textual representation, in arguments.
 * Status code reflects whether operation was succesful (and token
 * data is trustworthy) or an error occured. Errors are distinguished
 * using the status code returned.
 */
Lexer::StatusCode Lexer::lex()
{
    StatusCode status;
    QRegExp re;

    // Пополнить буфер, если это необходимо
    if (m_buf.isEmpty())
    {
        if (!m_in)
            return BAD_STREAM;

        QString line;
        do
        {
            if (m_in->atEnd())
                return EOS;
            line = m_in->readLine();
            m_lineno++;
        }
        while (line.isEmpty());
        m_buf.append(line);

        QVector<BlockTrait>::const_iterator itr;
        for (itr = m_blockTraits.begin(); itr != m_blockTraits.end(); ++itr)
        {
            // Block symbols are supposed to be found at the end of the string
            if (line.lastIndexOf(itr->first) >= 0)
            {
                while (line.indexOf(itr->second) < 0)
                {
                    line = m_in->readLine();
                    if (line.isNull())
                        return BLOCK_MISMATCH;
                    m_buf.append(line);
                }
                break;
            }
        }
    }

    // Идентифицировать следующую лексему в буфере и выбросить её
    int npos,
        minpos = m_buf.size();
    QVector<TokenTrait>::Iterator itr = m_traits.begin();
    while (itr != m_traits.end())
    {
        re = itr->second;
        npos = re.indexIn(m_buf);

        // Буфер начинается лексемой
        if (npos == 0)
        {
            if (m_ignore.contains(itr->first))
            {
                // Пропуск игнорируемой лексемы
                m_buf = m_buf.mid(re.matchedLength());

                // Заполнение буфера
                while (m_buf.isEmpty())
                {
                    m_buf = m_in->readLine();
                    if (m_buf.isNull())
                        return EOS;
                }

                // Перезапуск цикла
                minpos = m_buf.size();
                itr = m_traits.begin();
                continue;
            }
            else
            {
                // Обычная лексема
                m_lastId = itr->first;
                m_lastText = re.cap(0);
                m_buf = m_buf.mid(re.matchedLength()); // cut off previous data
                status = OK;
                break;
            }
        }
        // Буфер начинается не-лексемой - ошибка
        else
        {
            if (npos > 0 && npos < minpos)
                minpos = npos;
            status = BAD_TOKEN;
        }
        ++itr;
    }

    if (status == BAD_TOKEN)
    {
        // Сохранить ошибочную лексему
        m_lastText = m_buf.mid(0, minpos);
        m_buf = m_buf.mid(minpos);
    }

    return status;
}

#ifndef WIDGETGROUP_H
#define WIDGETGROUP_H

#include <QObject>
#include <QtAlgorithms>

template <typename WidgetTy, size_t SIZE>
class WidgetGroup: public QObject
{
public:
    WidgetGroup(QObject *owner = NULL):
        QObject(owner), _curpos(0)
    {
        qFill(_data, _data + SIZE, reinterpret_cast<WidgetTy*>(NULL));
    }
    void clear()
    {
        _curpos = 0;
    }
    int add(WidgetTy *w)
    {
        if (w && _curpos < SIZE)
        {
            _data[_curpos++] = w;
            return static_cast<int>(_curpos);
        }

        return -1;
    }
    WidgetGroup &operator <<(WidgetTy *w)
    {
        if (w && _curpos < SIZE)
            _data[_curpos++] = w;
        return *this;
    }
    bool set(size_t ndx, WidgetTy *w)
    {
        if (w && ndx < SIZE)
        {
            _data[ndx] = w;
            return true;
        }

        return false;
    }
    bool remove(WidgetTy *w)
    {
        if (w)
        {
            for (size_t i = 0; i < SIZE; i++)
                if (_data[i] == w)
                {
                    _data[i] == NULL;
                    return true;
                }
        }

        return false;
    }
    bool remove(size_t pos)
    {
        if (pos < SIZE)
        {
            _data[pos] = NULL;
            return true;
        }

        return false;
    }

    WidgetTy *widget(size_t index)
    {
        if (index < SIZE)
            return _data[index];
        else
            return NULL;
    }

    void setEnabled(bool enabled)
    {
        for (size_t i = 0; i < _curpos; i++)
            if (_data[i])
                _data[i]->setEnabled(enabled);
    }

private:
    WidgetTy* _data[SIZE];
    size_t _curpos;
};

#endif // WIDGETGROUP_H

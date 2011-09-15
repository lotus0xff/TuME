#include "tapeview.h"
#include <QResizeEvent>
#include <QPainter>
#include <QSizePolicy>


TapeView::TapeView(QWidget *parent, Machine *m) :
    QWidget(parent), _brCellBg(Qt::white),
    _brCellBgNI(Qt::lightGray), _clrCellPen(Qt::black),
    _clrText(Qt::black), _brCursor(Qt::green),
    _lvc(0), _rvc(0)
{
    setMachine(m);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::StrongFocus);
    _focus.setColor(Qt::blue);
    _focus.setStyle(Qt::DotLine);
}

void TapeView::rewindToCursor()
{
    if (canOperate())
    {
        _cur = _mach->cursor();
        setVisibleCells(_lvc+_rvc+1);
        update();
    }
}

void TapeView::rewindToLeft(bool inDeep)
{
    if (canView())
    {
        _mach->setToTapeFirst(_cur);
        if (inDeep)
        {
            _lvc += _rvc;
            _rvc = 0;
        }
        else
        {
            _rvc += _lvc;
            _lvc  = 0;
        }

        update();
    }
}

void TapeView::rewindToRight(bool inDeep)
{
    if (canView())
    {
        _mach->setToTapeLast(_cur);
        if (inDeep)
        {
            _rvc += _lvc;
            _lvc = 0;
        }
        else
        {
            _lvc += _rvc;
            _rvc = 0;
        }

        update();
    }
}

bool TapeView::shiftRight(size_t cells)
{
    if (!canView())
        return false;

    for (size_t i = 0; i < cells; i++)
    {
        // Check if machine is initialized
        if (!_mach->validCursor(_cur))
            return false;

        // Try to move cursor itself
        Machine::Cursor_RO old = _cur++;
        if (_mach->validCursor(_cur))
            continue;
        else
            _cur = old;

        /* Cursor is on the boundary. We can only
         * manipulate view parameters: increase
         * _rvc and decrease _lvc, until _lvc == 0.
         */
        if (_lvc > 0)
        {
            _lvc--;
            _rvc++;
        }
        else
            return false;
    }
    update();

    return true;
}

bool TapeView::shiftLeft(size_t cells)
{
    if (!canView())
        return false;

    // See shiftRight for detailed procedure description
    for (size_t i = 0; i < cells; i++)
    {
        if (!_mach->validCursor(_cur))
            return false;

        Machine::Cursor_RO old = _cur--;
        if (_mach->validCursor(_cur))
            continue;
        else
            _cur = old;

        if (_rvc > 0)
        {
            _lvc++;
            _rvc--;
        }
        else
            return false;
    }
    update();

    return true;
}

void TapeView::setMachine(Machine *m)
{
    _mach = m;

    // Initialize boundary cursors
    rewindToCursor();
}

void TapeView::setVisibleCells(size_t no)
{
    _rvc = no >> 1;
    _lvc = _rvc - (1 - no % 2);
}

void TapeView::resizeEvent(QResizeEvent *e)
{
    // Set cell size
    _cellBnd.setX(1);
    _cellBnd.setY(1);
    _cellBnd.setWidth(e->size().width() / (_lvc + _rvc + 1) - 1);
    _cellBnd.setHeight(e->size().height() * 2 / 3);
}

void TapeView::paintEvent(QPaintEvent *e)
{
    QPainter g(this);
    if (!canView())
    {
        g.setBrush(_brCellBgNI);
        g.setPen(_clrCellPen);
        QRect target = rect().adjusted(0,0,-1,-1);
        g.drawRect(target);
        g.setPen(_clrText);
        g.drawText(target, Qt::AlignCenter | Qt::TextWordWrap,
                   "MACHINE N/A");
        return;
    }

    Machine::Cursor_RO irunner = _mach->validCursor(_cur) ?
                                 _cur :
                                 _mach->initialCursor(),
                       runner = irunner;
    QRect cellcur = _cellBnd,
          cell;

    g.setPen(_clrCellPen);

    // Draw cell above cursor
    cellcur.moveLeft(cellcur.x() + (cellcur.width()+1) * _lvc);
    drawCell(g, runner, cellcur);

    // If it is that at which machine cursor points to,
    // draw the cursor symbol.
    if (runner == _mach->cursor())
    {
        cell = cellcur;
        cell.moveTop(cell.bottom()+1);
        cell.setBottom(height()-1);
        drawCursor(g, cell);
    }

    size_t i;   // Index

    // Draw the rest of the cells to right...
    cell = cellcur;
    for (i = 0; i < _rvc; i++)
    {
        cell.moveLeft(cell.right()+2);
        if (_mach->validCursor(runner))
            ++runner;
        drawCell(g, runner, cell);
        if (runner == _mach->cursor())
        {
            QRect cell4cur = cell;
            cell4cur.moveTop(cell4cur.bottom()+1);
            cell4cur.setBottom(height()-1);
            drawCursor(g, cell4cur);
        }
    }

    // Draw to the left...
    cell = cellcur;
    runner = irunner;
    for (i = 0; i < _lvc; i++)
    {
        cell.moveRight(cell.left()-2);
        if (_mach->validCursor(runner))
            --runner;
        drawCell(g, runner, cell);
        if (runner == _mach->cursor())
        {
            QRect cell4cur = cell;
            cell4cur.moveTop(cell4cur.bottom()+1);
            cell4cur.setBottom(height()-1);
            drawCursor(g, cell4cur);
        }
    }

    // Draw bounds if in focus
    if (hasFocus())
    {
        g.setPen(_focus);
        g.setBrush(Qt::NoBrush);
        g.drawRect(rect().adjusted(0, 0, -1, -1));
    }

}

void TapeView::drawCell(QPainter &g,
                        const Machine::Cursor_RO &cur,
                        const QRect &bounds)
{
    QFontMetrics fm = g.fontMetrics();
    QString symtext;
    if (_mach->validCursor(cur))
    {
        symtext = _mach->symbolName(*cur);
        g.setBrush(_brCellBg);
    }
    else
    {
        symtext = _mach->symbolName(Machine::SYMBOL_EMPTY);
        g.setBrush(_brCellBgNI);
    }
    int vlen = (bounds.width()-6) * (bounds.height()-6) / fm.height();
    symtext = fm.elidedText(symtext, Qt::ElideRight, vlen);
    g.setPen(_clrCellPen);
    g.drawRect(bounds);
    g.setPen(_clrText);
    g.drawText(bounds.adjusted(3, 3, -3, -3), Qt::TextWrapAnywhere | Qt::AlignCenter,
               symtext);
}

void TapeView::drawCursor(QPainter &g, QRect &cell)
{
    if (!_picCursor.isNull())
    {
        // Given a picture - use the scaled picture
        cell.setHeight(qMin(_picCursor.height(), cell.height()));
        int pw = _picCursor.width();
        if (pw < cell.width())
        {
            cell.setX(cell.x() + (cell.width()-pw) / 2);
            cell.setWidth(pw);
        }
        g.drawPixmap(cell, _picCursor);
    }
    else
    {
        // Otherwise use a polygon
        QPolygon poly;
        poly << QPoint(cell.width() / 2 + cell.left(), cell.top() + cell.height() / 3)
             << cell.bottomLeft()
             << cell.bottomRight();
        g.save();
        g.setBrush(_brCursor);
        g.drawPolygon(poly);
        g.restore();
    }
}

QSize TapeView::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    return QSize(fm.averageCharWidth() * 5,
                 fm.height() * 3);
}

void TapeView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Right:
        shiftRight();
        break;
    case Qt::Key_Left:
        shiftLeft();
        break;
    case Qt::Key_Up:
        rewindToRight();
        break;
    case Qt::Key_Down:
        rewindToLeft();
        break;
    case Qt::Key_Clear:
    case Qt::Key_Escape:
    case Qt::Key_Home:
        rewindToCursor();
        break;
    default:
        return;
    }
    update();
}

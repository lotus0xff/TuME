#ifndef TAPEVIEW_H
#define TAPEVIEW_H

#include <QWidget>
#include <QPixmap>
#include <QPen>
#include "machine.h"

class TapeView : public QWidget
{
    Q_OBJECT
public:
    explicit TapeView(QWidget *parent = NULL, Machine *m = NULL);
    void setMachine(Machine *m);
    const Machine *machine() const
    {
        return _mach;
    }
    void setVisibleCells(size_t no);
    size_t visibleCells() const
    {
        return _lvc + _rvc + 1;
    }
    size_t visibleCellsRight() const
    {
        return _rvc;
    }
    size_t visibleCellsLeft() const
    {
        return _lvc;
    }
    QPixmap &cursor()
    {
        return _picCursor;
    }
    const QPixmap &cursor() const
    {
        return _picCursor;
    }
    QBrush &cellBgBrush()
    {
        return _brCellBg;
    }
    const QBrush &cellBgBrush() const
    {
        return _brCellBg;
    }
    QBrush &inactiveCellBgBrush()
    {
        return _brCellBgNI;
    }
    const QBrush &inactiveCellBgBrush() const
    {
        return _brCellBgNI;
    }
    QColor &cellPenColor()
    {
        return _clrCellPen;
    }
    const QColor &cellPenColor() const
    {
        return _clrCellPen;
    }
    QColor &textColor()
    {
        return _clrText;
    }
    const QColor &textColor() const
    {
        return _clrText;
    }
    QBrush &cursorBrush()
    {
        return _brCursor;
    }
    const QBrush &cursorBrush() const
    {
        return _brCursor;
    }
    QPen &focusPen()
    {
        return _focus;
    }
    const QPen &focusPen() const
    {
        return _focus;
    }
    virtual int heightForWidth(int width) const;

public slots:
    /*! \brief Move cursor rightwards at specified number
     * of cells (1 by default).
     *
     * This method will rotate cells to the right until
     * the last "active" (currently in the list) will
     * be placed in the leftmost position.
     * \param cells Number of leftmost cells that will go
     * out of viewport (number of cells that will become visible
     * at the right side).
     * \return true, if rotation was successfull, false otherwise
     * (e.g. end of real tape reached).
     */
    bool shiftRight(size_t cells = 1);
    bool shiftLeft(size_t cells = 1);
    void rewindToCursor();
    void rewindToLeft(bool inDeep = false);
    void rewindToRight(bool inDeep = false);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual QSize sizeHint() const;
    bool canView() const
    {
        return _mach != NULL &&
               !_mach->states().isEmpty() &&
               !_mach->symbols().isEmpty() &&
               !_mach->tape().isEmpty();
    }
    bool canOperate() const
    {
        return canView() &&
               _mach->validCursor(_mach->cursor());
    }
    void drawCell(QPainter &g,
                  const Machine::Cursor_RO &cur,
                  const QRect &bounds);
    void drawCursor(QPainter &g,
                    QRect &cell);

private:
    /// Cell boundary template
    QRect _cellBnd;

    /// Display properties
    QBrush _brCellBg,
           _brCellBgNI,
           _brCursor;
    QColor _clrCellPen,
           _clrText;
    QPen _focus;

    /// Cursor image
    QPixmap _picCursor;

    /// Machine instance
    Machine *_mach;

    /// View bounds in terms of machine cursors
    Machine::Cursor_RO _cur;

    /// Left & right count of visible cells
    /// (excluding that one under cursor).
    size_t _lvc, _rvc;
};

#endif // TAPEVIEW_H

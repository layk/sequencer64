#ifndef EDITTIMEBAR_HPP
#define EDITTIMEBAR_HPP

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPen>

#include "sequence.hpp"

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq64
{
    class perform;

/**
 * The timebar for the sequence editor
 */

class qseqtime : public QWidget
{
    Q_OBJECT

public:
    explicit qseqtime(sequence *a_seq,
                      QWidget *parent);
    void zoom_in();
    void zoom_out();

protected:
    //override painting event to draw on the frame
    void paintEvent(QPaintEvent *);

    //override mouse events for interaction
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

    //override the sizehint to set our own defaults
    QSize sizeHint() const;

signals:

private slots:

private:
    sequence *m_seq;

    QTimer      *m_timer;
    QPen        *m_pen;
    QBrush      *m_brush;
    QPainter    *m_painter;
    QFont        m_font;

    int m_zoom;

};

}           // namespace seq64

#endif // EDITTIMEBAR_HPP

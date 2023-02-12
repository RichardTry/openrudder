#include "virtualanalogstick.h"
#include <QTouchEvent>
#include <QPainter>
#include <QDebug>
#include <QtMath>

VirtualAnalogStick::VirtualAnalogStick(const Button &btn, QWidget *parent) : m_btn(btn), QWidget(parent), m_touchPoint(this->rect().center()) {
    setAttribute(Qt::WA_AcceptTouchEvents);

    m_innerRadius = 50;
    m_outerRadius = 150;

    m_innerColor = QColor(32, 32, 32, 128);
    m_outerColor = QColor(255, 32, 32, 255);
    m_lineWidth = 10;

    m_timer.setInterval(25);
    m_timer.setSingleShot(true);
    QWidget::connect(&m_timer, &QTimer::timeout, this, QOverload <>::of (&QWidget::repaint));
    m_timer.start();
}

VirtualAnalogStick::~VirtualAnalogStick() {
    m_timer.stop();
}

bool VirtualAnalogStick::event(QEvent *event) {
    QEvent::Type eventType = event->type();

    if(eventType == QEvent::Resize) {
        const QResizeEvent *resizeEvent = static_cast <const QResizeEvent *> (event);
        const qreal w = resizeEvent->size().width();
        const qreal h = resizeEvent->size().height();
        qreal ri = (w < h ? w : h) / 2;
        ri *= innerRadius()/outerRadius();
        setInnerRadius(ri);
        qreal ro = (w < h ? w : h) / 2;
        setOuterRadius(ro);
        setMask(QRegion(QRect(QPoint(0, 0), resizeEvent->size()), QRegion::Ellipse));
        m_touchPoint = QPointF(w/2, h/2);
        m_timer.start();
        return QWidget::event(event);
    }

    if(eventType != QEvent::TouchBegin && eventType != QEvent::TouchUpdate && eventType != QEvent::TouchEnd && eventType != QEvent::TouchCancel) {
        return QWidget::event(event);
    }

    if(!m_timer.isActive()) {
        m_timer.start();
    }

    const QTouchEvent *touchEvent = static_cast <const QTouchEvent*> (event);

    switch (event->type()) {
        case QEvent::TouchEnd: {
            emit released(m_btn, normalisedTouchPoint());
            m_touchPoint = this->rect().center();
            return true;
        }
        case QEvent::TouchCancel: {
            emit released(m_btn, normalisedTouchPoint());
            m_touchPoint = this->rect().center();
            return true;
        }
        default: {
            const QPointF rawTouchPoint = touchEvent->touchPoints().first().pos();
            const QPoint origin = this->rect().center();
            const qreal maxRadius = m_outerRadius - m_innerRadius;
            if((rawTouchPoint - origin).manhattanLength() > maxRadius) {
                const QLineF line(origin, rawTouchPoint.toPoint());
                const double lineAngle = qDegreesToRadians(360-line.angle());
                m_touchPoint.setY(qSin(lineAngle) * maxRadius + origin.y());
                m_touchPoint.setX(qCos(lineAngle) * maxRadius + origin.x());
            } else {
                m_touchPoint = rawTouchPoint;
            }

            if(event->type() == QEvent::TouchBegin)
                emit pressed(m_btn, normalisedTouchPoint());
            else if(event->type() == QEvent::TouchUpdate)
                emit moved(m_btn, normalisedTouchPoint());

            return true;
        }
    }

}

QPointF VirtualAnalogStick::touchPoint() const {
    return m_touchPoint;
}

inline QPointF VirtualAnalogStick::normalisedTouchPoint() const {
    return (m_touchPoint - rect().center())/(m_outerRadius - m_innerRadius);
}

void VirtualAnalogStick::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    QPen pen = painter.pen();
    pen.setWidth(m_lineWidth);
    const QPointF origin = this->rect().center();

    // Outer circle
    pen.setColor(m_outerColor);
    painter.setPen(pen);
    const int ro = m_outerRadius - m_lineWidth/2.f;
    painter.drawEllipse(origin, ro, ro);

    // Inner circle
    pen.setColor(m_innerColor);
    painter.setPen(pen);
    const int ri = m_innerRadius - m_lineWidth/2.f;
    painter.drawEllipse(m_touchPoint, ri, ri);

//#if defined (QT_DEBUG)
//    // Draw x axis positive side
//    pen.setColor(QColor(0, 255, 0));
//    painter.setPen(pen);
//    painter.drawLine(QLineF(origin + QPoint(m_lineWidth, 0), origin + QPoint(m_outerRadius, 0)));
//    // Label axis
//    QFontMetrics fm(painter.font());
//    int textWidth = fm.horizontalAdvance("+x");
//    painter.drawText(origin + QPoint(m_outerRadius - textWidth, -20), "+x");

//    // Draw y axis positive side
//    pen.setColor(QColor(255, 0, 0));
//    painter.setPen(pen);
//    painter.drawLine(QLineF(origin + QPoint(0, m_lineWidth), origin + QPoint(0, m_outerRadius)));
//    // Label axis
//    int textHeight = fm.boundingRect("+y").height();
//    painter.drawText(origin + QPoint(-textHeight, m_outerRadius), "+y");

//    // Draw line from origin to touch point's center
//    pen.setColor(QColor(0, 0, 0));
//    pen.setWidth(m_lineWidth/2 > 0 ? m_lineWidth/2 : 1);
//    painter.setPen(pen);
//    painter.drawLine(QLineF(origin, m_touchPoint.toPoint()));
//#endif
}

int VirtualAnalogStick::heightForWidth(int w) const {
    return w;
}

qreal VirtualAnalogStick::innerRadius() const
{
    return m_innerRadius;
}

void VirtualAnalogStick::setInnerRadius(const qreal &innerRadius)
{
    m_innerRadius = innerRadius;
}

qreal VirtualAnalogStick::outerRadius() const
{
    return m_outerRadius;
}

void VirtualAnalogStick::setOuterRadius(const qreal &outerRadius)
{
    m_outerRadius = outerRadius;
}

QColor VirtualAnalogStick::outerColor() const
{
    return m_outerColor;
}

void VirtualAnalogStick::setOuterColor(const QColor &outerColor)
{
    m_outerColor = outerColor;
}

QColor VirtualAnalogStick::innerColor() const
{
    return m_innerColor;
}

void VirtualAnalogStick::setInnerColor(const QColor &innerColor)
{
    m_innerColor = innerColor;
}

QSize VirtualAnalogStick::minimumSizeHint() const {
    return QSize(m_outerRadius * 2, m_outerRadius * 2);
}

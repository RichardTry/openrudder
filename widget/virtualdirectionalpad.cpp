#include "virtualdirectionalpad.h"
#include <QEvent>
#include <QTouchEvent>
#include <QtMath>
#include <QPainter>
#include <QDebug>

VirtualDirectionalPad::VirtualDirectionalPad(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_AcceptTouchEvents);

    m_increment = 90;
    m_startAngle = 0;
    m_endAngle = 360;
    m_offset = -m_increment/2;

    m_outerRadius = 100;
    m_innerRadius = m_outerRadius/3;
    m_pressedButtons = Button::DPAD;

    // Transitional region [315, 45] broken up into two pieces because no positive angle could between these two values
    m_regions.push_back(DpadRegion(315, 360, Button::RIGHT));
    m_regions.push_back(DpadRegion(0, 45, Button::RIGHT));
    // Rest are all positive values
    m_regions.push_back(DpadRegion(1 * m_increment + m_offset, 2 * m_increment + m_offset, Button::DOWN));
    m_regions.push_back(DpadRegion(2 * m_increment + m_offset, 3 * m_increment + m_offset, Button::LEFT));
    m_regions.push_back(DpadRegion(3 * m_increment + m_offset, 4 * m_increment + m_offset, Button::UP));
}

VirtualDirectionalPad::~VirtualDirectionalPad() {
}

QSize VirtualDirectionalPad::minimumSizeHint() const {
    return QSize(100, 100);
}

bool VirtualDirectionalPad::event(QEvent *event) {
    QEvent::Type eventType = event->type();

    if(eventType == QEvent::Resize) {
        const QResizeEvent *resizeEvent = static_cast <const QResizeEvent*> (event);
        m_outerRadius = sqrt(resizeEvent->size().width() * resizeEvent->size().width());
        m_innerRadius = resizeEvent->size().width()/6;
        return QWidget::event(event);
    }

    if(eventType != QEvent::TouchBegin && eventType != QEvent::TouchUpdate && eventType != QEvent::TouchEnd && eventType != QEvent::TouchCancel) {
        return QWidget::event(event);
    }

    const QTouchEvent *touchEvent = static_cast <const QTouchEvent*> (event);

    if(touchEvent->target() != this) {
        return QWidget::event(event);
    }

    if(!m_timer.isActive()) {
        m_timer.start();
    }

    // DETERMINE WHICH REGION WAS TOUCHED
    const QPointF rawTouchPoint = touchEvent->touchPoints().first().pos();
    const QPoint origin = this->rect().center();
    const QLineF line(origin, rawTouchPoint.toPoint());
    const qreal lineAngle = 360-line.angle();
    for(DpadRegion region: m_regions) {
        if(region.startAngle < lineAngle && lineAngle <= region.endAngle) {
            m_pressedButtons = region.pressedButtons;
            break;
        }
    }

    if(m_pressedButtons != m_pressedButtonsPrevious)
        repaint();
    m_pressedButtonsPrevious = m_pressedButtons;

    switch (event->type()) {
        case QEvent::TouchEnd: {
            if(m_pressedButtons != Button::DPAD)
                emit released((Button)m_pressedButtons);
            m_pressedButtons = Button::DPAD;
            repaint();
            return true;
        }
        case QEvent::TouchCancel: {
            if(m_pressedButtons != Button::DPAD)
                emit released((Button)m_pressedButtons);
            m_pressedButtons = Button::DPAD;
            repaint();
            return true;
        }
        case QEvent::TouchBegin: {
            if(m_pressedButtons != Button::DPAD)
                emit pressed((Button)m_pressedButtons);
            repaint();
            return true;
        }
        default: {
            return true;
        }
    }
}

void VirtualDirectionalPad::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    QSharedPointer<QPixmap> pixmap = Common::buttonIcon(m_pressedButtons, rect().size());
    if(!pixmap.isNull())
        painter.drawPixmap(rect(), *pixmap);
}

int VirtualDirectionalPad::heightForWidth(int w) const {
    return w;
}

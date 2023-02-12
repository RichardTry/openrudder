#include "virtualgamepadbutton.h"
#include <QPixmap>
#include <QPainter>
#include <QTouchEvent>
#include <QResizeEvent>
#include <QEvent>

VirtualGamepadButton::VirtualGamepadButton(const Button &btn, QWidget *parent) : QWidget(parent), m_button(btn), m_pressed(false) {
    setAttribute(Qt::WA_AcceptTouchEvents);
    m_pressedScale = 0.8;
    m_releasedScale = 0.9;
}

VirtualGamepadButton::~VirtualGamepadButton() {
}

bool VirtualGamepadButton::event(QEvent *event) {
    const QEvent::Type eventType = event->type();

    if(eventType == QEvent::Resize) {
        const QResizeEvent *resizeEvent = static_cast <const QResizeEvent*> (event);
        // Get icons
        m_pixmap = Common::buttonIcon(m_button, resizeEvent->size() * m_releasedScale);
        m_pixmapPressed = Common::buttonIcon(m_button, resizeEvent->size() * m_pressedScale);
        // Set rects
        const QRect rect = QRect(QPoint(0, 0), resizeEvent->size());
        {
            const int sX = rect.x() + rect.width() * (1 - m_releasedScale) * 0.5;
            const int sY = rect.y() + rect.height() * (1 - m_releasedScale) * 0.5;
            const int sW = rect.width() * m_releasedScale;
            const int sH = rect.height() * m_releasedScale;
            m_releasedRect = QRect(sX, sY, sW, sH);
        }
        {
            const int sX = rect.x() + rect.width() * (1 - m_pressedScale) * 0.5;
            const int sY = rect.y() + rect.height() * (1 - m_pressedScale) * 0.5;
            const int sW = rect.width() * m_pressedScale;
            const int sH = rect.height() * m_pressedScale;
            m_pressedRect = QRect(sX, sY, sW, sH);
        }

        // Return base implementation result
        return QWidget::event(event);
    }

    if(eventType != QEvent::TouchBegin && eventType != QEvent::TouchUpdate && eventType != QEvent::TouchEnd && eventType != QEvent::TouchCancel) {
        return QWidget::event(event);
    }

//    const QTouchEvent *touchEvent = static_cast <const QTouchEvent*> (event);

    switch (event->type()) {
        case QEvent::TouchEnd: {
            emit released(m_button);
            m_pressed = false;
            repaint();
            return true;
        }
        case QEvent::TouchCancel: {
            m_pressed = false;
            repaint();
            return true;
        }
        case QEvent::TouchBegin: {
            emit pressed(m_button);
            m_pressed = true;
            repaint();
            return true;
        }
        default: {
            return true;
        }
    }

}

void VirtualGamepadButton::paintEvent(QPaintEvent *event) {
    if(m_pixmap.isNull())
        return;

    QPainter painter(this);
    if(!m_pressed) {
        painter.drawPixmap(m_releasedRect, *m_pixmap);
    }
    else {
        painter.drawPixmap(m_pressedRect, *m_pixmapPressed);
    }
}

int VirtualGamepadButton::heightForWidth(int w) const {
    return w;
}

QSize VirtualGamepadButton::minimumSizeHint() const {
    return QSize(50, 50);
}

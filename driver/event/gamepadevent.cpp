#include "gamepadevent.h"
#include <QDataStream>

GamepadEvent::GamepadEvent(const Type &type, const Button &btn, const QPointF &value): m_type(type), m_button(btn), m_value(value) {

}

GamepadEvent::GamepadEvent(const QByteArray &data) {
    QByteArray dt = data;
    QDataStream in(&dt, QIODevice::ReadOnly);

    in >> m_type;
    in >> m_button;
    if(m_type != GamepadEvent::ButtonPressEvent && m_type != GamepadEvent::ButtonReleaseEvent)
        in >> m_value;
}

QByteArray GamepadEvent::data() const {
    QByteArray dt;
    QDataStream out(&dt, QIODevice::WriteOnly);

    out << m_type;
    out << m_button;
    if(m_type != GamepadEvent::ButtonPressEvent && m_type != GamepadEvent::ButtonReleaseEvent)
        out << m_value;

    return dt;
}

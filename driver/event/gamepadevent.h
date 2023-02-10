#ifndef GAMEPADEVENT_H
#define GAMEPADEVENT_H

//#include <QByteArray>
//#include <QPointF>
#include "common/common.h"

struct GamepadEvent {
    enum Type {
        DummyEvent,
        ButtonPressEvent,
        ButtonReleaseEvent,
        StickMoveEvent,
        StickPressEvent,
        StickReleaseEvent,
    };

    GamepadEvent(const Type &type, const Button &btn, const QPointF &value = QPointF());
    GamepadEvent(const QByteArray &data);
    QByteArray data() const;

    Type m_type;
    Button m_button;
    QPointF m_value;
};


#endif // GAMEPADEVENT_H

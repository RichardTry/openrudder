#ifndef VIRTUALDIRECTIONALPAD_H
#define VIRTUALDIRECTIONALPAD_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QMap>
#include "common/common.h"

class VirtualDirectionalPad : public QWidget {
    Q_OBJECT
public:
    explicit VirtualDirectionalPad(QWidget *parent = nullptr);
    ~VirtualDirectionalPad();

    QSize minimumSizeHint() const override;

signals:
    void pressed(Button buttons);
    void released(Button buttons);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    int heightForWidth(int) const override;

private:
    QTimer m_timer;
    QPointF m_touchPoint;

    // Paramaters defining the click regions
    int m_outerRadius;
    int m_innerRadius;
    qreal m_increment;
    qreal m_startAngle;
    qreal m_endAngle;
    qreal m_offset;

    struct DpadRegion {
        DpadRegion(qreal startAngle, qreal endAngle, quint32 pressedButtons): startAngle(startAngle), endAngle(endAngle), pressedButtons(pressedButtons) {
        }
        qreal startAngle;
        qreal endAngle;
        quint32 pressedButtons;
    };
    QVector <DpadRegion> m_regions;

    // Button state
    quint32 m_pressedButtons;
    quint32 m_pressedButtonsPrevious;
};

#endif // VIRTUALDIRECTIONALPAD_H

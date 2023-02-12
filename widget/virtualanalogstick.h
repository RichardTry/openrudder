#ifndef VIRTUALANALOGSTICK_H
#define VIRTUALANALOGSTICK_H

#include "common/common.h"
#include <QWidget>
#include <QTimer>

class VirtualAnalogStick : public QWidget {
    Q_OBJECT

public:
    explicit VirtualAnalogStick(const Button &btn, QWidget *parent = nullptr);
    ~VirtualAnalogStick();


    QPointF touchPoint() const;
    QPointF normalisedTouchPoint() const;

    QColor innerColor() const;
    void setInnerColor(const QColor &innerColor);

    QColor outerColor() const;
    void setOuterColor(const QColor &outerColor);

    QSize minimumSizeHint() const override;

    qreal outerRadius() const;
    void setOuterRadius(const qreal &outerRadius);

    qreal innerRadius() const;
    void setInnerRadius(const qreal &innerRadius);

signals:
    void moved(Button btn, QPointF normalisedTouchPoint);
    void pressed(Button btn, QPointF normalisedTouchPoint);
    void released(Button btn, QPointF normalisedTouchPoint);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    int heightForWidth(int) const override;

private:
    Button m_btn;
    qreal m_outerRadius;
    qreal m_innerRadius;

    qreal m_lineWidth;
    QColor m_innerColor;
    QColor m_outerColor;

    QPointF m_touchPoint;
    QRect m_innerRect;

    QTimer m_timer;
};

#endif // VIRTUALANALOGSTICK_H

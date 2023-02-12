#ifndef VIRTUALGAMEPADBUTTON_H
#define VIRTUALGAMEPADBUTTON_H

#include <QWidget>
#include "common/common.h"

class VirtualGamepadButton : public QWidget {
    Q_OBJECT
public:
    explicit VirtualGamepadButton(const Button &btn, QWidget *parent = nullptr);
    ~VirtualGamepadButton();

signals:
    void pressed(const Button &btn);
    void released(const Button &btn);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    int heightForWidth(int) const override;
    QSize minimumSizeHint() const override;

private:
    bool m_pressed;
    Button m_button;
    QSharedPointer <QPixmap> m_pixmap;
    QSharedPointer <QPixmap> m_pixmapPressed;
    qreal m_pressedScale;
    qreal m_releasedScale;
    QRect m_pressedRect;
    QRect m_releasedRect;
};

#endif // VIRTUALGAMEPADBUTTON_H

#ifndef NETWORKTRANSCEIVERWIDGET_H
#define NETWORKTRANSCEIVERWIDGET_H

//#include <QWidget>
#include "transceiver/networktransceiver.h"

Q_DECLARE_METATYPE(QHostAddress)

namespace Ui {
class NetworkTransceiverMaster;
class NetworkTransceiverSlave;
}

class NetworkTransceiverWidget : public QWidget {
    Q_OBJECT
public:
    explicit NetworkTransceiverWidget(NetworkTransceiver *transceiver, QWidget *parent = nullptr);

signals:

public slots:
    void onStateChanged(NetworkTransceiver::State state);
    void onHostFound(QString address);

private:
    void loadMasterUI();
    void loadSlaveUI();

    NetworkTransceiver *m_transceiver;
    QList <QHostAddress> m_interfaces;
    // Different ui's loaded for each mode
    Ui::NetworkTransceiverMaster *masterUi;
    Ui::NetworkTransceiverSlave *slaveUi;
    QSharedPointer<QPixmap> m_logoPixmap;
    class StatusAnimation;
    StatusAnimation *m_broadcastAnimation;
    StatusAnimation *m_receiveAnimation;
};

class NetworkTransceiverWidget::StatusAnimation: public QWidget {
    Q_OBJECT
public:
    enum Status {
        Idle,
        Broadcast,
        ReceiveInput,
    };

    StatusAnimation(const Status status, NetworkTransceiverWidget *transWidget);

    void paintEvent(QPaintEvent *event) override;

public slots:
    void stop();
    void start();

private slots:
    void onTimeOut();
    void resizeEvent(QResizeEvent *event) override;

private:
    void loadPixmaps();
    QTimer m_pixmapLoadTimer;
    Status m_status;
    NetworkTransceiverWidget *m_transWidget;
    // Animation data
    int m_currentPixmapIndex;
    std::vector <QSharedPointer<QPixmap>> m_pixmap;
    QTimer m_timer;
    int m_periodms;
};

#endif // NETWORKTRANSCEIVERWIDGET_H

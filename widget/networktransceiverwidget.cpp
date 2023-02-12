#include <QNetworkInterface>
#include <QMessageBox>
#include <QPainter>
#include "common/common.h"
#include "networktransceiverwidget.h"
#include "ui_networktransceivermaster.h"
#include "ui_networktransceiverslave.h"

NetworkTransceiverWidget::NetworkTransceiverWidget(NetworkTransceiver *transceiver, QWidget *parent) : QWidget(parent), m_transceiver(transceiver) {
    // Get available interfaces
    for(const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if(address.isLoopback() || address.isNull())
            continue;
        else
            m_interfaces.push_back(address);
    }

    // Load initial state and ui
    if(transceiver->mode() == AbstractTransceiver::Mode::Master) {
        loadMasterUI();
    } else if(transceiver->mode() == AbstractTransceiver::Mode::Slave) {
        loadSlaveUI();
    }

    connect(m_transceiver, &NetworkTransceiver::hostFound, this, &NetworkTransceiverWidget::onHostFound);
    connect(m_transceiver, &NetworkTransceiver::stateChanged, this, &NetworkTransceiverWidget::onStateChanged);
}

void NetworkTransceiverWidget::loadMasterUI() {
    masterUi = new Ui::NetworkTransceiverMaster();
    masterUi->setupUi(this);
    int logoSize = width() < height() ? width() * 0.5 : height() * 0.5;
    m_logoPixmap = Common::renderSvg(":/emulator-logo.svg", QSize(logoSize, logoSize));
    masterUi->logoLabel->setPixmap(*m_logoPixmap);

    // INIT
    connect(masterUi->startPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStart);
    connect(masterUi->backPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::closeCalled);
    masterUi->networkInterfaceComboBox->addItem("", QVariant::fromValue <QHostAddress> (QHostAddress::Null));
    for(const QHostAddress &address: m_interfaces) {
        masterUi->networkInterfaceComboBox->addItem(address.toString(), QVariant::fromValue <QHostAddress> (address));
    }
    connect(masterUi->networkInterfaceComboBox, QOverload <int>::of (&QComboBox::activated), [this] (int index) {
        const QHostAddress selectedInterface = masterUi->networkInterfaceComboBox->itemData(index).value <QHostAddress> ();
        m_transceiver->setSelectedInterface(selectedInterface);
    });

    // LISTEN
    connect(masterUi->connectPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStart);
    connect(masterUi->backPushButton2, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStop);
    connect(masterUi->hostListWidget, &QListWidget::itemClicked, [this] (QListWidgetItem *item) {
        QVariant data = item->data(Qt::UserRole);
        if(!data.canConvert <QHostAddress> ()) {
            QMessageBox messageBox;
            messageBox.setText(tr("Error, can't convert QVariant to QHostAddress"));
            messageBox.setWindowTitle(tr("Error"));
            messageBox.exec();
        } else {
            const QHostAddress slaveHost = data.value <QHostAddress> ();
            m_transceiver->setSlaveHost(slaveHost);
        }
    });

    // SEND INPUT
    connect(masterUi->stopSendingPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStop);
}

void NetworkTransceiverWidget::loadSlaveUI() {
    slaveUi = new Ui::NetworkTransceiverSlave();
    slaveUi->setupUi(this);
    int logoSize = width() < height() ? width() * 0.75 : height() * 0.75;
    m_logoPixmap = Common::renderSvg(":/emulator-logo.svg", QSize(logoSize, logoSize));
    slaveUi->logoLabel->setPixmap(*m_logoPixmap);

    m_broadcastAnimation = new StatusAnimation(StatusAnimation::Broadcast, this);
    m_receiveAnimation = new StatusAnimation(StatusAnimation::ReceiveInput, this);
    slaveUi->broadcastAnimationLayout->addWidget(m_broadcastAnimation);
    slaveUi->receiveInputAnimationLayout->addWidget(m_receiveAnimation);

    // INIT
    connect(slaveUi->startPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStart);
    connect(slaveUi->quitPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::closeCalled);
    slaveUi->networkInterfaceComboBox->clear();
    slaveUi->networkInterfaceComboBox->addItem("", QVariant::fromValue <QHostAddress> (QHostAddress::Null));
    for(const QHostAddress &address: m_interfaces) {
        slaveUi->networkInterfaceComboBox->addItem(address.toString(), QVariant::fromValue <QHostAddress> (address));
    }
    connect(slaveUi->networkInterfaceComboBox, QOverload <int>::of (&QComboBox::activated), [this] (int index) {
        const QHostAddress selectedInterface = slaveUi->networkInterfaceComboBox->itemData(index).value <QHostAddress> ();
        m_transceiver->setSelectedInterface(selectedInterface);
    });

    // BROADCAST
    connect(slaveUi->stopBroadcastPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStop);

    // RECEIVE INPUT
    connect(slaveUi->stopReceivingPushButton, &QPushButton::clicked, m_transceiver, &NetworkTransceiver::onStop);
}

void NetworkTransceiverWidget::onStateChanged(NetworkTransceiver::State state) {
    // Clear host list on every state change
    if(m_transceiver->mode() == NetworkTransceiver::Master)
        masterUi->hostListWidget->clear();

    switch (state) {
    // MASTER STATES
    case NetworkTransceiver::State::InitMaster: {
        masterUi->stackedWidget->setCurrentWidget(masterUi->stateMasterInit);
        break;
    }
    case NetworkTransceiver::State::Listen: {
        masterUi->stackedWidget->setCurrentWidget(masterUi->stateListen);
        break;
    }
    case NetworkTransceiver::State::SendInput: {
        masterUi->stackedWidget->setCurrentWidget(masterUi->stateSendnput);
        break;
    }
    // SLAVE STATES
    case NetworkTransceiver::State::InitSlave: {
        slaveUi->stackedWidget->setCurrentWidget(slaveUi->StateInitSlave);
        setWindowTitle(tr("Setup"));
        break;
    }
    case NetworkTransceiver::State::Broadcast: {
        slaveUi->stackedWidget->setCurrentWidget(slaveUi->StateBroadcast);
        setWindowTitle(tr("Broadcasting"));
        m_broadcastAnimation->start();
        break;
    }
    case NetworkTransceiver::State::ReceiveInput: {
        slaveUi->stackedWidget->setCurrentWidget(slaveUi->StateReceiveInput);
        setWindowTitle(tr("Receiving Input"));
        m_receiveAnimation->start();
        break;
    }
    }
}

void NetworkTransceiverWidget::onHostFound(QString address) {
    const QHostAddress hostAddress(address);
    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::UserRole, QVariant::fromValue <QHostAddress> (hostAddress));
    item->setText(address);
    masterUi->hostListWidget->addItem(item);
}

NetworkTransceiverWidget::StatusAnimation::StatusAnimation(const Status status, NetworkTransceiverWidget *transWidget):
QWidget(transWidget), m_status(status), m_transWidget(transWidget), m_currentPixmapIndex(0) {
    connect(&m_timer, &QTimer::timeout, this, &StatusAnimation::onTimeOut);
    if(status == Status::Broadcast)
        m_periodms = 100;
    else if (status == Status::ReceiveInput)
        m_periodms = 750;
    m_pixmapLoadTimer.setInterval(1000);
    m_pixmapLoadTimer.setSingleShot(true);
    connect(&m_pixmapLoadTimer, &QTimer::timeout, this, &StatusAnimation::loadPixmaps);
}

void NetworkTransceiverWidget::StatusAnimation::onTimeOut() {
    ++m_currentPixmapIndex;
    m_currentPixmapIndex %= m_pixmap.size();
    repaint();
}

void NetworkTransceiverWidget::StatusAnimation::paintEvent(QPaintEvent *event) {
    if(m_pixmap.empty())
        loadPixmaps();
    QPixmap *pixmap = m_pixmap[m_currentPixmapIndex].data();
    const int sX = rect().x() + (rect().width() - pixmap->width())/2;
    const int sY = rect().y() + (rect().height() - pixmap->height())/2;
    const int sW = pixmap->width();
    const int sH = pixmap->height();
    QPainter painter(this);
    painter.drawPixmap(QRect(sX, sY, sW, sH), *pixmap);
}

void NetworkTransceiverWidget::StatusAnimation::stop() {
    m_timer.stop();
}

void NetworkTransceiverWidget::StatusAnimation::start() {
    m_timer.start(m_periodms);
}

void NetworkTransceiverWidget::StatusAnimation::loadPixmaps() {
    const QSize imageSize = this->size() * 0.5;
    m_pixmap.clear();
    if(m_status == Status::Broadcast) {
        for(int i = 1; i <= 11; ++i)
            m_pixmap.push_back(Common::renderSvg(":/nm-stage01-connecting" + QString::number(i).rightJustified(2, '0') +  ".svg", imageSize));
    } else if(m_status == Status::ReceiveInput) {
        m_pixmap.push_back(Common::renderSvg(":/network-receive.svg", imageSize));
        m_pixmap.push_back(Common::renderSvg(":/network-transmit-receive.svg", imageSize));
        m_pixmap.push_back(Common::renderSvg(":/network-transmit.svg", imageSize));
    }
}

void NetworkTransceiverWidget::StatusAnimation::resizeEvent(QResizeEvent *event) {
    if(isVisible())
        m_pixmapLoadTimer.start();
}

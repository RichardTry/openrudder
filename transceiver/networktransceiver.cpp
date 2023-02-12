#include "networktransceiver.h"
#include <QThreadPool>
#include <QWidget>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QDataStream>

unsigned int NetworkTransceiver::m_datagramId = 0;

NetworkTransceiver::NetworkTransceiver(const Mode &mode, QObject *parent):
    AbstractTransceiver(mode, parent),
    m_port(45800),
    m_selectedInterface(QHostAddress::Null),
    m_slaveHost(QHostAddress::Null),
    m_masterHost(QHostAddress::Null)
{
    if(m_mode == Mode::Master) {
        m_state = new StateInitMaster(this);
    }
    else if (m_mode == Mode::Slave) {
        m_state = new StateInitSlave(this);
    }

    // Initilize udp socket and make connections
    m_udpSocket = new QUdpSocket();
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkTransceiver::onReadyRead);
}

NetworkTransceiver::~NetworkTransceiver() {
    delete m_state;
}

qint64 NetworkTransceiver::sendData(const QByteArray &data, const bool &acknowledge) {
    return m_state->sendData(data, acknowledge);
}

void NetworkTransceiver::onStart() {
    AbstractState *nextState = m_state->start();
    if(nextState) {
        delete m_state;
        m_state = nextState;
    }
}

void NetworkTransceiver::onStop() {
    AbstractState *nextState = m_state->stop();
    if(nextState) {
        delete m_state;
        m_state = nextState;
    }
}

void NetworkTransceiver::onReadyRead() {
    AbstractState *nextState = m_state->onReadyRead();
    if(nextState) {
        delete m_state;
        m_state = nextState;
    }
}

void NetworkTransceiver::setSlaveHost(const QHostAddress &slaveHost)
{
    m_slaveHost = slaveHost;
}

void NetworkTransceiver::setSelectedInterface(const QHostAddress &selectedInterface)
{
    m_selectedInterface = selectedInterface;
}



// MASTER INITIAL
NetworkTransceiver::StateInitMaster::StateInitMaster(NetworkTransceiver *transceiver): AbstractState(transceiver) {
    m_transceiver->emit stateChanged(State::InitMaster);
}

NetworkTransceiver::StateInitMaster::~StateInitMaster() {

}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateInitMaster::start() {

    if(m_transceiver->m_selectedInterface.isNull()) {
        m_transceiver->emit error(tr("Error, no interface selected!"));
        return nullptr;
    }

    m_transceiver->m_udpSocket->close();
    if(!m_transceiver->m_udpSocket->bind(QHostAddress::Any, m_transceiver->m_port)) {
//    if(!m_transceiver->m_udpSocket->bind(m_transceiver->m_selectedInterface, m_transceiver->m_port)) {
        m_transceiver->emit error(tr("Error binding socket to host: ") + m_transceiver->m_selectedInterface.toString() + tr(", port: ") + QString::number(m_transceiver->m_port));
        return nullptr;
    }

    return new StateListen(m_transceiver);
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateInitMaster::stop() {
    m_transceiver->closeCalled();
    return nullptr;
}

qint64 NetworkTransceiver::StateInitMaster::sendData(const QByteArray &data, const bool &acknowledge) {
    return -1;
}

// MASTER LISTEN
NetworkTransceiver::StateListen::StateListen(NetworkTransceiver *transceiver): AbstractState(transceiver) {
    transceiver->emit stateChanged(State::Listen);
}

NetworkTransceiver::StateListen::~StateListen() {
    m_transceiver->m_slaveHost = QHostAddress::Null;
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateListen::start() {
    if(m_transceiver->m_slaveHost.isNull()) {
        m_transceiver->emit error(tr("Error, no target device selected!"));
        return nullptr;
    }
    else {
        return new StateSendInput(m_transceiver);
    }
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateListen::stop() {
    return new StateInitMaster(m_transceiver);
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateListen::onReadyRead() {
    QNetworkDatagram datagram = m_transceiver->m_udpSocket->receiveDatagram();
    if(!m_hosts.contains(datagram.senderAddress().toIPv4Address())) {
        m_hosts[datagram.senderAddress().toIPv4Address()] = datagram.senderAddress();
        m_transceiver->emit hostFound(datagram.senderAddress().toString());
    }
    return nullptr;
}

qint64 NetworkTransceiver::StateListen::sendData(const QByteArray &data, const bool &acknowledge) {
    return -1;
}

// MASTER SEND INPUT
NetworkTransceiver::StateSendInput::StateSendInput(NetworkTransceiver *transceiver): AbstractState(transceiver) {
    m_transceiver->emit stateChanged(State::SendInput);
    m_transceiver->connected();
    m_transceiver->m_udpSocket->connectToHost(m_transceiver->m_slaveHost, m_transceiver->m_port);
}

NetworkTransceiver::StateSendInput::~StateSendInput() {
    m_transceiver->m_udpSocket->disconnectFromHost();
    m_transceiver->disconnected("Disconnected by user");
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateSendInput::start() {
    return nullptr;
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateSendInput::stop() {
    return new StateInitMaster(m_transceiver);
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateSendInput::onReadyRead() {
    QNetworkDatagram datagram = m_transceiver->m_udpSocket->receiveDatagram();
    QByteArray data = datagram.data();
    QDataStream in(&data, QIODevice::ReadOnly);
    QString str;
    in << str;
    if(str == "quit") {
        return new StateListen(m_transceiver);
    } else
        return nullptr;
}

qint64 NetworkTransceiver::StateSendInput::sendData(const QByteArray &data, const bool &acknowledge) {
//    QNetworkDatagram datagram;
//    datagram.setData(data);
//    datagram.setSender(m_transceiver->m_selectedInterface, m_transceiver->m_port);
//    datagram.setDestination(m_transceiver->m_slaveHost, m_transceiver->m_port);
//    m_transceiver->m_udpSocket->connectToHost(m_transceiver->m_slaveHost, m_transceiver->m_port);
    return m_transceiver->m_udpSocket->write(data);
//    return m_transceiver->m_udpSocket->writeDatagram(datagram);
}

// SLAVE INIT
NetworkTransceiver::StateInitSlave::StateInitSlave(NetworkTransceiver *transceiver): AbstractState(transceiver) {
    m_transceiver->emit stateChanged(State::InitSlave);
}

NetworkTransceiver::StateInitSlave::~StateInitSlave() {

}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateInitSlave::start() {
    if(m_transceiver->m_selectedInterface.isNull()) {
        m_transceiver->emit error(tr("Error, no interface selected!"));
        return nullptr;
    }

    m_transceiver->m_udpSocket->close();
    if(!m_transceiver->m_udpSocket->bind(m_transceiver->m_selectedInterface, m_transceiver->m_port)) {
        m_transceiver->emit error(tr("Error binding socket to host: ") + m_transceiver->m_selectedInterface.toString() + tr(", port: ") + QString::number(m_transceiver->m_port));
        return nullptr;
    }

    return new StateBroadcast(m_transceiver);
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateInitSlave::stop() {
    m_transceiver->closeCalled();
    return nullptr;
}

qint64 NetworkTransceiver::StateInitSlave::sendData(const QByteArray &data, const bool &acknowledge) {
    return -1;
}

// SLAVE BROADCAST
NetworkTransceiver::StateBroadcast::StateBroadcast(NetworkTransceiver *transceiver): AbstractState(transceiver), m_pollPeriodMS(200) {
    m_transceiver->emit stateChanged(State::Broadcast);

    QNetworkDatagram datagram;
    datagram.setDestination(QHostAddress::Broadcast, m_transceiver->m_port);
    connect(&m_timer, &QTimer::timeout, [this, datagram] () {
        m_transceiver->m_udpSocket->writeDatagram(datagram);
    });

    m_timer.setInterval(m_pollPeriodMS);
    m_timer.start();
}

NetworkTransceiver::StateBroadcast::~StateBroadcast() {

}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateBroadcast::start() {
    return nullptr;
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateBroadcast::stop() {
    return new StateInitSlave(m_transceiver);
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateBroadcast::onReadyRead() {
    QNetworkDatagram datagram = m_transceiver->m_udpSocket->receiveDatagram();
    m_transceiver->m_masterHost = datagram.senderAddress();
    return new StateReceiveInput(m_transceiver);
}

qint64 NetworkTransceiver::StateBroadcast::sendData(const QByteArray &data, const bool &acknowledge) {
    return -1;
}

// SLAVE RECEIVE INPUT
NetworkTransceiver::StateReceiveInput::StateReceiveInput(NetworkTransceiver *transceiver): AbstractState(transceiver), m_timeoutms(1000) {
    m_transceiver->emit stateChanged(State::ReceiveInput);
    m_transceiver->emit connected();

    connect(&m_timer, &QTimer::timeout, m_transceiver, &NetworkTransceiver::onStop);
    m_timer.start(m_timeoutms);
}

NetworkTransceiver::StateReceiveInput::~StateReceiveInput() {
    m_transceiver->emit disconnected("");
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateReceiveInput::start() {
    return nullptr;
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateReceiveInput::stop() {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << QString("quit");

    QNetworkDatagram datagram;
    datagram.setDestination(m_transceiver->m_masterHost, m_transceiver->m_port);
    datagram.setData(data);
    m_transceiver->m_udpSocket->writeDatagram(datagram);

    return new StateBroadcast(m_transceiver);
}

NetworkTransceiver::AbstractState *NetworkTransceiver::StateReceiveInput::onReadyRead() {
    m_timer.start(m_timeoutms);
    QNetworkDatagram datagram = m_transceiver->m_udpSocket->receiveDatagram();
    m_transceiver->dataArrived(datagram.data());
}

qint64 NetworkTransceiver::StateReceiveInput::sendData(const QByteArray &data, const bool &acknowledge) {
    QNetworkDatagram datagram;
    datagram.setDestination(m_transceiver->m_masterHost, m_transceiver->m_port);
    datagram.setData(data);
    return m_transceiver->m_udpSocket->writeDatagram(datagram);
}

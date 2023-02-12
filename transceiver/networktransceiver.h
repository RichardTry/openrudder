#ifndef NETWORKTRANSCEIVER_H
#define NETWORKTRANSCEIVER_H

#include "transceiver/abstracttransceiver.h"
//#include <QUdpSocket>
//#include <QTimer>
//#include <QListWidgetItem>

class NetworkTransceiver : public AbstractTransceiver {

    class AbstractState;
    // MASTER STATES
    class StateInitMaster;
    class StateListen;
    class StateSendInput;
    // SLAVE STATES
    class StateInitSlave;
    class StateBroadcast;
    class StateReceiveInput;

public:
    enum State {
        // Master States
        InitMaster,
        Listen,
        SendInput,
        // Slave States
        InitSlave,
        Broadcast,
        ReceiveInput,
    };

    NetworkTransceiver(const Mode &mode, QObject *parent = nullptr);
    ~NetworkTransceiver();

    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;

    void setSelectedInterface(const QHostAddress &selectedInterface);

    void setSlaveHost(const QHostAddress &slaveHost);

signals:
    void stateChanged(State state);
    void hostFound(QString address);

public slots:
    // Not start & stop in the strict sense, start could mean connect to target, stop could be quit etc.
    // They represent transitions between states in opposing directions
    void onStart() override;
    void onStop() override;

private slots:
    void onReadyRead();

private:

    AbstractState *m_state;
    QUdpSocket *m_udpSocket;
    quint16 m_port;
    // Configured by master only, slave receives at the rate the master dictates
    int m_pollPeriodMS;
    // Common to both modes
    QHostAddress m_selectedInterface;
    // Paired devices, slave stores master and master vice versa
    QHostAddress m_slaveHost;
    QHostAddress m_masterHost;
    static unsigned int m_datagramId;
};

class NetworkTransceiver::AbstractState {
public:
    AbstractState(NetworkTransceiver *transceiver): m_transceiver(transceiver) {

    }

    virtual ~AbstractState() {}

    virtual AbstractState *start() = 0;
    virtual AbstractState *stop() = 0;
    virtual AbstractState *onReadyRead() { return nullptr; }
    virtual qint64 sendData(const QByteArray &data, const bool &acknowledge = false) {}

protected:
    NetworkTransceiver *m_transceiver;
};

// MASTER STATES
class NetworkTransceiver::StateInitMaster : public NetworkTransceiver::AbstractState {
public:
    StateInitMaster(NetworkTransceiver *transceiver);
    ~StateInitMaster();

    AbstractState *start() override;
    AbstractState *stop() override;
    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;
};

class NetworkTransceiver::StateListen: public NetworkTransceiver::AbstractState {
public:
    StateListen(NetworkTransceiver *transceiver);
    ~StateListen();

    AbstractState *start() override;
    AbstractState *stop() override;
    AbstractState *onReadyRead() override; // When a host is found add it to host list
    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;

private:
    QMap <int, QHostAddress> m_hosts;
};

class NetworkTransceiver::StateSendInput: public NetworkTransceiver::AbstractState {
public:
    StateSendInput(NetworkTransceiver *transceiver);
    ~StateSendInput();

    AbstractState *start() override;
    AbstractState *stop() override;
    AbstractState *onReadyRead() override; // When the slave informs it has quit receiving input go back to previous state and display an info message
    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;
};

// SLAVE STATES
class NetworkTransceiver::StateInitSlave : public NetworkTransceiver::AbstractState {
public:
    StateInitSlave(NetworkTransceiver *transceiver);
    ~StateInitSlave();

    AbstractState *start() override;
    AbstractState *stop() override;
    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;
};

class NetworkTransceiver::StateBroadcast: public NetworkTransceiver::AbstractState {
public:
    StateBroadcast(NetworkTransceiver *transceiver);
    ~StateBroadcast();

    AbstractState *start() override;
    AbstractState *stop() override;
    AbstractState *onReadyRead() override; // When a master sends a datagram go into receive input state and store master address
    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;

private:
    int m_pollPeriodMS;
    QTimer m_timer;
};

class NetworkTransceiver::StateReceiveInput: public NetworkTransceiver::AbstractState {
public:
    StateReceiveInput(NetworkTransceiver *transceiver);
    ~StateReceiveInput();

    AbstractState *start() override;
    AbstractState *stop() override;
    AbstractState *onReadyRead() override; // Receive data and emit data arrived signal
    qint64 sendData(const QByteArray &data, const bool &acknowledge = false) override;

private:
    QTimer m_timer;
    int m_timeoutms;
};

#endif // NETWORKTRANSCEIVER_H

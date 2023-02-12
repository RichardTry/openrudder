#ifndef ABSTRACTTRANSCEIVER_H
#define ABSTRACTTRANSCEIVER_H

#include <vector>
//#include <QObject>

class AbstractTransceiver {
public:
    enum Mode{
        Master,
        Slave
    };

    AbstractTransceiver(const Mode &mode): m_mode(mode) {

    }

    virtual ~AbstractTransceiver() {}

    Mode mode() const {
        return m_mode;
    }

//signals:
    void error(std::string error);
    void dataArrived(std::vector<uint8_t> data);
    void connected();
    void disconnected(std::string msg);
    void closeCalled();

public:
    virtual qint64 sendData(const std::vector<uint8_t> &data, const bool &acknowledge = false) = 0;
    virtual void onStart() = 0;
    virtual void onStop() = 0;

protected:
    Mode m_mode;
};

#endif // ABSTRACTTRANSCEIVER_H

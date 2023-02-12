#ifndef ABSTRACTDRIVER_H
#define ABSTRACTDRIVER_H

#include "sigslot/signal.h"

class AbstractDriver {
public:
    explicit AbstractDriver();

//slots
public:
    virtual void onDataArrived(const std::vector<uint8_t> &data) = 0;
    virtual void onConnected() = 0;
    virtual void onDisconnect() = 0;
};

#endif // ABSTRACTDRIVER_H

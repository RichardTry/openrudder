#ifndef LINUXGAMEPADDRIVER_H
#define LINUXGAMEPADDRIVER_H

#include "driver/abstractdriver.h"
#include "common/common.h"
//#include <QTimer>
// Required headers to use uinput and linux input
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define Point int
#define Timer int

class LinuxGamepadDriver : public AbstractDriver {
public:
    LinuxGamepadDriver();
    ~LinuxGamepadDriver();

//slots
public:
    void onDataArrived(const std::vector<uint8_t> &data);
    void onConnected();
    void onDisconnect();

private:
    void init();
    void writeSyncReport();
    void moveStick(const Button &btn, const Point &value);
    void pressButton(const Button &btn);
    void releaseButton(const Button &btn);
    Timer m_syncReportTimer;
    int m_syncPeriodms;
    int m_fileDescriptor;
    struct input_event m_ev;
};

#endif // LINUXGAMEPADDRIVER_H

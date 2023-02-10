#include "linuxgamepaddriver.h"
//#include "event/gamepadevent.h"

#define STICK_MAX_VAL 1024
#define STICK_FLAT_VAL 0
#define STICK_FUZZ_VAL 0

inline __u16 mapButton2Input(const Button &btn) {
    switch (btn) {
    case Button::X: return BTN_X;
    case Button::Y: return BTN_Y;
    case Button::A: return BTN_A;
    case Button::B: return BTN_B;
    case Button::UP: return BTN_DPAD_UP;
    case Button::DOWN: return BTN_DPAD_DOWN;
    case Button::LEFT: return BTN_DPAD_LEFT;
    case Button::RIGHT: return BTN_DPAD_RIGHT;
    case Button::BACK: return BTN_SELECT;
    case Button::START: return BTN_START;
    case Button::LEFTTRIGGER: return BTN_TL;
    case Button::RIGHTTRIGGER: return BTN_TR;
    case Button::LEFTBUMPER: return BTN_TL2;
    case Button::RIGHTBUMPER: return BTN_TR2;
    }
}

LinuxGamepadDriver::LinuxGamepadDriver(): AbstractDriver(), m_syncPeriodms(1) {
    m_syncReportTimer.setInterval(m_syncPeriodms);
    m_syncReportTimer.setSingleShot(true);
    connect(&m_syncReportTimer, &QTimer::timeout, this, &LinuxGamepadDriver::writeSyncReport);
    init();
}

LinuxGamepadDriver::~LinuxGamepadDriver() {
    if(ioctl(m_fileDescriptor, UI_DEV_DESTROY) < 0) {
        printf("error: ioctl");
    }
    close(m_fileDescriptor);
}

void LinuxGamepadDriver::onDataArrived(const std::vector<uint8_t> &data) {
    GamepadEvent event(data);

    const std::string btnLabel = labelForButton(event.m_button);
    switch (event.m_type) {
        case GamepadEvent::ButtonPressEvent: {
            pressButton(event.m_button);
            break;
        }
        case GamepadEvent::ButtonReleaseEvent: {
            releaseButton(event.m_button);
            break;
        }
        case GamepadEvent::StickMoveEvent: {
            moveStick(event.m_button, event.m_value);
            break;
        }
        case GamepadEvent::StickPressEvent: {
            moveStick(event.m_button, event.m_value);
            break;
        }
        case GamepadEvent::StickReleaseEvent: {
            moveStick(event.m_button, QPointF(0, 0));
            break;
        }
        case GamepadEvent::DummyEvent: {
            // Do nothing, sent to keep connection alive
            break;
        }
    }
}

void LinuxGamepadDriver::onConnected() {
    m_syncReportTimer.start(m_syncPeriodms);
}

void LinuxGamepadDriver::onDisconnect() {
    m_syncReportTimer.stop();
}

void LinuxGamepadDriver::init() {
    m_fileDescriptor = open("/dev/uinput", O_WRONLY | O_NONBLOCK); //opening of uinput
    if (m_fileDescriptor < 0) {
        printf("Opening of uinput failed!\n");
    }
    ioctl(m_fileDescriptor, UI_SET_EVBIT, EV_KEY); //setting Gamepad keys
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_A);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_B);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_X);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_Y);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_TL);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_TR);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_TL2);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_TR2);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_START);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_THUMBL);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_THUMBR);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_DPAD_UP);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_DPAD_DOWN);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_DPAD_LEFT);
    ioctl(m_fileDescriptor, UI_SET_KEYBIT, BTN_DPAD_RIGHT);
    ioctl(m_fileDescriptor, UI_SET_EVBIT, EV_ABS); //setting Gamepad thumbsticks
    ioctl(m_fileDescriptor, UI_SET_ABSBIT, ABS_X);
    ioctl(m_fileDescriptor, UI_SET_ABSBIT, ABS_Y);
    ioctl(m_fileDescriptor, UI_SET_ABSBIT, ABS_RX);
    ioctl(m_fileDescriptor, UI_SET_ABSBIT, ABS_RY);
    struct uinput_user_dev uidev; //setting the default settings of Gamepad
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Gamepad Emulator"); //Name of Gamepad
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x3;
    uidev.id.product = 0x3;
    uidev.id.version = 2;
    uidev.absmax[ABS_X] = STICK_MAX_VAL; //Parameters of thumbsticks
    uidev.absmin[ABS_X] = -STICK_MAX_VAL;
    uidev.absfuzz[ABS_X] = STICK_FUZZ_VAL;
    uidev.absflat[ABS_X] = STICK_FLAT_VAL;
    uidev.absmax[ABS_Y] = STICK_MAX_VAL;
    uidev.absmin[ABS_Y] = -STICK_MAX_VAL;
    uidev.absfuzz[ABS_Y] = STICK_FUZZ_VAL;
    uidev.absflat[ABS_Y] = STICK_FLAT_VAL;
    uidev.absmax[ABS_RX] = STICK_MAX_VAL;
    uidev.absmin[ABS_RX] = -STICK_MAX_VAL;
    uidev.absfuzz[ABS_RX] = STICK_FUZZ_VAL;
    uidev.absflat[ABS_RX] = STICK_FLAT_VAL;
    uidev.absmax[ABS_RY] = STICK_MAX_VAL;
    uidev.absmin[ABS_RY] = -STICK_MAX_VAL;
    uidev.absfuzz[ABS_RY] = STICK_FUZZ_VAL;
    uidev.absflat[ABS_RY] = STICK_FLAT_VAL;
    if(write(m_fileDescriptor, &uidev, sizeof(uidev)) < 0) //writing settings
    {
        printf("error: write");
    }
    if(ioctl(m_fileDescriptor, UI_DEV_CREATE) < 0) //writing ui dev create
    {
        printf("error: ui_dev_create");
    }
}

void LinuxGamepadDriver::writeSyncReport() {
    memset(&m_ev, 0, sizeof(struct input_event));
    m_ev.type = EV_SYN;
    m_ev.code = SYN_REPORT;
    m_ev.value = 0;
    if(write(m_fileDescriptor, &m_ev, sizeof(struct input_event)) < 0) //writing the sync report
    {
        printf("error: sync-report");
    }
}

void LinuxGamepadDriver::moveStick(const Button &btn, const QPointF &value) {
    // X axis
    memset(&m_ev, 0, sizeof(struct input_event)); //setting the memory for event
    m_ev.type = EV_ABS;
    m_ev.code = btn == Button::LEFTSTICK ? ABS_X : ABS_RX;
    m_ev.value = value.x() * STICK_MAX_VAL;
    if(write(m_fileDescriptor, &m_ev, sizeof(struct input_event)) < 0) //writing the thumbstick change
    {
        printf("error: thumbstick-write");
    }
    // Y axis
    memset(&m_ev, 0, sizeof(struct input_event)); //setting the memory for event
    m_ev.type = EV_ABS;
    m_ev.code = btn == Button::LEFTSTICK ? ABS_Y : ABS_RY;
    m_ev.value = value.y() * STICK_MAX_VAL;
    if(write(m_fileDescriptor, &m_ev, sizeof(struct input_event)) < 0) //writing the thumbstick change
    {
        printf("error: thumbstick-write");
    }

    if(!m_syncReportTimer.isActive())
        m_syncReportTimer.start();
}
void LinuxGamepadDriver::pressButton(const Button &btn) {
    memset(&m_ev, 0, sizeof(struct input_event)); //setting the memory for event
    m_ev.type = EV_KEY;
    m_ev.code = mapButton2Input(btn);
    m_ev.value = 1;
    if(write(m_fileDescriptor, &m_ev, sizeof(struct input_event)) < 0) //writing the thumbstick change
    {
        printf("error: key-write");
    }

    if(!m_syncReportTimer.isActive())
        m_syncReportTimer.start();
}
void LinuxGamepadDriver::releaseButton(const Button &btn) {
    memset(&m_ev, 0, sizeof(struct input_event)); //setting the memory for event
    m_ev.type = EV_KEY;
    m_ev.code = mapButton2Input(btn);
    m_ev.value = 0;
    if(write(m_fileDescriptor, &m_ev, sizeof(struct input_event)) < 0) //writing the thumbstick change
    {
        printf("error: key-write");
    }

    if(!m_syncReportTimer.isActive())
        m_syncReportTimer.start();
}

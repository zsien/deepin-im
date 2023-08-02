#ifndef INPUTMETHODV2_H
#define INPUTMETHODV2_H

#include "common.h"

#include <QObject>

struct wl_client;
struct wl_display;
struct wl_resource;

class Core;
class InputMethodV2Private;
class InputMethodKeyboardGrabV2;

class InputMethodV2 : public QObject
{
    Q_OBJECT

    friend class InputMethodV2Private;

public:
    InputMethodV2(Core *core, struct ::wl_resource *seat, QObject *parent);
    ~InputMethodV2();

    INIT_FUNCS_DEF
    
    void sendDeactivate();
    void sendActivate();

private:
    std::unique_ptr<InputMethodV2Private> d;
    Core *core_;
    struct ::wl_resource *seat_;

    InputMethodKeyboardGrabV2 *grab_;
};

#endif // !INPUTMETHODV2_H

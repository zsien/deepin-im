// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef X11KEYBOARDGRABBER_H
#define X11KEYBOARDGRABBER_H

#include "Xcb.h"

#include <wayland-server-core.h>

struct GrabberKeyEvent
{
    uint32_t keycode;
    bool isRelease;
};

class X11KeyboardGrabber : public Xcb
{
public:
    X11KeyboardGrabber(wl_event_loop *loop);
    virtual ~X11KeyboardGrabber();

protected:
    void xcbEvent(const std::unique_ptr<xcb_generic_event_t> &event) override;

public:
    struct
    {
        struct wl_signal key;
    } events;

private:
    int xcbFd_;
    uint8_t xinput2OPCode_;

    xcb_window_t win_;

    void initXinputExtension();
};

#endif // !X11KEYBOARDGRABBER_H

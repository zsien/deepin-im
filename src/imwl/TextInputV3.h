// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINPUTV3_H
#define TEXTINPUTV3_H

#include "common.h"

#include <memory>

struct wl_client;
struct wl_display;
struct wl_resource;

class Core;
class TextInputV3Private;

class TextInputV3
{
    friend class TextInputV3Private;

public:
    TextInputV3(Core *core, struct ::wl_resource *seat);
    ~TextInputV3();

    INIT_FUNCS_DEF

    void sendPreeditString(const char *text, int32_t cursor_begin, int32_t cursor_end);
    void sendCommitString(const char *text);
    void sendDone(uint32_t serial);

private:
    std::unique_ptr<TextInputV3Private> d;
    Core *core_;
    struct ::wl_resource *seat_;
};

#endif // !TEXTINPUTV3_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TextInputV3.h"

#include "Core.h"
#include "InputMethodManagerV2.h"
#include "InputMethodV2.h"
#include "wl/server/Resource.h"

TextInputV3::TextInputV3(Core *core, wl::server::Seat *seat)
    : core_(core)
    , seat_(seat)
{
}

TextInputV3::~TextInputV3() { }

void TextInputV3::sendPreeditString(const char *text, int32_t cursor_begin, int32_t cursor_end)
{
    const auto resources = resourceMap();
    for (auto &[client, resource] : resources) {
        if (m_enabled.find(resource.get()) == m_enabled.end()) {
            continue;
        }
        // d->send_preedit_string(resource->handle, text, cursor_begin, cursor_end);
    }
}

void TextInputV3::sendCommitString(const char *text)
{
    const auto resources = resourceMap();
    for (auto &[client, resource] : resources) {
        if (m_enabled.find(resource.get()) == m_enabled.end()) {
            continue;
        }
        // d->send_commit_string(text);
    }
}

void TextInputV3::sendDone(uint32_t serial)
{
    const auto resources = resourceMap();
    for (auto &[client, resource] : resources) {
        if (m_enabled.find(resource.get()) == m_enabled.end()) {
            continue;
        }
        // d->send_done(resource->handle, serial);
    }
}

void TextInputV3::zwp_text_input_v3_destroy(wl::server::Resource *resource)
{
    m_enabled.erase(resource);

    resource->destroy();
}

void TextInputV3::zwp_text_input_v3_enable(wl::server::Resource *resource)
{
    m_enabled.emplace(resource);

    auto im = core_->getInputMethodManagerV2()->getInputMethodV2BySeat(seat_);
    im->sendActivate();
}

void TextInputV3::zwp_text_input_v3_disable(wl::server::Resource *resource)
{
    m_enabled.erase(resource);

    auto im = core_->getInputMethodManagerV2()->getInputMethodV2BySeat(seat_);
    im->sendDeactivate();
}

void TextInputV3::zwp_text_input_v3_set_surrounding_text(wl::server::Resource *resource,
                                                         const char *text,
                                                         int32_t cursor,
                                                         int32_t anchor)
{
}

void TextInputV3::zwp_text_input_v3_set_text_change_cause(wl::server::Resource *resource,
                                                          uint32_t cause)
{
}

void TextInputV3::zwp_text_input_v3_set_content_type(wl::server::Resource *resource,
                                                     uint32_t hint,
                                                     uint32_t purpose)
{
}

void TextInputV3::zwp_text_input_v3_set_cursor_rectangle(
    wl::server::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
}

void TextInputV3::zwp_text_input_v3_commit(wl::server::Resource *resource) { }

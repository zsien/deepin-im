// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TextInputV3.h"

#include "Core.h"
#include "InputMethodManagerV2.h"
#include "InputMethodV2.h"
#include "wl/server/Resource.h"
#include "wl/server/ZwpTextInputV3.h"

#include <set>

class TextInputV3Private : public wl::server::ZwpTextInputV3
{
    friend class TextInputV3;

public:
    TextInputV3Private(TextInputV3 *q)
        : q(q)
    {
    }

    ~TextInputV3Private() { }

protected:
    void zwp_text_input_v3_destroy(wl::server::Resource *resource) override
    {
        m_enabled.erase(resource);

        resource->destroy();
    }

    void zwp_text_input_v3_enable(wl::server::Resource *resource) override
    {
        m_enabled.emplace(resource);

        auto im = q->core_->getInputMethodManagerV2()->getInputMethodV2BySeat(q->seat_);
        im->sendActivate();
    }

    void zwp_text_input_v3_disable(wl::server::Resource *resource) override
    {
        m_enabled.erase(resource);

        auto im = q->core_->getInputMethodManagerV2()->getInputMethodV2BySeat(q->seat_);
        im->sendDeactivate();
    }

    void zwp_text_input_v3_set_surrounding_text(wl::server::Resource *resource,
                                                const char *text,
                                                int32_t cursor,
                                                int32_t anchor) override
    {
    }

    void zwp_text_input_v3_set_text_change_cause(wl::server::Resource *resource,
                                                 uint32_t cause) override
    {
    }

    void zwp_text_input_v3_set_content_type(wl::server::Resource *resource,
                                            uint32_t hint,
                                            uint32_t purpose) override
    {
    }

    void zwp_text_input_v3_set_cursor_rectangle(wl::server::Resource *resource,
                                                int32_t x,
                                                int32_t y,
                                                int32_t width,
                                                int32_t height) override
    {
    }

    void zwp_text_input_v3_commit(wl::server::Resource *resource) override { }

private:
    TextInputV3 *q;
    std::set<wl::server::Resource *> m_enabled;
};

TextInputV3::TextInputV3(Core *core, struct ::wl_resource *seat)
    : core_(core)
    , d(std::make_unique<TextInputV3Private>(this))
{
}

TextInputV3::~TextInputV3() { }

INIT_FUNCS(TextInputV3)

void TextInputV3::sendPreeditString(const char *text, int32_t cursor_begin, int32_t cursor_end)
{
    const auto resources = d->resourceMap();
    for (auto &[client, resource] : resources) {
        if (d->m_enabled.find(resource.get()) == d->m_enabled.end()) {
            continue;
        }
        // d->send_preedit_string(resource->handle, text, cursor_begin, cursor_end);
    }
}

void TextInputV3::sendCommitString(const char *text)
{
    const auto resources = d->resourceMap();
    for (auto &[client, resource] : resources) {
        if (d->m_enabled.find(resource.get()) == d->m_enabled.end()) {
            continue;
        }
        // d->send_commit_string(text);
    }
}

void TextInputV3::sendDone(uint32_t serial)
{
    const auto resources = d->resourceMap();
    for (auto &[client, resource] : resources) {
        if (d->m_enabled.find(resource.get()) == d->m_enabled.end()) {
            continue;
        }
        // d->send_done(resource->handle, serial);
    }
}

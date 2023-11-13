// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodV2.h"

#include "Server.h"
#include "TextInputV3.h"

InputMethodV2::InputMethodV2(Server *server, wlr_input_method_v2 *input_method)
    : server_(server)
    , input_method_(input_method)
    , commit_(this)
    , new_popup_surface_(this)
    , grab_keyboard_(this)
    , destroy_(this)
    , keyboard_grab_destroy_(this)
{
    wl_signal_add(&input_method_->events.commit, commit_);
    wl_signal_add(&input_method_->events.new_popup_surface, new_popup_surface_);
    wl_signal_add(&input_method_->events.grab_keyboard, grab_keyboard_);
    wl_signal_add(&input_method_->events.destroy, destroy_);
}

InputMethodV2::~InputMethodV2() { }

void InputMethodV2::commitNotify(void *data)
{
    auto *textInput = getFocusedTextInput();
    if (!textInput) {
        return;
    }

    if (input_method_->current.preedit.text) {
        wlr_text_input_v3_send_preedit_string(textInput->text_input_,
                                              input_method_->current.preedit.text,
                                              input_method_->current.preedit.cursor_begin,
                                              input_method_->current.preedit.cursor_end);
    }
    if (input_method_->current.commit_text) {
        wlr_text_input_v3_send_commit_string(textInput->text_input_,
                                             input_method_->current.commit_text);
    }
    if (input_method_->current.delete_.before_length
        || input_method_->current.delete_.after_length) {
        wlr_text_input_v3_send_delete_surrounding_text(textInput->text_input_,
                                                       input_method_->current.delete_.before_length,
                                                       input_method_->current.delete_.after_length);
    }

    wlr_text_input_v3_send_done(textInput->text_input_);
}

void InputMethodV2::newPopupSurfaceNotify(void *data) { }

void InputMethodV2::grabKeyboardNotify(void *data)
{
    auto *keyboard_grab = static_cast<wlr_input_method_keyboard_grab_v2 *>(data);

    struct wlr_keyboard *active_keyboard = wlr_seat_get_keyboard(input_method_->seat);

    // send modifier state to grab
    wlr_input_method_keyboard_grab_v2_set_keyboard(keyboard_grab, active_keyboard);

    server_->startGrab();
    wl_signal_add(&keyboard_grab->events.destroy, keyboard_grab_destroy_);
}

void InputMethodV2::destroyNotify(void *data)
{
    delete this;
}

void InputMethodV2::keyboardGrabDestroyNotify(void *data)
{
    server_->stopGrab();

    auto *keyboard_grab = static_cast<wlr_input_method_keyboard_grab_v2 *>(data);
    // wl_list_remove(&input_method_keyboard_grab_destroy_.link);

    if (keyboard_grab->keyboard) {
        // send modifier state to original client
        wlr_seat_keyboard_notify_modifiers(keyboard_grab->input_method->seat,
                                           &keyboard_grab->keyboard->modifiers);
    }
}

TextInputV3 *InputMethodV2::getFocusedTextInput() const
{
    wl_list *textInputs = server_->textInputs();
    TextInputV3 *textInput = nullptr;
    wl_list_for_each(textInput, textInputs, link_)
    {
        if (textInput->text_input_->focused_surface) {
            return textInput;
        }
    }

    return nullptr;
}

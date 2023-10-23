// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Dim.h"

#include "Addon.h"
#include "FrontendAddon.h"
#include "InputContext.h"
#include "InputMethodAddon.h"
#include "ProxyAddon.h"
#include "common/common.h"
#include "config.h"

#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QSettings>
#include <QTimer>

#include <dlfcn.h>

#define DIM_INPUT_METHOD_SWITCH_KEYBINDING_CODE (SHIFT_MASK + CONTROL_MASK)

using namespace org::deepin::dim;

static const QMap<QString, AddonType> AddonsType = {
    { "Frontend", AddonType::Frontend },
    { "InputMethod", AddonType::InputMethod },
};

Dim::Dim(QObject *parent)
    : QObject(parent)
    , focusedInputContext_(0)
{
    loadAddons();
}

Dim::~Dim() { }

void Dim::loadAddons()
{
    QDir dir(DIM_ADDON_INFO_DIR);
    qInfo() << "addon info dir" << dir.absolutePath();
    auto addonInfoFiles = dir.entryInfoList(QDir::Filter::Files | QDir::Filter::Readable);
    for (const auto &addonInfoFile : addonInfoFiles) {
        QString filename = addonInfoFile.fileName();
        if (filename.startsWith('.') || !filename.endsWith(".conf")) {
            continue;
        }

        loadAddon(addonInfoFile.absoluteFilePath());
    }
}

void Dim::loadAddon(const QString &infoFile)
{
    QSettings settings(infoFile, QSettings::Format::IniFormat);
    settings.beginGroup("Addon");
    if (!settings.contains("Name") || !settings.contains("Category")
        || !settings.contains("Library")) {
        qWarning() << "Addon info file" << infoFile << "is invalid";
        return;
    }

    QString name = settings.value("Name").toString();
    QString category = settings.value("Category").toString();
    QString library = settings.value("Library").toString();
    settings.endGroup();

    QDir addonDir(DIM_ADDON_DIR);
    QString libraryFile = library + ".so";
    if (!addonDir.exists(libraryFile)) {
        qWarning() << "Addon library" << libraryFile << "not found";
        return;
    }

    QString libraryPath = addonDir.filePath(libraryFile);
    void *handle = dlopen(libraryPath.toStdString().c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        qWarning() << "Addon library" << libraryFile << "failed to load:" << dlerror();
        return;
    }

    void *createFn = dlsym(handle, "create");
    if (createFn == nullptr) {
        qWarning() << "Addon library" << libraryFile << "does not have a create() function";
        return;
    }

    auto create = reinterpret_cast<addonCreate *>(createFn);
    Addon *addon = create(this);

    switch (AddonsType[category]) {
    case AddonType::Frontend: {
        auto *frontend = qobject_cast<FrontendAddon *>(addon);
        frontends_.insert(frontend);
        break;
    }
    case AddonType::InputMethod: {
        auto *imAddon = qobject_cast<InputMethodAddon *>(addon);
        connect(imAddon, &InputMethodAddon::addonInitFinished, this, &Dim::initInputMethodAddon);
        imAddon->initInputMethods();
        inputMethodAddons_.emplace(imAddon->key(), imAddon);
        break;
    }
    default:
        qWarning() << "Addon" << name << "has an invalid category" << category;
        delete addon;
    }
}

void Dim::initInputMethodAddon(InputMethodAddon *imAddon)
{
    for (const auto &entry : imAddon->getInputMethods()) {
        imEntries_.emplace_back(entry);
    }

    if (!imEntries_.empty()) {
        QTimer::singleShot(0, [this]() {
            Q_EMIT inputMethodEntryChanged();
        });
    }
}

bool Dim::postEvent(Event &event)
{
    switch (event.type()) {
    case EventType::InputContextCreated:
        postInputContextCreated(event);
        break;
    case EventType::InputContextDestroyed:
        postInputContextCreated(event);
        break;
    case EventType::InputContextFocused:
        postInputContextFocused(event);
        break;
    case EventType::InputContextUnfocused:
        postInputContextUnfocused(event);
        break;
    case EventType::InputContextKeyEvent:
        return postInputContextKeyEvent(reinterpret_cast<InputContextKeyEvent &>(event));
        break;
    case EventType::InputContextCursorRectChanged:
        postInputContextCursorRectChanged(
            reinterpret_cast<InputContextCursorRectChangeEvent &>(event));
        break;
    case EventType::InputContextUpdateSurroundingText:
        postInputContextSetSurroundingTextEvent(event);
        break;
    }

    return false;
}

void Dim::postInputContextCreated(Event &event)
{
    auto *ic = event.ic();

    connect(ic, &InputContext::imSwitch, this, &Dim::switchIM);

    inputContexts_.emplace(ic->id(), ic);

    for (const auto &[_, v] : inputMethodAddons_) {
        auto addon = getImAddon<ProxyAddon *>(v);
        if (addon) {
            addon->createFcitxInputContext(ic);
        }
    }
}

void Dim::postInputContextDestroyed([[maybe_unused]] Event &event)
{
    inputContexts_.erase(event.ic()->id());
    for (const auto &[_, v] : inputMethodAddons_) {
        auto addon = getImAddon<ProxyAddon *>(v);
        if (addon) {
            addon->destroyed(event.ic()->id());
        }
    }
}

void Dim::postInputContextFocused(Event &event)
{
    focusedInputContext_ = event.ic()->id();
    emit focusedInputContextChanged(focusedInputContext_);

    for (const auto &[_, v] : inputMethodAddons_) {
        auto addon = getImAddon<ProxyAddon *>(v);

        if (addon) {
            addon->focusIn(focusedInputContext_);
        }
    }
}

void Dim::postInputContextUnfocused(Event &event)
{
    focusedInputContext_ = 0;
    emit focusedInputContextChanged(focusedInputContext_);

    for (const auto &[_, v] : inputMethodAddons_) {
        auto addon = getImAddon<ProxyAddon *>(v);
        if (addon) {
            addon->focusOut(event.ic()->id());
        }
    }
}

bool Dim::postInputContextKeyEvent(InputContextKeyEvent &event)
{
    auto &inputState = event.ic()->inputState();

    if (event.isRelease() && event.state() == DIM_INPUT_METHOD_SWITCH_KEYBINDING_CODE) {
        QTimer::singleShot(0, [&inputState]() {
            inputState.switchIMAddon();
        });
    }

    const auto &state = event.ic()->inputState();

    auto addon = getImAddon<InputMethodAddon *>(getInputMethodAddon(state));
    const auto &imList = imEntries();

    if (addon && !imList.empty()) {
        return addon->keyEvent(imList[state.currentIMIndex()], event);
    }

    return false;
}

void Dim::postInputContextCursorRectChanged(InputContextCursorRectChangeEvent &event)
{
    auto addon = getImAddon<ProxyAddon *>(getInputMethodAddon(event.ic()->inputState()));

    if (addon) {
        addon->cursorRectangleChangeEvent(event);
    }
}

void Dim::postInputContextSetSurroundingTextEvent(Event &event)
{
    auto addon = getImAddon<InputMethodAddon *>(getInputMethodAddon(event.ic()->inputState()));

    if (addon) {
        addon->updateSurroundingText(event);
    }
}

InputMethodAddon *Dim::getInputMethodAddon(const InputState &inputState)
{
    const QString &addonKey = imEntries()[inputState.currentIMIndex()].addonName();
    auto j = imAddons().find(addonKey);
    assert(j != imAddons().end());

    return j->second;
}

template<typename T>
T Dim::getImAddon(InputMethodAddon *imAddon) const
{
    T addon = qobject_cast<T>(imAddon);

    return addon ? addon : nullptr;
}

void Dim::switchIM(const std::pair<QString, QString> &imIndex)
{
    auto addon = getImAddon<ProxyAddon *>(imAddons().at(imIndex.first));

    if (addon) {
        addon->setCurrentIM(imIndex.second);
    }
}

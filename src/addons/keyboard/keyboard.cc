#include "keyboard.h"

#include "config.h"
#include "dimcore/Events.h"
#include "dimcore/InputContext.h"

#include <QDir>
#include <QDomDocument>
#include <QList>

using namespace org::deepin::dim;

DIM_ADDON_FACTORY(Keyboard)

struct Variant
{
    QString name;
    QString shortDescription;
    QString description;
    QList<QString> languageList;
};

struct Layout
{
    QString name;
    QString shortDescription;
    QString description;
    QList<QString> languageList;
    QList<Variant> variantList;
};

Keyboard::Keyboard(Dim *dim)
    : InputMethodAddon(dim, "keyboard")
{
    ctx_.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));

    if (!ctx_) {
        throw std::runtime_error("Failed to create xkb context");
    }

    keymap_.reset(nullptr);
    state_.reset(nullptr);

    QDir dir(QStringLiteral(XKEYBOARDCONFIG_XKBBASE) + QDir::separator() + "rules");
    QString rules = dir.absoluteFilePath(QString("%1.xml").arg(DEFAULT_XKB_RULES));
    QString extraRules = dir.absoluteFilePath(QString("%1.extras.xml").arg(DEFAULT_XKB_RULES));
}

Keyboard::~Keyboard() { }

QList<InputMethodEntry> Keyboard::getInputMethods()
{
    return keyboards_;
}

void Keyboard::keyEvent(const InputMethodEntry &entry, InputContextKeyEvent &keyEvent)
{
    Q_UNUSED(entry);
    // TODO: handle keymap
    auto inputContext = keyEvent.ic();

    inputContext->updatePreeditString(QString());
    inputContext->updateCommitString(QString());
}

// static QList<QString> parseLanguageList(const QDomElement &languageListEle) {
//     QList<QString> languageList;
//     for (auto language = languageListEle.firstChildElement("iso639Id"); !language.isNull();
//          language = language.nextSiblingElement("iso639Id")) {
//         auto languageName = language.text();
//         languageList.append(languageName);
//     }

//     return languageList;
// }

void Keyboard::parseLayoutList(const QDomElement &layoutListEle)
{
    for (auto layoutEle = layoutListEle.firstChildElement("layout"); !layoutEle.isNull();
         layoutEle = layoutEle.nextSiblingElement("layout")) {
        auto configItemEle = layoutEle.firstChildElement("configItem");

        QString name = configItemEle.firstChildElement("name").text();
        QString shortDescription = configItemEle.firstChildElement("shortDescription").text();
        QString description = configItemEle.firstChildElement("description").text();
        // QString languageList =
        // parseLanguageList(configItemEle.firstChildElement("languageList"));

        keyboards_.append(
            { key(), QString("keyboard-%1").arg(name), name, shortDescription, description, "" });

        parseVariantList(name, layoutEle.firstChildElement("variantList"));
    }
}

void Keyboard::parseVariantList(const QString &layoutName, const QDomElement &variantListEle)
{
    QList<Variant> list;
    for (auto variantEle = variantListEle.firstChildElement("variant"); !variantEle.isNull();
         variantEle = variantEle.nextSiblingElement("variant")) {
        auto configItemEle = variantEle.firstChildElement("configItem");

        QString name = configItemEle.firstChildElement("name").text();
        QString shortDescription = configItemEle.firstChildElement("shortDescription").text();
        QString description = configItemEle.firstChildElement("description").text();
        // QString languageList =
        // parseLanguageList(configItemEle.firstChildElement("languageList"));

        QString fullname = layoutName + "_" + name;

        keyboards_.append({ key(),
                            QString("keyboard-%1").arg(fullname),
                            fullname,
                            shortDescription,
                            description,
                            "" });
    }
}

void Keyboard::parseRule(const QString &file)
{
    QFile xmlFile(file);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QDomDocument xmlReader;
    xmlReader.setContent(&xmlFile);
    auto layoutListEle = xmlReader.documentElement().firstChildElement("layoutList");

    parseLayoutList(layoutListEle);
}

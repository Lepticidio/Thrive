#include "ogre/text_overlay.h"

#include "engine/component_factory.h"
#include "engine/engine.h"
#include "engine/entity_filter.h"
#include "engine/serialization.h"
#include "scripting/luabind.h"

#include <iostream>
#include <OgreOverlayManager.h>

using namespace thrive;

////////////////////////////////////////////////////////////////////////////////
// TextOverlayComponent
////////////////////////////////////////////////////////////////////////////////


luabind::scope
TextOverlayComponent::luaBindings() {
    using namespace luabind;
    return class_<TextOverlayComponent, Component>("TextOverlayComponent")
        .enum_("HorizontalAlignment") [
            value("Left", Ogre::GHA_LEFT),
            value("Center", Ogre::GHA_CENTER),
            value("Right", Ogre::GHA_RIGHT)
        ]
        .enum_("VerticalAlignment") [
            value("Top", Ogre::GVA_TOP),
            value("Center", Ogre::GVA_CENTER),
            value("Bottom", Ogre::GVA_BOTTOM)
        ]
        .enum_("ID") [
            value("TYPE_ID", TextOverlayComponent::TYPE_ID)
        ]
        .scope [
            def("TYPE_NAME", &TextOverlayComponent::TYPE_NAME),
            class_<Properties, Touchable>("Properties")
                .def_readwrite("charHeight", &Properties::charHeight)
                .def_readwrite("colour", &Properties::colour)
                .def_readwrite("fontName", &Properties::fontName)
                .def_readwrite("height", &Properties::height)
                .def_readwrite("horizontalAlignment", &Properties::horizontalAlignment)
                .def_readwrite("left", &Properties::left)
                .def_readwrite("text", &Properties::text)
                .def_readwrite("top", &Properties::top)
                .def_readwrite("verticalAlignment", &Properties::verticalAlignment)
                .def_readwrite("width", &Properties::width)
        ]
        .def(constructor<std::string>())
        .def("name", &TextOverlayComponent::name)
        .def_readonly("properties", &TextOverlayComponent::m_properties)
    ;
}

TextOverlayComponent::TextOverlayComponent(
    Ogre::String name
) : m_name(name)
{
}


TextOverlayComponent::TextOverlayComponent() {}


void
TextOverlayComponent::load(
    const StorageContainer& storage
) {
    Component::load(storage);
    m_name = storage.get<Ogre::String>("name", "");
    m_properties.charHeight = storage.get<Ogre::Real>("charHeight", 16.0f);
    m_properties.colour = storage.get<Ogre::ColourValue>("colour", Ogre::ColourValue::White);
    m_properties.fontName = storage.get<Ogre::String>("fontName", "Thrive");
    m_properties.height = storage.get<Ogre::Real>("height", 100.0f);
    m_properties.horizontalAlignment = static_cast<Ogre::GuiHorizontalAlignment>(
        storage.get<uint8_t>("horizontalAlignment", Ogre::GHA_LEFT)
    );
    m_properties.left = storage.get<Ogre::Real>("left", 0.0f);
    m_properties.text = storage.get<Ogre::String>("text", "");
    m_properties.top = storage.get<Ogre::Real>("top", 0.0f);
    m_properties.verticalAlignment = static_cast<Ogre::GuiVerticalAlignment>(
        storage.get<uint8_t>("verticalAlignment", Ogre::GVA_TOP)
    );
    m_properties.width = storage.get<Ogre::Real>("width", 100.0f);
}


StorageContainer
TextOverlayComponent::storage() const {
    StorageContainer storage = Component::storage();
    storage.set<Ogre::String>("name", m_name);
    storage.set<Ogre::Real>("charHeight", m_properties.charHeight);
    storage.set<Ogre::ColourValue>("colour", m_properties.colour);
    storage.set<Ogre::String>("fontName", m_properties.fontName);
    storage.set<Ogre::Real>("height", m_properties.height);
    storage.set<uint8_t>("horizontalAlignment", m_properties.horizontalAlignment);
    storage.set<Ogre::Real>("left", m_properties.left);
    storage.set<Ogre::String>("text", m_properties.text);
    storage.set<Ogre::Real>("top", m_properties.top);
    storage.set<uint8_t>("verticalAlignment", m_properties.verticalAlignment);
    storage.set<Ogre::Real>("width", m_properties.width);
    return storage;
}


REGISTER_COMPONENT(TextOverlayComponent)


////////////////////////////////////////////////////////////////////////////////
// TextOverlaySystem
////////////////////////////////////////////////////////////////////////////////

struct TextOverlaySystem::Implementation {

    Implementation() {
        m_overlayManager = Ogre::OverlayManager::getSingletonPtr();
        m_overlay = m_overlayManager->create("text_overlay");
        m_panel = static_cast<Ogre::OverlayContainer*>(
            m_overlayManager->createOverlayElement("Panel", "text_panel")
        );
        m_panel->setDimensions(1.0, 1.0);
        m_panel->setPosition(0.0, 0.0);
        m_overlay->add2D(m_panel);
    }

    EntityFilter<
        TextOverlayComponent
    > m_entities = {true};

    Ogre::Overlay* m_overlay = nullptr;

    Ogre::OverlayManager* m_overlayManager = nullptr;

    Ogre::OverlayContainer* m_panel = nullptr;

    std::unordered_map<EntityId, Ogre::TextAreaOverlayElement*> m_textOverlays;
};


TextOverlaySystem::TextOverlaySystem()
  : m_impl(new Implementation())
{
}


TextOverlaySystem::~TextOverlaySystem() {}


void
TextOverlaySystem::init(
    Engine* engine
) {
    System::init(engine);
    m_impl->m_entities.setEntityManager(&engine->entityManager());
    m_impl->m_overlay->show();
}


void
TextOverlaySystem::shutdown() {
    m_impl->m_overlay->hide();
    m_impl->m_entities.setEntityManager(nullptr);
    System::shutdown();
}


void
TextOverlaySystem::update(int) {
    for (EntityId entityId : m_impl->m_entities.removedEntities()) {
        Ogre::OverlayElement* textOverlay = m_impl->m_textOverlays[entityId];
        m_impl->m_overlayManager->destroyOverlayElement(textOverlay);
        m_impl->m_textOverlays.erase(entityId);
    }
    for (auto& value : m_impl->m_entities.addedEntities()) {
        EntityId entityId = value.first;
        TextOverlayComponent* textOverlayComponent = std::get<0>(value.second);
        Ogre::TextAreaOverlayElement* textOverlay = static_cast<Ogre::TextAreaOverlayElement*>(
            m_impl->m_overlayManager->createOverlayElement(
                "TextArea",
                textOverlayComponent->name()
            )
        );
        textOverlayComponent->m_overlayElement = textOverlay;
        m_impl->m_textOverlays[entityId] = textOverlay;
        m_impl->m_panel->addChild(textOverlay);
        textOverlay->setMetricsMode(Ogre::GMM_PIXELS);
    }
    m_impl->m_entities.clearChanges();
    for (auto& value : m_impl->m_entities) {
        TextOverlayComponent* textOverlayComponent = std::get<0>(value.second);
        auto& properties = textOverlayComponent->m_properties;
        if (properties.hasChanges()) {
            Ogre::TextAreaOverlayElement* textOverlay = textOverlayComponent->m_overlayElement;
            textOverlay->setPosition(
                properties.left,
                properties.top
            );
            textOverlay->setDimensions(
                properties.width,
                properties.height
            );
            textOverlay->setCharHeight(properties.charHeight);
            textOverlay->setColour(properties.colour);
            textOverlay->setFontName(properties.fontName);
            textOverlay->setCaption(properties.text);
            textOverlay->setHorizontalAlignment(properties.horizontalAlignment);
            textOverlay->setVerticalAlignment(properties.verticalAlignment);
            // Untouch
            properties.untouch();
        }
    }
}



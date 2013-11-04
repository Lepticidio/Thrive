#include "bullet/collision_system.h"

#include "engine/component_factory.h"
#include "engine/engine.h"

#include "scripting/luabind.h"

#include <unordered_map>
#include "bullet/rigid_body_system.h"
#include "engine/serialization.h"
#include "engine/entity_manager.h"
#include "engine/entity.h"

using namespace thrive;

////////////////////////////////////////////////////////////////////////////////
// CollisionHandlerComponent
////////////////////////////////////////////////////////////////////////////////

luabind::scope
CollisionHandlerComponent::luaBindings() {
    using namespace luabind;
    return class_<CollisionHandlerComponent, Component>("CollisionHandlerComponent")
        .enum_("ID") [
            value("TYPE_ID", CollisionHandlerComponent::TYPE_ID)
        ]
        .scope [
            def("TYPE_NAME", &CollisionHandlerComponent::TYPE_NAME)
        ]
        .def(constructor<>())
        .def_readwrite("collisionCallbackKey", &CollisionHandlerComponent::m_collisionCallbackKey)
    ;
}


void
CollisionHandlerComponent::load(
    const StorageContainer& storage
) {
    Component::load(storage);/*
    // Static
    m_properties.shape = CollisionShape::load(storage.get<StorageContainer>("shape", StorageContainer()));
    m_properties.restitution = storage.get<btScalar>("restitution", 0.0f);
    m_properties.rollingFriction = storage.get<btScalar>("rollingFriction", 0.0f);
    m_properties.hasContactResponse = storage.get<bool>("hasContactResponse", true);
    m_properties.kinematic = storage.get<bool>("kinematic", false);
    m_properties.touch();
    // Dynamic
    m_dynamicProperties.position = storage.get<Ogre::Vector3>("position", Ogre::Vector3::ZERO);
    m_dynamicProperties.angularVelocity = storage.get<Ogre::Vector3>("angularVelocity", Ogre::Vector3::ZERO);*/
}


StorageContainer
CollisionHandlerComponent::storage() const {
    StorageContainer storage = Component::storage();
    /*// Static
    storage.set<StorageContainer>("shape", m_properties.shape->storage());
    storage.set<Ogre::Vector3>("linearFactor", m_properties.linearFactor);
    storage.set<bool>("hasContactResponse", m_properties.hasContactResponse);
    storage.set<bool>("kinematic", m_properties.kinematic);
    // Dynamic
    storage.set<Ogre::Vector3>("position", m_dynamicProperties.position);
    storage.set<Ogre::Vector3>("angularVelocity", m_dynamicProperties.angularVelocity);*/
    return storage;
}

REGISTER_COMPONENT(CollisionHandlerComponent)



////////////////////////////////////////////////////////////////////////////////
// CollisionSystem
////////////////////////////////////////////////////////////////////////////////



struct CollisionSystem::Implementation {

    btDiscreteDynamicsWorld* m_world = nullptr;

};


CollisionSystem::CollisionSystem()
  : m_impl(new Implementation())
{
    CollisionSystem::registerCollisionCallback("microbe_message", [](EntityId, EntityId){std::cout << "YAYA!!";});
}


CollisionSystem::~CollisionSystem() {}


void
CollisionSystem::init(
    Engine* engine
) {
    System::init(engine);
    m_impl->m_world = engine->physicsWorld();
}


void
CollisionSystem::shutdown() {
    System::shutdown();
}

using CollisionCallback = std::function<void (EntityId selfId, EntityId opponentId)>;

static std::unordered_map<std::string, CollisionCallback> callbackFunctions;

void
CollisionSystem::update(int) {

    auto dispatcher = m_impl->m_world->getDispatcher();
    int numManifolds = dispatcher->getNumManifolds();
    for (int i=0;i<numManifolds;i++)
    {

        btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
        auto objectA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
        auto objectB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
        EntityId entityId1 = (reinterpret_cast<uintptr_t>(objectA->getUserPointer()));
        EntityId entityId2 = (reinterpret_cast<uintptr_t>(objectB->getUserPointer()));
        Component* componentPtr = System::engine()->entityManager().getComponent(entityId1, CollisionHandlerComponent::TYPE_ID);
        if (componentPtr)
        {
            CollisionCallback callback = callbackFunctions[static_cast<CollisionHandlerComponent*>(
                                            componentPtr
                                        )->m_collisionCallbackKey];
            if (callback != nullptr)
            {
                callback(entityId1, entityId2);
            }

        }
        componentPtr = System::engine()->entityManager().getComponent(entityId2, CollisionHandlerComponent::TYPE_ID);
        if (componentPtr)
        {
            CollisionCallback callback = callbackFunctions[static_cast<CollisionHandlerComponent*>(
                                            componentPtr
                                        )->m_collisionCallbackKey];
            if (callback != nullptr)
            {
                callback(entityId2, entityId1);
            }

        }
        contactManifold->clearManifold();
    }
}


void
CollisionSystem::registerCollisionCallback(
    const std::string& key,
    CollisionCallback callback
) {
    callbackFunctions[key] = callback;
}

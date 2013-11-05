#include "bullet/collision_system.h"



#include "scripting/luabind.h"

#include "engine/component_factory.h"
#include "engine/engine.h"
#include "engine/entity.h"
#include "engine/entity_manager.h"
#include "engine/serialization.h"
#include "bullet/rigid_body_system.h"
#include <unordered_map>


#include "util/pair_hash.h"

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
CollisionHandlerComponent::addCollisionGroup(
    const std::string& group
) {
    m_collisionGroups.push_back(group);
}

const std::vector<std::string>&
CollisionHandlerComponent::getCollisionGroups() {
    return m_collisionGroups;
}

void
CollisionHandlerComponent::load(
    const StorageContainer& storage
) {
    Component::load(storage);
    m_collisionCallbackKey = storage.get<std::string>("collisionCallbackKey","");
}


StorageContainer
CollisionHandlerComponent::storage() const {
    StorageContainer storage = Component::storage();
    storage.set<std::string>("collisionCallbackKey", m_collisionCallbackKey);
    return storage;
}

REGISTER_COMPONENT(CollisionHandlerComponent)



////////////////////////////////////////////////////////////////////////////////
// CollisionSystem
////////////////////////////////////////////////////////////////////////////////



struct CollisionSystem::Implementation {

    btDiscreteDynamicsWorld* m_world = nullptr;

    std::unordered_multimap<std::pair<std::string, std::string>, CollisionFilter&>  m_collisionFilterMap;

};


CollisionSystem::CollisionSystem()
  : m_impl(new Implementation())
{
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
        CollisionHandlerComponent* collisionComponent1 = static_cast<CollisionHandlerComponent*>(
                                            System::engine()->entityManager().getComponent(entityId1, CollisionHandlerComponent::TYPE_ID)
                                        );
        CollisionHandlerComponent* collisionComponent2 = static_cast<CollisionHandlerComponent*>(
                                            System::engine()->entityManager().getComponent(entityId1, CollisionHandlerComponent::TYPE_ID)
                                        );

        if (collisionComponent1 && collisionComponent2)
        {
            std::vector<std::string> collisionGroups1 = collisionComponent1->getCollisionGroups();
            std::vector<std::string> collisionGroups2 = collisionComponent2->getCollisionGroups();
            std::vector<std::pair<std::string, std::string>> collisionGroupPermutations(collisionGroups1.size() * collisionGroups2.size());

            for(std::string collisionGroup1 : collisionGroups1)
            {
                for(std::string collisionGroup2 : collisionGroups1)
                {
                    collisionGroupPermutations.push_back(std::pair<std::string, std::string>(collisionGroup1, collisionGroup2));
                }
            }
            for(std::pair<std::string, std::string> collisionGroup : collisionGroupPermutations)
            {
                auto filterIterators = m_impl->m_collisionFilterMap.equal_range(collisionGroup);
                for(auto it = filterIterators.first; it != filterIterators.second; ++it)        // Foreach CollisionFilter object
                {
                    it->second.addCollision(std::pair<EntityId, EntityId>(entityId1, entityId2));
                }
            }
        }


/*
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
        }*/
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

void
CollisionSystem::registerCollisionFilter(
    CollisionFilter& collisionFilter
) {
    m_impl->m_collisionFilterMap.insert(std::pair<std::pair<std::string, std::string>,CollisionFilter&>(collisionFilter.getCollisionSignature(), collisionFilter));
}


#pragma once

#include "engine/component.h"
#include "engine/system.h"
#include "engine/entity.h"

#include <iostream>

namespace luabind {
class scope;
}

namespace thrive {

/**
* @brief A component for a collision reactive entity
*/
class CollisionHandlerComponent : public Component {
    COMPONENT(CollisionHandlerComponent)

public:


    /**
    * @brief Lua bindings
    *
    * Exposes:
    * - RigidBodyComponent()
    * - m_collisionCallbackKey
    *
    * @return
    */
    static luabind::scope
    luaBindings();


    /**
    * @brief Loads the component
    *
    * @param storage
    */
    void
    load(
        const StorageContainer& storage
    ) override;


    /**
    * @brief Serializes the component
    *
    * @return
    */
    StorageContainer
    storage() const override;

    /**
    * @brief The string key for the callback function to be called.
    *
    *   The callback function must be known before any collisions can occur and cannot be serialized directly, so a map key is used.
    *   Call CollisionSystem::registerCollisionCallback(key, function<void (Entity& 1, Entity& 2)>) before game is fully initialized to register your callback.
    */
    std::string m_collisionCallbackKey = "";

private:

    std::vector<std::string> collisionGroups;

};



class CollisionSystem : public System {

public:

    /**
    * @brief Constructor
    */
    CollisionSystem();

    /**
    * @brief Destructor
    */
    ~CollisionSystem();

    /**
    * @brief Initializes the engine
    *
    * @param engine
    */
    void init(
        Engine* engine
    ) override;

    /**
    * @brief Shuts the system down
    */
    void shutdown() override;

    /**
    * @brief Updates the system
    *
    * @param milliSeconds
    */
    void update(
        int milliSeconds
    ) override;

    /**
    * @brief Typedef for collision callback function type and signature
    */
    using CollisionCallback = std::function<void (EntityId selfId, EntityId opponentId)>;

    /**
    * @brief Register a collision callback.
    *
    *  Can only be successfully called before game is fully initialized as registration is required before collisions can occur.
    *  Throws std::invalid_argument if key is already registered.
    *  Throws std::bad_function_call if called after game has started running.
    *
    * @param key
    *   The string key to be used for reference to the callback
    *
    * @param callback
    *   The function to call. Full type is std::function<void (Entity& self, Entity& opponent)>.
    */
    static void registerCollisionCallback(
        const std::string& key,
        CollisionCallback callback
    );

    /**
    * @brief Register a collision filter.
    *
    *  Once a collision filter is registered it will automatically receive new relevant collisions.
    *  Throws std::invalid_argument if key is already registered.
    *  Throws std::bad_function_call if called after game has started running.
    *
    * @param key
    *   The string key to be used for reference to the callback
    *
    * @param callback
    *   The function to call. Full type is std::function<void (Entity& self, Entity& opponent)>.
    */
    void
    registerCollisionFilter(
        CollisionFilter& collisionFilter
    );

private:

    struct Implementation;
    std::unique_ptr<Implementation> m_impl;

};

}

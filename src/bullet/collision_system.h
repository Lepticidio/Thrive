#pragma once

#include "engine/component.h"
#include "engine/entity.h"
#include "engine/system.h"
#include "bullet/collision_filter.h"

#include <vector>
#include <iostream>

namespace luabind {
class scope;
}

namespace thrive {

class CollisionFilter;

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

    void
    addCollisionGroup(
        const std::string& group
    );

    const std::vector<std::string>&
    getCollisionGroups();

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

private:

    std::vector<std::string> m_collisionGroups;

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
    * @brief Register a collision filter.
    *
    *  Once a collision filter is registered it will automatically receive new relevant collisions.
    *
    * @param collisionFilter
    *   The filter to register
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

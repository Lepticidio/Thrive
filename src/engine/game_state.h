#pragma once

#include <memory>
#include <vector>

class btDiscreteDynamicsWorld;

namespace luabind {
    class scope;
}

namespace Ogre {
    class SceneManager;
}

namespace thrive {

class Engine;
class EntityManager;
class StorageContainer;
class System;

class GameState {

public:

    static luabind::scope
    luaBindings();

    GameState(
        Engine& engine,
        std::string name,
        std::vector<std::unique_ptr<System>> systems
    );

    GameState(const GameState&) = delete;

    ~GameState();

    void
    activate();

    void
    deactivate();

    Engine&
    engine();

    const Engine&
    engine() const;

    EntityManager&
    entityManager();

    const EntityManager&
    entityManager() const;

    void
    init();

    void
    load(
        const StorageContainer& storage
    );

    /**
    * @brief The physics world
    */
    btDiscreteDynamicsWorld*
    physicsWorld() const;

    /**
    * @brief The Ogre scene manager
    */
    Ogre::SceneManager*
    sceneManager() const;

    /**
    * @brief Enables or disables physics debug drawing
    *
    * @param enabled
    */
    void
    setPhysicsDebugDrawingEnabled(
        bool enabled
    );

    void
    shutdown();

    StorageContainer
    storage() const;

    void
    update(
        int milliseconds
    );

private:

    struct Implementation;
    std::unique_ptr<Implementation> m_impl;

    
};

}

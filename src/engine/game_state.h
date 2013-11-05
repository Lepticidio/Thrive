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

/**
* @brief Represents a distinct set of active systems and entities
*
* The game has to switch between different states. Examples of a state are
* "main menu", "microbe gameplay" or "microbe editor". These states usually
* share very few entities and even fewer systems, so it is sensible to 
* separate them completely (and, if necessary, share data over other channels).
*
* Each GameState has its own EntityManager and its own set of systems. Game 
* states are identified by their name, a unique string.
*
* GameStates cannot be created directly. Use Engine::addGameState to create 
* new GameStates.
*/
class GameState {

public:

    /**
    * @brief Lua bindings
    *
    * Currently exposes nothing, but GameState is used as an opaque 
    * parameter and thus needs to be exported as a class.
    *
    * @return 
    */
    static luabind::scope
    luaBindings();

    /**
    * @brief Not copyable
    *
    */
    GameState(const GameState&) = delete;

    /**
    * @brief Destructor
    */
    ~GameState();

    /**
    * @brief Not copyable
    *
    */
    GameState& operator=(const GameState&) = delete;

    /**
    * @brief Returns the engine this game state belongs to
    *
    * @return 
    */
    Engine&
    engine();

    /**
    * @brief Returns the engine this game state belongs to
    *
    * @return 
    */
    const Engine&
    engine() const;

    /**
    * @brief Returns the game state's entity manager
    *
    * @return 
    */
    EntityManager&
    entityManager();

    /**
    * @brief Returns the game state's entity manager
    *
    * @return 
    */
    const EntityManager&
    entityManager() const;

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

private:

    friend class Engine;

    /**
    * @brief Constructor
    *
    * Use Engine::addGameState to create new game states
    *
    * @param engine
    *   The engine this game state belongs to
    *
    * @param name
    *   The game state's name, a unique identifier
    *
    * @param systems
    *   The game state's systems. The game state takes ownership of them.
    */
    GameState(
        Engine& engine,
        std::string name,
        std::vector<std::unique_ptr<System>> systems
    );

    /**
    * @brief Called by the engine when the game state is activated
    */
    void
    activate();

    /**
    * @brief Called by the engine when the game state is deactivated
    */
    void
    deactivate();

    /**
    * @brief Called by the engine to initialize the game state
    *
    * Initializes all the systems in turn
    */
    void
    init();

    /**
    * @brief Called by the engine during loading of a savegame
    *
    * @param storage
    *
    * @see GameState::storage()
    */
    void
    load(
        const StorageContainer& storage
    );

    /**
    * @brief Called by the engine to shut the game state down
    *
    * Shuts down all the systems in turn
    */
    void
    shutdown();

    /**
    * @brief Called by the engine during savegame creation
    *
    * @return 
    *
    * @see GameState::load()
    */
    StorageContainer
    storage() const;

    /**
    * @brief Called by the engine to update the game state
    *
    * Updates all the systems in turn
    *
    * @param milliseconds
    *   The number of milliseconds of game time elapsed since the
    *   last frame (which may have been rendered by another game state).
    */
    void
    update(
        int milliseconds
    );

    struct Implementation;
    std::unique_ptr<Implementation> m_impl;

    
};

}

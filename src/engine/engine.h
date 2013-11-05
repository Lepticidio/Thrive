#pragma once

#include "engine/typedefs.h"

#include <memory>
#include <vector>

class btDiscreteDynamicsWorld;
class lua_State;

namespace luabind {
    class scope;
}

namespace Ogre {
    class RenderWindow;
    class Root;
    class SceneManager;
    class Viewport;
}

namespace OIS {
    class InputManager;
}

namespace thrive {

class ComponentFactory;
class EntityManager;
class GameState;
class Keyboard;
class Mouse;
class OgreViewportSystem;
class System;

/**
* @brief The heart of the game
*
* The engine keeps an ordered list of System objects and updates them each 
* frame. It handles initialization and shutdown of graphics, physics, scripts
* and more.
*/
class Engine {

public:
    
    /**
    * @brief Lua bindings
    *
    * Exposes:
    * - Engine::addGameState()
    * - Engine::load()
    * - Engine::save()
    * - Engine::componentFactory() (as property)
    * - Engine::keyboard() (as property)
    * - Engine::mouse() (as property)
    * - Engine::sceneManager() (as property)
    *
    * @return 
    */
    static luabind::scope
    luaBindings();

    /**
    * @brief Constructor
    */
    Engine();

    /**
    * @brief Non-copyable
    *
    */
    Engine(const Engine& other) = delete;

    /**
    * @brief Destructor
    */
    ~Engine();

    /**
    * @brief Creates a new game state
    *
    * @param name
    *   The game state's name
    *
    * @param systems
    *   The systems active in the game state
    */
    void
    addGameState(
        std::string name,
        std::vector<std::unique_ptr<System>> systems
    );

    /**
    * @brief Returns the internal component factory
    *
    * @return 
    */
    ComponentFactory&
    componentFactory();

    /**
    * @brief Returns the currently active game state
    *
    * If no game state has been set yet, returns \c nullptr
    *
    */
    GameState*
    currentGameState() const;

    /**
    * @brief Initializes the engine
    *
    * This sets up basic data structures for the different engine parts 
    * (input, graphics, physics, etc.) and then calls System::init() on
    * all systems.
    */
    void
    init();

    /**
    * @brief The engine's input manager
    */
    OIS::InputManager*
    inputManager() const;

    /**
    * @brief Returns the keyboard interface
    *
    */
    const Keyboard&
    keyboard() const;

    /**
    * @brief Loads a savegame
    *
    * @param filename
    *   The file to load
    */
    void
    load(
        std::string filename
    );

    /**
    * @brief The script engine's Lua state
    */
    lua_State*
    luaState();

    /**
    * @brief Returns the mouse interface
    *
    */
    const Mouse&
    mouse() const;

    /**
    * @brief The Ogre root object
    */
    Ogre::Root*
    ogreRoot() const;

    /**
    * @brief Creates a savegame
    *
    * @param filename
    *   The file to save
    */
    void
    save(
        std::string filename
    );

    /**
    * @brief Sets the current game state
    *
    * Activating a game state that has not been added with addGameState() yet
    * is an error.
    *
    * @param name
    *   The new game state's id
    */
    void
    setCurrentGameState(
        std::string name
    );

    /**
    * @brief Shuts the engine down
    *
    * This calls System::shutdown() on all systems and then destroys the data
    * structures created in Engine::init().
    */
    void 
    shutdown();

    /**
    * @brief Renders a single frame
    *
    * Before calling update() the first time, you need to call Engine::init().
    *
    * @param milliseconds
    *   The number of milliseconds to advance. For real-time, this is the
    *   number of milliseconds since the last frame.
    */
    void 
    update(
        int milliseconds
    );

    /**
    * @brief The render window
    */
    Ogre::RenderWindow*
    renderWindow() const;

private:
    
    struct Implementation;
    std::unique_ptr<Implementation> m_impl;

};

}

#include "engine/engine.h"

#include "engine/component_collection.h"
#include "engine/component_factory.h"
#include "engine/entity_manager.h"
#include "engine/game_state.h"
#include "engine/serialization.h"
#include "engine/system.h"
#include "game.h"

// Bullet
#include "bullet/bullet_to_ogre_system.h"
#include "bullet/rigid_body_system.h"
#include "bullet/update_physics_system.h"

// Ogre
#include "ogre/camera_system.h"
#include "ogre/keyboard.h"
#include "ogre/light_system.h"
#include "ogre/mouse.h"
#include "ogre/render_system.h"
#include "ogre/scene_node_system.h"
#include "ogre/sky_system.h"
#include "ogre/text_overlay.h"

// Scripting
#include "scripting/luabind.h"
#include "scripting/lua_state.h"
#include "scripting/script_initializer.h"


// Microbe
#include "microbe_stage/agent.h"

#include "util/contains.h"
#include "util/pair_hash.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <ctime>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <luabind/adopt_policy.hpp>
#include <OgreConfigFile.h>
#include <OgreLogManager.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreWindowEventUtilities.h>
#include <OISInputManager.h>
#include <OISMouse.h>
#include <random>
#include <set>
#include <stdlib.h>
#include <unordered_map>

#include <iostream>

using namespace thrive;

static const char* RESOURCES_CFG = "resources.cfg";
static const char* PLUGINS_CFG   = "plugins.cfg";

////////////////////////////////////////////////////////////////////////////////
// Engine
////////////////////////////////////////////////////////////////////////////////

struct Engine::Implementation : public Ogre::WindowEventListener {

    Implementation(
        Engine& engine
    ) : m_engine(engine)
    {
    }

    ~Implementation() {
        Ogre::WindowEventUtilities::removeWindowEventListener(
            m_graphics.renderWindow,
            this
        );
    }

    void
    loadSavegame() {
        std::ifstream stream(
            m_serialization.loadFile,
            std::ifstream::binary
        );
        m_serialization.loadFile = "";
        stream.clear();
        stream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        StorageContainer savegame;
        try {
            stream >> savegame;
        }
        catch(const std::ofstream::failure& e) {
            std::cerr << "Error loading file: " << e.what() << std::endl;
            throw;
        }
        StorageContainer gameStates = savegame.get<StorageContainer>("gameStates");
        for (const auto& pair : m_gameStates) {
            if (gameStates.contains(pair.first)) {
                pair.second->load(
                    gameStates.get<StorageContainer>(pair.first)
                );
            }
            else {
                pair.second->entityManager().clear();
            }
        }
    }

    void
    loadOgreConfig() {
        if(not (m_graphics.root->restoreConfig() or m_graphics.root->showConfigDialog()))
        {
            exit(EXIT_SUCCESS);
        }
    }

    void
    loadResources() {
        Ogre::ConfigFile config;
        config.load(RESOURCES_CFG);
        auto sectionIter = config.getSectionIterator();
        auto& resourceManager = Ogre::ResourceGroupManager::getSingleton();
        while (sectionIter.hasMoreElements()) {
            std::string sectionName = sectionIter.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* sectionContent = sectionIter.getNext();
            for(auto& setting : *sectionContent) {
                std::string resourceType = setting.first;
                std::string resourceLocation = setting.second;
                resourceManager.addResourceLocation(
                    resourceLocation,
                    resourceType,
                    sectionName
                );
            }
        }
    }

    void
    loadScripts(
        const boost::filesystem::path& directory
    ) {
        namespace fs = boost::filesystem;
        fs::path manifestPath = directory / "manifest.txt";
        if (not fs::exists(manifestPath)) {
            return;
        }
        std::ifstream manifest(manifestPath.string());
        if (not manifest.is_open()) {
            throw std::runtime_error("Could not open manifest file: " + manifestPath.string());
        }
        std::string line;
        while(not manifest.eof()) {
            std::getline(manifest, line);
            boost::algorithm::trim(line);
            if (line.empty() or line.find("//") == 0) {
                continue;
            }
            fs::path manifestEntryPath = directory / line;
            if (not fs::exists(manifestEntryPath)) {
                std::cerr << "Warning: Could not find file " << manifestEntryPath.string() << std::endl;
                continue;
            }
            else if (fs::is_directory(manifestEntryPath)) {
                this->loadScripts(manifestEntryPath);
            }
            else {
                int error = 0;
                error = luaL_loadfile(
                    m_luaState, 
                    manifestEntryPath.string().c_str()
                );
                error = error or luabind::detail::pcall(m_luaState, 0, LUA_MULTRET);
                if (error) {
                    std::string errorMessage = lua_tostring(m_luaState, -1);
                    lua_pop(m_luaState, 1);
                    std::cerr << errorMessage << std::endl;
                }
            }
        }
    }

    bool
    quitRequested() {
        return m_input.keyboard.isKeyDown(
            OIS::KeyCode::KC_ESCAPE
        );
    }

    void
    saveSavegame() {
        StorageContainer savegame;
        StorageContainer gameStates;
        for (const auto& pair : m_gameStates) {
            gameStates.set(pair.first, pair.second->storage());
        }
        savegame.set("gameStates", std::move(gameStates));
        std::ofstream stream(
            m_serialization.saveFile, 
            std::ofstream::trunc | std::ofstream::binary
        );
        m_serialization.saveFile = "";
        stream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        if (stream) {
            try {
                stream << savegame;
                stream.flush();
                stream.close();
            }
            catch (const std::ofstream::failure& e) {
                std::cerr << "Error saving file: " << e.what() << std::endl;
                throw;
            }
        }
        else {
            std::perror("Could not open file for saving");
        }
    }

    void
    setupGraphics() {
        m_graphics.root.reset(new Ogre::Root(PLUGINS_CFG));
        this->loadResources();
        this->loadOgreConfig();
        m_graphics.renderWindow = m_graphics.root->initialise(true, "Thrive");
        m_input.mouse.setWindowSize(
            m_graphics.renderWindow->getWidth(),
            m_graphics.renderWindow->getHeight()
        );
        Ogre::WindowEventUtilities::addWindowEventListener(
            m_graphics.renderWindow,
            this
        );
        // Set default mipmap level (NB some APIs ignore this)
        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        // initialise all resource groups
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }

    void
    setupInputManager() {
        const std::string HANDLE_NAME = "WINDOW";
        size_t windowHandle = 0;
        m_graphics.renderWindow->getCustomAttribute(HANDLE_NAME, &windowHandle);
        OIS::ParamList parameters;
        parameters.insert(std::make_pair(
            HANDLE_NAME,
            boost::lexical_cast<std::string>(windowHandle)
        ));
#if defined OIS_WIN32_PLATFORM
        parameters.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
        parameters.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
        parameters.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
        parameters.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
        parameters.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
        parameters.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
        parameters.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
        parameters.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif
        m_input.inputManager = OIS::InputManager::createInputSystem(parameters);
        m_input.keyboard.init(m_input.inputManager);
        m_input.mouse.init(m_input.inputManager);
    }

    void
    setupLog() {
        static Ogre::LogManager logManager;
        logManager.createLog("default", true, false, false);
    }

    void
    setupScripts() {
        initializeLua(m_luaState);
    }
/*
    void
    setupSystems() {
        std::shared_ptr<System> systems[] = {
            // Loading, this should be first
            m_loadSystem,
            // Input
            m_input.keyboardSystem,
            m_input.mouseSystem,
            // Scripts
            m_scriptSystemUpdater,
            // Microbe
            std::make_shared<AgentLifetimeSystem>(),
            std::make_shared<AgentMovementSystem>(),
            std::make_shared<AgentEmitterSystem>(),
            std::make_shared<AgentAbsorberSystem>(),
            // Physics
            std::make_shared<RigidBodyInputSystem>(),
            std::make_shared<UpdatePhysicsSystem>(),
            std::make_shared<RigidBodyOutputSystem>(),
            std::make_shared<BulletToOgreSystem>(),
            m_physics.debugDrawSystem,
            // Graphics
            std::make_shared<OgreAddSceneNodeSystem>(),
            std::make_shared<OgreUpdateSceneNodeSystem>(),
            std::make_shared<OgreCameraSystem>(),
            std::make_shared<OgreLightSystem>(),
            std::make_shared<SkySystem>(),
            std::make_shared<TextOverlaySystem>(),
            m_viewportSystem, // Has to come *after* camera system
            std::make_shared<OgreRemoveSceneNodeSystem>(),
            std::make_shared<RenderSystem>(),
            // Saving, this should be last
            m_saveSystem
        };
        for (auto system : systems) {
            this->addSystem(system);
        }
    }
*/
    void
    shutdownInputManager() {
        if (not m_input.inputManager) {
            return;
        }
        OIS::InputManager::destroyInputSystem(m_input.inputManager);
        m_input.inputManager = nullptr;
    }

    bool 
    windowClosing(
        Ogre::RenderWindow* window
    ) override {
        if (window == m_graphics.renderWindow) {
            Game::instance().quit();
        }
        return true;
    }

    void
    windowResized(
        Ogre::RenderWindow* window
    ) override {
        if (window == m_graphics.renderWindow) {
            m_input.mouse.setWindowSize(
                window->getWidth(),
                window->getHeight()
            );
        }
    }

    // Lua state must be one of the last to be destroyed, so keep it at top. 
    // The reason for that is that some components keep luabind::object 
    // instances around that rely on the lua state to still exist when they
    // are destroyed. Since those components are destroyed with the entity 
    // manager, the lua state has to live longer than the manager.
    LuaState m_luaState;

    GameState* m_currentGameState = nullptr;

    ComponentFactory m_componentFactory;

    Engine& m_engine;

    std::map<std::string, std::unique_ptr<GameState>> m_gameStates;

    struct Graphics {

        std::unique_ptr<Ogre::Root> root;

        Ogre::RenderWindow* renderWindow = nullptr;

    } m_graphics;

    struct Input {

        OIS::InputManager* inputManager = nullptr;

        Keyboard keyboard;

        Mouse mouse;

    } m_input;

    struct Serialization {

        std::string loadFile;

        std::string saveFile;

    } m_serialization;

};


static void
Engine_addGameState(
    Engine* self,
    std::string name,
    luabind::object luaSystems
) {
    std::vector<std::unique_ptr<System>> systems;
    for (luabind::iterator iter(luaSystems), end; iter != end; ++iter) {
        System* system = luabind::object_cast<System*>(
            *iter,
            luabind::adopt(luabind::result)
        );
        systems.emplace_back(system);
    }
    self->addGameState(name, std::move(systems));
}


luabind::scope
Engine::luaBindings() {
    using namespace luabind;
    return class_<Engine>("__Engine")
        .def("addGameState", Engine_addGameState)
        .def("setCurrentGameState", &Engine::setCurrentGameState)
        .def("load", &Engine::load)
        .def("save", &Engine::save)
        .property("componentFactory", &Engine::componentFactory)
        .property("keyboard", &Engine::keyboard)
        .property("mouse", &Engine::mouse)
    ;
}




Engine::Engine() 
  : m_impl(new Implementation(*this))
{
}


Engine::~Engine() { }


void
Engine::addGameState(
    std::string name,
    std::vector<std::unique_ptr<System>> systems
) {
    assert(m_impl->m_gameStates.find(name) == m_impl->m_gameStates.end() && "Duplicate GameState name");
    m_impl->m_gameStates.insert(std::make_pair(
        name, 
        make_unique<GameState>(*this, name, std::move(systems))
    ));
}


ComponentFactory&
Engine::componentFactory() {
    return m_impl->m_componentFactory;
}


GameState*
Engine::currentGameState() const {
    return m_impl->m_currentGameState;
}


void
Engine::init() {
    std::srand(unsigned(time(0)));
    m_impl->setupLog();
    m_impl->setupScripts();
    m_impl->setupGraphics();
    m_impl->setupInputManager();
    m_impl->loadScripts("../scripts");
    for (const auto& pair : m_impl->m_gameStates) {
        const auto& gameState = pair.second;
        gameState->init();
    }
}


OIS::InputManager*
Engine::inputManager() const {
    return m_impl->m_input.inputManager;
}


const Keyboard&
Engine::keyboard() const {
    return m_impl->m_input.keyboard;
}


void
Engine::load(
    std::string filename
) {
    m_impl->m_serialization.loadFile = filename;
}


const Mouse&
Engine::mouse() const {
    return m_impl->m_input.mouse;
}


Ogre::Root*
Engine::ogreRoot() const {
    return m_impl->m_graphics.root.get();
}


Ogre::RenderWindow*
Engine::renderWindow() const {
    return m_impl->m_graphics.renderWindow;
}


void
Engine::save(
    std::string filename
) {
    m_impl->m_serialization.saveFile = filename;
}


void
Engine::setCurrentGameState(
    std::string name
) {
    GameState* previousState = m_impl->m_currentGameState;
    if (previousState) {
        previousState->deactivate();
    }
    GameState* newState = m_impl->m_gameStates[name].get();
    assert(newState != nullptr && "Unknown game state");
    m_impl->m_currentGameState = newState;
    newState->activate();
}


void
Engine::shutdown() {
    for (const auto& pair : m_impl->m_gameStates) {
        const auto& gameState = pair.second;
        gameState->shutdown();
    }
    m_impl->shutdownInputManager();
    m_impl->m_graphics.renderWindow->destroy();
    m_impl->m_graphics.root.reset();
}


void
Engine::update(
    int milliseconds
) {
    Ogre::WindowEventUtilities::messagePump();
    if (m_impl->quitRequested()) {
        Game::instance().quit();
    }
    m_impl->m_input.keyboard.update();
    m_impl->m_input.mouse.update();
    assert(m_impl->m_currentGameState != nullptr);
    m_impl->m_currentGameState->update(milliseconds);
}


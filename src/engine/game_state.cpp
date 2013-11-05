#include "engine/game_state.h"

#include "engine/engine.h"
#include "engine/entity_manager.h"
#include "engine/serialization.h"
#include "ogre/viewport_system.h"

#include <btBulletDynamicsCommon.h>
#include <OgreRoot.h>

using namespace thrive;

struct GameState::Implementation {

    Implementation(
        Engine& engine,
        std::string name,
        std::vector<std::unique_ptr<System>> systems
    ) : m_engine(engine),
        m_name(name),
        m_systems(std::move(systems))
    {
        m_viewportSystem = this->findSystem<OgreViewportSystem>();
    }

    template<typename S>
    S*
    findSystem() {
        for (const auto& system : m_systems) {
            S* result = dynamic_cast<S*>(system.get());
            if (result) {
                return result;
            }
        }
        return nullptr;
    }

    void
    setupPhysics() {
        m_physics.collisionConfiguration.reset(new btDefaultCollisionConfiguration());
        m_physics.dispatcher.reset(new btCollisionDispatcher(
            m_physics.collisionConfiguration.get()
        ));
        m_physics.broadphase.reset(new btDbvtBroadphase());
        m_physics.solver.reset(new btSequentialImpulseConstraintSolver());
        m_physics.world.reset(new btDiscreteDynamicsWorld(
            m_physics.dispatcher.get(),
            m_physics.broadphase.get(),
            m_physics.solver.get(),
            m_physics.collisionConfiguration.get()
        ));
        m_physics.world->setGravity(btVector3(0,0,0));
    }

    void
    setupSceneManager() {
        m_sceneManager = m_engine.ogreRoot()->createSceneManager(
            Ogre::ST_GENERIC,
            m_name
        );
        m_sceneManager->setAmbientLight(
            Ogre::ColourValue(0.5, 0.5, 0.5)
        );
    }

    Engine& m_engine;

    EntityManager m_entityManager;

    std::string m_name;

    Ogre::SceneManager* m_sceneManager = nullptr;

    struct Physics {

        std::unique_ptr<btBroadphaseInterface> broadphase;

        std::unique_ptr<btCollisionConfiguration> collisionConfiguration;

        std::unique_ptr<btDispatcher> dispatcher;

        std::unique_ptr<btConstraintSolver> solver;

        std::unique_ptr<btDiscreteDynamicsWorld> world;

    } m_physics;

    std::vector<std::unique_ptr<System>> m_systems;

    OgreViewportSystem* m_viewportSystem = nullptr;

};


luabind::scope
GameState::luaBindings() {
    using namespace luabind;
    return class_<GameState>("GameState")
    ;
}


GameState::GameState(
    Engine& engine,
    std::string name,
    std::vector<std::unique_ptr<System>> systems
) : m_impl(new Implementation(engine, name, std::move(systems)))
{
}


GameState::~GameState() {}


void
GameState::activate() {
    if (m_impl->m_viewportSystem) {
        m_impl->m_viewportSystem->restoreAllViewports();
    }
}


void
GameState::deactivate() {
    if (m_impl->m_viewportSystem) {
        m_impl->m_viewportSystem->removeAllViewports();
    }
}


Engine&
GameState::engine() {
    return m_impl->m_engine;
}


const Engine&
GameState::engine() const {
    return m_impl->m_engine;
}


EntityManager&
GameState::entityManager() {
    return m_impl->m_entityManager;
}


void
GameState::init() {
    m_impl->setupPhysics();
    m_impl->setupSceneManager();
    for (const auto& system : m_impl->m_systems) {
        system->init(this);
    }
}


void
GameState::load(
    const StorageContainer& storage
) {
    StorageContainer entities = storage.get<StorageContainer>("entities");
    m_impl->m_entityManager.clear();
    try {
        m_impl->m_entityManager.restore(
            entities,
            m_impl->m_engine.componentFactory()
        );
    }
    catch (const luabind::error& e) {
        luabind::object error_msg(luabind::from_stack(
            e.state(),
            -1
        ));
        // TODO: Log error
        std::cerr << error_msg << std::endl;
        throw;
    }
}


btDiscreteDynamicsWorld*
GameState::physicsWorld() const {
    return m_impl->m_physics.world.get();
}


Ogre::SceneManager*
GameState::sceneManager() const {
    return m_impl->m_sceneManager;
}


void
GameState::setPhysicsDebugDrawingEnabled(
    bool enabled
) {
    (void) enabled;
    //m_impl->m_physics.debugDrawSystem->setActive(enabled);
}


void
GameState::shutdown() {
    for (const auto& system : m_impl->m_systems) {
        system->shutdown();
    }
    m_impl->m_physics.world.reset();
    m_impl->m_engine.ogreRoot()->destroySceneManager(
        m_impl->m_sceneManager
    );
}


StorageContainer
GameState::storage() const {
    StorageContainer storage;
    StorageContainer entities;
    try {
        entities = m_impl->m_entityManager.storage(
            m_impl->m_engine.componentFactory()
        );
    }
    catch (const luabind::error& e) {
        luabind::object error_msg(luabind::from_stack(
            e.state(),
            -1
        ));
        // TODO: Log error
        std::cerr << error_msg << std::endl;
        throw;
    }
    storage.set("entities", std::move(entities));
    return storage;
}


void
GameState::update(
    int milliseconds
) {
    for(auto& system : m_impl->m_systems) {
        if (system->active()) {
            system->update(milliseconds);
        }
    }
    m_impl->m_entityManager.processRemovals();
}

#pragma once

#include "bullet/collision_system.h"

#include <iostream>
#include <string>

#include <assert.h>
#include "engine/typedefs.h"
#include <unordered_set>
#include <utility>

#include "util/pair_hash.h"

namespace thrive {

class CollisionSystem;

/**
* @brief Filters for entities that contain specific components
*
* An entity filter helps a system in finding the entities that have exactly
* the right components to be relevant for the system.
*
* @tparam ComponentTypes
*   The component classes to watch for. You can wrap a class with the
*   Optional template if you want to know if it's there, but it's not
*   required.
*
* Usage example:
* \code
* class MySystem : public System {
*
* private:
*
*   EntityFilter<
*       MyComponent,
*       Optional<SomeOtherComponent>
*   >
*   m_entities;
*
* public:
*
*   void init(Engine* engine) override {
*       System::init(engine);
*       m_entities.setEntityFilter(&engine->entityManager());
*   }
*
*   void update(int milliseconds) override {
*       for (auto& value : m_entities) {
*           EntityId entity = value.first;
*           MyComponent* myComponent = std::get<0>(value.second);
*           // Do something with myComponent
*           SomeOtherComponent* someOtherComponent = std::get<1>(value.second);
*           if (someOtherComponent) {
*               // Do something with someOtherComponent
*           }
*       }
*   }
*
*   void shutdown() overrde {
*       m_entities.setEntityManager(nullptr);
*       System::shutdown();
*   }
* };
* \endcode
*/

class CollisionFilter {

public:

    /**
    * @brief Constructor
    *
    * @param recordChanges
    *   If \c true, you can query the added and removed entities through
    *   this filter.
    *
    * @warning
    *   If \a recordChanges is true, you are responsible for clearing the
    *   collections returned by addedEntities() and removedEntities().
    *   If you don't clear them regularly, it's a memory leak.
    *   You can use EntityFilter::clearChanges() to clear both collections.
    */
    CollisionFilter(
        const std::string& collisionGroup1,
        const std::string& collisionGroup2,
        CollisionSystem* collisionSystem
    );


    ~CollisionFilter()= default;


    /**
    * @brief Returns the entities added to this filter
    *
    * When you have processed the collection, please call clear() on
    * it.
    *
    */
    std::unordered_set<Collision>&
    collisions();

    /**
    * @brief Clears the lists for added and removed entities
    */
    void
    clearCollisions();

    void
    addCollision(Collision collision);

    /**
    * @brief Iterator
    *
    * Equivalent to
    * \code
    * entities().cbegin()
    * \endcode
    *
    * @return An iterator to the first relevant entity
    */
    typename std::unordered_set<Collision>::const_iterator
    begin() const;

    /**
    * @brief Iterator
    *
    * Equivalent to
    * \code
    * entities().cend()
    * \endcode
    *
    * @return An iterator to the end of the relevant entities
    */
    typename std::unordered_set<Collision>::const_iterator
    end() const;

    /**
    * @brief Sets the entity manager this filter applies to
    *
    * @param entityManager
    *   The new entity manager to listen to. If \c nullptr, the filter stays
    *   empty.
    */
    void
    setCollisionSystem(
        CollisionSystem* collisionSystem
    );

    std::pair<const std::string&, const std::string&>
    getCollisionSignature() const;

private:

    struct Implementation;
    std::unique_ptr<Implementation> m_impl;

};

}

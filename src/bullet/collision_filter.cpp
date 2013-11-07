#include "collision_filter.h"

using namespace thrive;



struct CollisionFilter::Implementation {

    Implementation(
        const std::string& collisionGroup1,
        const std::string& collisionGroup2,
        CollisionSystem* collisionSystem
    ) : m_collisionGroup1(collisionGroup1),
        m_collisionGroup2(collisionGroup2),
        m_collisionSystem(collisionSystem)
    {
    }

    std::unordered_set<Collision> m_collisions;

    std::string m_collisionGroup1;
    std::string m_collisionGroup2;

    CollisionSystem* m_collisionSystem;

};


CollisionFilter::CollisionFilter(
    const std::string& collisionGroup1,
    const std::string& collisionGroup2,
    CollisionSystem* collisionSystem
) : m_impl(new Implementation(collisionGroup1, collisionGroup2, collisionSystem))
{
    collisionSystem->registerCollisionFilter(*this);
}


std::unordered_set<Collision>&
CollisionFilter::collisions() {
    return m_impl->m_collisions;
}

void
CollisionFilter::addCollision(
    Collision collision
) {
    m_impl->m_collisions.insert(collision);
}

typename std::unordered_set<Collision>::const_iterator
CollisionFilter::begin() const {
    return m_impl->m_collisions.cbegin();
}


typename std::unordered_set<Collision>::const_iterator
CollisionFilter::end() const {
    return m_impl->m_collisions.cend();
}


void
CollisionFilter::clearCollisions() {
    m_impl->m_collisions.clear();
}


void
CollisionFilter::setCollisionSystem(
    CollisionSystem* collisionSystem
) {
    m_impl->m_collisions.clear();
    m_impl->m_collisionSystem = collisionSystem;
    collisionSystem->registerCollisionFilter(*this);
}

std::pair<const std::string&, const std::string&>
CollisionFilter::getCollisionSignature() const {
    return std::pair<const std::string&, const std::string&>(m_impl->m_collisionGroup1, m_impl->m_collisionGroup2);
}


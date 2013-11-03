#include "engine/saving.h"

#include "engine/component_collection.h"
#include "engine/component_factory.h"
#include "engine/engine.h"
#include "engine/entity_manager.h"
#include "engine/serialization.h"

#include <string>

using namespace thrive;

struct SaveSystem::Implementation {

    std::string m_filename;

};


SaveSystem::SaveSystem()
  : m_impl(new Implementation())
{
}


SaveSystem::~SaveSystem() {}


void
SaveSystem::save(
    std::string filename
) {
    m_impl->m_filename = filename;
    this->setActive(true);
}


void
SaveSystem::update(int) {
}


////////////////////////////////////////////////////////////////////////////////
// LoadSystem
////////////////////////////////////////////////////////////////////////////////

struct LoadSystem::Implementation {

    std::string m_filename;

};


LoadSystem::LoadSystem()
  : m_impl(new Implementation())
{
}


LoadSystem::~LoadSystem() {}


void
LoadSystem::load(
    std::string filename
) {
    m_impl->m_filename = filename;
    this->setActive(true);
}


void
LoadSystem::update(int) {
}

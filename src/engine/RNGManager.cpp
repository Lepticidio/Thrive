#include "RNGManager.h"

using namespace thrive;

struct RNGManager::Implementation {
    Implementation(
        RNGSeed seed
    ) : m_seed(seed)
    {
        m_mt = std::mt19937(seed);
    }

    RNGSeed m_seed;
    std::random_device m_rd;
    std::mt19937 m_mt;
    std::uniform_real_distribution<double> m_realDist; //default range [0,1]
};


luabind::scope
RNGManager::luaBindings(){
    using namespace luabind;
    return class_<RNGManager>("RNGManager")
        .def(constructor<>())
        .def(constructor<int>())
   //     .def("rand", reinterpret_cast<int(RNGManager::*)(int, int)>(&RNGManager::rand)) //Looks like C casts are needed, but treated as error with compiler flag.
    //    .def("rand", reinterpret_cast<double(RNGManager::*)()>(&RNGManager::rand))
        .def("generateRandomSeed", &RNGManager::generateRandomSeed)
        .def("setSeed", &RNGManager::setSeed)
        .def("getSeed", &RNGManager::getSeed)
    ;
}

RNGManager::RNGManager() { //Cannot use constructor delegation or initialization lists because random_device object is needed.
    std::random_device rd;
    m_impl = new Implementation(rd());
}


RNGManager::RNGManager(
    RNGSeed seed
) : m_impl(new Implementation(seed))
{
}


void
RNGManager::setSeed(
    RNGSeed seed
) {
    m_impl->m_seed = seed;
    m_impl->m_mt.seed(seed);
}

RNGSeed
RNGManager::getSeed() const {
    return m_impl->m_seed;
}

RNGSeed
RNGManager::generateRandomSeed() {
    return m_impl->m_rd();
}


double
RNGManager::getDouble() {
    return m_impl->m_realDist(m_impl->m_mt);
}

double
RNGManager::getDoubleBetween(
    double min,
    double max
) {
    std::uniform_real_distribution<double> dis(min, max);
    return dis(m_impl->m_mt);
}


int
RNGManager::getIntBetween(
    int min,
    int max
) {
    std::uniform_int_distribution<int> dis(min, max);
    return dis(m_impl->m_mt);
}





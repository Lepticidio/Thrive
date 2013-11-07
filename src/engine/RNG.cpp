#include "RNG.h"

using namespace thrive;

struct RNG::Implementation {
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
RNG::luaBindings(){
    using namespace luabind;
    return class_<RNG>("RNG")
        .def(constructor<>())
        .def(constructor<int>())
   //     .def("rand", reinterpret_cast<int(RNG::*)(int, int)>(&RNG::rand)) //Looks like C casts are needed, but treated as error with compiler flag.
    //    .def("rand", reinterpret_cast<double(RNG::*)()>(&RNG::rand))
        .def("generateRandomSeed", &RNG::generateRandomSeed)
        .def("setSeed", &RNG::setSeed)
        .def("getSeed", &RNG::getSeed)
    ;
}

RNG::RNG() { //Cannot use constructor delegation or initialization lists because random_device object is needed.
    std::random_device rd;
    m_impl = new Implementation(rd());
}


RNG::RNG(
    RNGSeed seed
) : m_impl(new Implementation(seed))
{
}


void
RNG::setSeed(
    RNGSeed seed
) {
    m_impl->m_seed = seed;
    m_impl->m_mt.seed(seed);
}

RNGSeed
RNG::getSeed() const {
    return m_impl->m_seed;
}

RNGSeed
RNG::generateRandomSeed() {
    return m_impl->m_rd();
}


double
RNG::getDouble() {
    return m_impl->m_realDist(m_impl->m_mt);
}

double
RNG::getDoubleBetween(
    double min,
    double max
) {
    std::uniform_real_distribution<double> dis(min, max);
    return dis(m_impl->m_mt);
}


int
RNG::getIntBetween(
    int min,
    int max
) {
    std::uniform_int_distribution<int> dis(min, max);
    return dis(m_impl->m_mt);
}





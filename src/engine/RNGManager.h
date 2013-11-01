#include <random>

using RNGSeed = unsigned int;  // parts <random> uses unsigned int

namespace luabind {
class scope;
}

namespace thrive {

/**
* @brief Class for RNG
*
* Uses C++11 RNG features for optimal quality RNG.
* Can be instantiated multiple times for multiple
*/
class RNGManager final {

public:

    /**
    * @brief Lua bindings
    *
    * Exposes:
    *
    * - RNGManager::rand(min, max)
    * - RNGManager::rand()
    * - RNGManager::generateRandomSeed()
    * - RNGManager::setSeed(seed)
    * - RNGManager::getSeed()
    */
    static luabind::scope
    luaBindings();

    /**
    * @brief Constructor using proper random seed
    */
    RNGManager();

    /**
    * @brief Constructor
    *
    * @param seed
    *   The seed used for initializing the RNG
    */
    RNGManager(RNGSeed seed);

    /**
    * @brief Restarts the RNG with provided seed
    *
    * @param seed
    *   The seed used for reinitializing the RNG
    */
    void
    setSeed(RNGSeed seed);

    /**
    * @brief Returns the current seed
    *
    * @return
    *   The used seed
    */
    RNGSeed
    getSeed();


    /**
    * @brief Generates a proper random seed
    *
    * @return
    *  Seed with high amount of entropy
    */
    RNGSeed
    generateRandomSeed();

    /**
    * @brief Generates a random double between 0 and 1
    *
    * @return
    *  Double in range [0, 1]
    */
    double
    rand();

    /**
    * @brief Generates a random number between min and max
    *
    * @return
    *  int in range [min, max] inclusive
    */
    int
    rand(int min, int max);

    /**
    * @brief Shuffles iterable collection.
    *
    * @tparam iterType
    *   Iterator type
    *
    * @return
    *  int in range [min, max] inclusive
    *//*    //NOT SURE HOW TO MAKE THIS WORK (scope of Implementation definition and apparently std::shuffle doesn't exist???):
    template<typename iterType>
    void shuffle(               //cannot be virtual
        iterType first,
        iterType last
    ) {
        std::shuffle(first, last, m_impl->m_mt);
    }*/


private:

    struct Implementation;
    Implementation* m_impl;

};

}

#include <algorithm>
#include <random>
#include <scripting/luabind.h>


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
class RNG final {

public:

    /**
    * @brief Lua bindings
    *
    * Exposes:
    *
    * - RNG::rand(min, max)
    * - RNG::rand()
    * - RNG::generateRandomSeed()
    * - RNG::setSeed(seed)
    * - RNG::getSeed()
    */
    static luabind::scope
    luaBindings();

    /**
    * @brief Constructor using proper random seed
    */
    RNG();

    /**
    * @brief Constructor
    *
    * @param seed
    *   The seed used for initializing the RNG
    */
    RNG(RNGSeed seed);

    ~RNG();

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
    getSeed() const;


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
    getDouble();

    /**
    * @brief Generates a random double between min and max
    *
    * @return
    *  Double in range [min, max]
    */
    double
    getDoubleBetween(double min, double max);

    /**
    * @brief Generates a random integer between min and max
    *
    * @return
    *  int in range [min, max] inclusive
    */
    int
    getIntBetween(int min, int max);

    /**
    * @brief Shuffles iterable collection.
    *
    * @tparam iterType
    *   Iterator type
    *
    * @return
    *  int in range [min, max] inclusive
    */
    template<typename iterType>
    void shuffle(
        iterType first,
        iterType last
    ) {
        std::shuffle(first, last, mersenneTwister());
    }


private:

    std::mt19937&
    mersenneTwister();

    struct Implementation;
    std::unique_ptr<Implementation> m_impl;

};

}

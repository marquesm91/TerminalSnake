/**
 * @file random_service.hpp
 * @brief RandomService - Deterministic PRNG for replay
 * 
 * Clean Architecture: Domain Layer - Domain Service
 * - No external state
 * - Deterministic for replay
 * - Platform independent
 */

#ifndef DOMAIN_SERVICES_RANDOM_SERVICE_HPP
#define DOMAIN_SERVICES_RANDOM_SERVICE_HPP

#include <cstdint>

namespace Snake {
namespace Domain {

/**
 * @brief Deterministic random number service
 * 
 * Uses xorshift32 to guarantee same sequence on all platforms.
 * Critical for the anti-cheat replay system.
 */
class RandomService {
public:
    explicit RandomService(uint32_t seed = 1) : state_(seed ? seed : 1) {}
    
    /**
     * @brief Generate next random number
     */
    uint32_t next() {
        // xorshift32 - Marsaglia's algorithm
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return state_;
    }
    
    /**
     * @brief Generate number in range [0, max)
     */
    uint32_t nextInt(uint32_t max) {
        if (max == 0) return 0;
        return next() % max;
    }
    
    /**
     * @brief Generate number in range [min, max)
     */
    uint32_t nextIntRange(uint32_t min, uint32_t max) {
        if (min >= max) return min;
        return min + nextInt(max - min);
    }
    
    /**
     * @brief Generate float in range [0.0, 1.0)
     */
    float nextFloat() {
        return static_cast<float>(next()) / static_cast<float>(0xFFFFFFFF);
    }
    
    /**
     * @brief Return current state (for serialization)
     */
    uint32_t getState() const { return state_; }
    
    /**
     * @brief Set state (for deserialization)
     */
    void setState(uint32_t state) { state_ = state ? state : 1; }
    
    /**
     * @brief Reseta com nova seed
     */
    void reset(uint32_t seed) { state_ = seed ? seed : 1; }

private:
    uint32_t state_;
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_SERVICES_RANDOM_SERVICE_HPP

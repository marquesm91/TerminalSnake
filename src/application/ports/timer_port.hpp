/**
 * @file timer_port.hpp
 * @brief ITimer - Interface para controle de tempo
 * 
 * Clean Architecture: Application Layer - Port
 */

#ifndef APPLICATION_PORTS_TIMER_PORT_HPP
#define APPLICATION_PORTS_TIMER_PORT_HPP

#include <cstdint>

namespace Snake {
namespace Application {
namespace Ports {

/**
 * @brief Interface para serviços de tempo
 * 
 * Abstrai:
 * - std::chrono (desktop)
 * - millis() (Arduino)
 * - etc.
 */
class ITimer {
public:
    virtual ~ITimer() = default;
    
    /**
     * @brief Return current timestamp in milliseconds
     */
    virtual uint32_t currentTimeMs() const = 0;
    
    /**
     * @brief Wait for a period of time
     */
    virtual void delayMs(uint32_t ms) = 0;
    
    /**
     * @brief Calculate elapsed time since a timestamp
     */
    uint32_t elapsedSince(uint32_t startTime) const {
        return currentTimeMs() - startTime;
    }
    
    /**
     * @brief Check if time has passed
     */
    bool hasPassed(uint32_t startTime, uint32_t duration) const {
        return elapsedSince(startTime) >= duration;
    }
};

} // namespace Ports
} // namespace Application
} // namespace Snake

#endif // APPLICATION_PORTS_TIMER_PORT_HPP

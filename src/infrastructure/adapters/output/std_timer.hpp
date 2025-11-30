/**
 * @file std_timer.hpp
 * @brief StdTimer - Adaptador de tempo usando std::chrono
 * 
 * Clean Architecture: Infrastructure Layer - Adapter
 */

#ifndef INFRASTRUCTURE_ADAPTERS_OUTPUT_STD_TIMER_HPP
#define INFRASTRUCTURE_ADAPTERS_OUTPUT_STD_TIMER_HPP

#include <chrono>
#include <thread>
#include "application/ports/timer_port.hpp"

namespace Snake {
namespace Infrastructure {
namespace Adapters {

/**
 * @brief Timer usando std::chrono
 */
class StdTimer : public Application::Ports::ITimer {
public:
    StdTimer() : startTime_(std::chrono::steady_clock::now()) {}
    
    uint32_t currentTimeMs() const override {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - startTime_
        );
        return static_cast<uint32_t>(duration.count());
    }
    
    void delayMs(uint32_t ms) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    
    void reset() {
        startTime_ = std::chrono::steady_clock::now();
    }

private:
    std::chrono::steady_clock::time_point startTime_;
};

} // namespace Adapters
} // namespace Infrastructure
} // namespace Snake

#endif // INFRASTRUCTURE_ADAPTERS_OUTPUT_STD_TIMER_HPP

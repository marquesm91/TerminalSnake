/**
 * @file leaderboard_port.hpp
 * @brief ILeaderboard - Interface para leaderboard remoto
 * 
 * Clean Architecture: Application Layer - Output Port
 */

#ifndef APPLICATION_PORTS_LEADERBOARD_PORT_HPP
#define APPLICATION_PORTS_LEADERBOARD_PORT_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "domain/services/replay_service.hpp"

namespace Snake {
namespace Application {
namespace Ports {

/**
 * @brief Entrada no leaderboard
 */
struct LeaderboardEntry {
    std::string odUserId;
    std::string displayName;
    std::string photoUrl;
    uint32_t score;
    uint16_t snakeSize;
    std::string difficulty;
    std::string timestamp;
    bool verified;
};

/**
 * @brief Resultado da submissão
 */
struct SubmitResult {
    bool success;
    std::string errorMessage;
    int rank;  // -1 se não entrou no leaderboard
};

/**
 * @brief Interface para leaderboard remoto
 * 
 * Abstrai:
 * - Firebase Firestore
 * - REST API custom
 * - etc.
 */
class ILeaderboard {
public:
    virtual ~ILeaderboard() = default;
    
    /**
     * @brief Fetch top N entries
     */
    virtual std::vector<LeaderboardEntry> fetchTop(uint32_t limit = 20) = 0;
    
    /**
     * @brief Submete score com replay para validação
     */
    virtual SubmitResult submitScore(const Domain::ReplayData& replay, 
                                     uint16_t snakeSize) = 0;
    
    /**
     * @brief Get current user's rank
     */
    virtual int getUserRank(const std::string& odUserId) = 0;
    
    /**
     * @brief Check if online
     */
    virtual bool isOnline() const = 0;
};

} // namespace Ports
} // namespace Application
} // namespace Snake

#endif // APPLICATION_PORTS_LEADERBOARD_PORT_HPP

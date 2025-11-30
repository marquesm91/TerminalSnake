/**
 * @file submit_score_use_case.hpp
 * @brief SubmitScoreUseCase - Submete score ao leaderboard
 * 
 * Clean Architecture: Application Layer - Use Case
 */

#ifndef APPLICATION_USE_CASES_SUBMIT_SCORE_USE_CASE_HPP
#define APPLICATION_USE_CASES_SUBMIT_SCORE_USE_CASE_HPP

#include "application/ports/leaderboard_port.hpp"
#include "application/ports/auth_port.hpp"
#include "domain/services/replay_service.hpp"

namespace Snake {
namespace Application {
namespace UseCases {

/**
 * @brief Resultado da submissão
 */
struct SubmitScoreResult {
    bool success;
    bool authenticated;
    int rank;
    std::string errorMessage;
};

/**
 * @brief Use Case: Submeter score ao leaderboard
 */
class SubmitScoreUseCase {
public:
    SubmitScoreUseCase(Ports::ILeaderboard& leaderboard,
                       Ports::IAuth& auth)
        : leaderboard_(leaderboard), auth_(auth) {}
    
    /**
     * @brief Submete score com replay
     */
    SubmitScoreResult execute(const Domain::ReplayData& replay, uint16_t snakeSize) {
        SubmitScoreResult result;
        result.success = false;
        result.authenticated = auth_.isAuthenticated();
        result.rank = -1;
        
        // Check authentication
        if (!result.authenticated) {
            result.errorMessage = "Not authenticated";
            return result;
        }
        
        // Renew token if needed
        if (!auth_.refreshTokenIfNeeded()) {
            result.errorMessage = "Failed to refresh token";
            return result;
        }
        
        // Check connection
        if (!leaderboard_.isOnline()) {
            result.errorMessage = "No internet connection";
            return result;
        }
        
        // Validate replay locally first (optional, saves bandwidth)
        auto validation = Domain::ReplayValidator::validate(replay);
        if (!validation.valid) {
            result.errorMessage = "Local validation failed: " + validation.errorMessage;
            return result;
        }
        
        // Submete ao servidor
        auto submitResult = leaderboard_.submitScore(replay, snakeSize);
        result.success = submitResult.success;
        result.errorMessage = submitResult.errorMessage;
        result.rank = submitResult.rank;
        
        return result;
    }

private:
    Ports::ILeaderboard& leaderboard_;
    Ports::IAuth& auth_;
};

} // namespace UseCases
} // namespace Application
} // namespace Snake

#endif // APPLICATION_USE_CASES_SUBMIT_SCORE_USE_CASE_HPP

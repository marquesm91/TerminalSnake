/**
 * @file play_game_use_case.hpp
 * @brief PlayGameUseCase - Orquestra uma sessão de jogo
 * 
 * Clean Architecture: Application Layer - Use Case
 */

#ifndef APPLICATION_USE_CASES_PLAY_GAME_USE_CASE_HPP
#define APPLICATION_USE_CASES_PLAY_GAME_USE_CASE_HPP

#include <cstdint>
#include <memory>
#include "application/ports/renderer_port.hpp"
#include "application/ports/input_port.hpp"
#include "application/ports/timer_port.hpp"
#include "application/ports/storage_port.hpp"
#include "domain/entities/game.hpp"
#include "domain/services/replay_service.hpp"

namespace Snake {
namespace Application {
namespace UseCases {

/**
 * @brief Resultado de uma sessão de jogo
 */
struct GameSessionResult {
    uint32_t score;
    uint16_t snakeSize;
    uint32_t duration;  // em ms
    bool newHighscore;
    Domain::ReplayData replay;
};

/**
 * @brief Callback para eventos do use case
 */
struct PlayGameCallbacks {
    void (*onGameStart)(void* ctx) = nullptr;
    void (*onGameOver)(const GameSessionResult& result, void* ctx) = nullptr;
    void (*onScoreUpdate)(uint32_t score, void* ctx) = nullptr;
    void* context = nullptr;
};

/**
 * @brief Use Case: Jogar uma partida
 * 
 * Orquestra:
 * - Game (domínio)
 * - Renderer (output)
 * - Input (input)
 * - Timer (timing)
 * - ReplayRecorder (anti-cheat)
 */
class PlayGameUseCase {
public:
    PlayGameUseCase(Ports::IRenderer& renderer,
                    Ports::IInput& input,
                    Ports::ITimer& timer,
                    Ports::IStorage& storage)
        : renderer_(renderer), input_(input), timer_(timer), storage_(storage),
          highscore_(0), callbacks_{} {}
    
    void setCallbacks(const PlayGameCallbacks& callbacks) {
        callbacks_ = callbacks;
    }
    
    /**
     * @brief Executa uma sessão de jogo
     * @param config Configuração do jogo
     * @param seed Seed para o PRNG (usar timer_.currentTimeMs() para aleatoriedade)
     * @return Resultado da sessão
     */
    GameSessionResult execute(const Domain::GameConfig& config, uint32_t seed) {
        GameSessionResult result;
        result.newHighscore = false;
        
        // Carrega highscore
        highscore_ = storage_.loadHighscore();
        
        // Create game and recorder
        Domain::Game game(config);
        Domain::ReplayRecorder recorder;
        
        // Configura callback para gravar eventos
        game.setEventCallback(gameEventCallback, &recorder);
        
        // Inicia
        recorder.startRecording(seed, config);
        game.start(seed);
        
        if (callbacks_.onGameStart && callbacks_.context) {
            callbacks_.onGameStart(callbacks_.context);
        }
        
        uint32_t startTime = timer_.currentTimeMs();
        uint32_t lastFrameTime = startTime;
        uint16_t frameDelay = config.adjustedFrameDelayMs();
        
        // Game loop
        while (game.state() == Domain::GameState::Playing ||
               game.state() == Domain::GameState::Paused) {
            
            // Timing
            uint32_t currentTime = timer_.currentTimeMs();
            uint32_t elapsed = currentTime - lastFrameTime;
            
            if (elapsed < frameDelay) {
                timer_.delayMs(1);  // Yield CPU
                continue;
            }
            lastFrameTime = currentTime;
            
            // Input
            Ports::InputCommand cmd = input_.readCommand();
            if (cmd == Ports::InputCommand::Quit) {
                break;
            } else if (cmd == Ports::InputCommand::Pause) {
                game.togglePause();
            } else if (Ports::IInput::isMovementCommand(cmd)) {
                Domain::Direction dir = Ports::IInput::commandToDirection(cmd);
                game.handleInput(dir);
            }
            
            // Update (if not paused)
            if (game.state() == Domain::GameState::Playing) {
                if (!game.update()) {
                    // Game over
                    break;
                }
                
                // Notify score update
                if (callbacks_.onScoreUpdate && callbacks_.context) {
                    callbacks_.onScoreUpdate(game.score(), callbacks_.context);
                }
            }
            
            // Render
            renderer_.beginFrame();
            renderer_.clear();
            renderer_.drawBorder(config.boardWidth(), config.boardHeight());
            renderer_.drawSnake(game.snake());
            renderer_.drawFood(game.food());
            renderer_.drawScore(game.score(), highscore_);
            
            if (game.state() == Domain::GameState::Paused) {
                renderer_.drawPaused();
            }
            
            renderer_.endFrame();
        }
        
        // Fim do jogo
        uint32_t endTime = timer_.currentTimeMs();
        recorder.stopRecording(game.score(), game.frameCount());
        
        // Preenche resultado
        result.score = game.score();
        result.snakeSize = game.snake().size();
        result.duration = endTime - startTime;
        result.replay = recorder.getData();
        
        // Verifica highscore
        if (result.score > highscore_) {
            result.newHighscore = true;
            storage_.saveHighscore(result.score);
            highscore_ = result.score;
        }
        
        // Mostra game over
        renderer_.beginFrame();
        renderer_.clear();
        renderer_.drawGameOver(result.score, highscore_);
        renderer_.endFrame();
        
        // Callback
        if (callbacks_.onGameOver && callbacks_.context) {
            callbacks_.onGameOver(result, callbacks_.context);
        }
        
        // Aguarda input para continuar
        input_.waitForCommand();
        
        return result;
    }
    
    uint32_t getHighscore() const { return highscore_; }

private:
    static void gameEventCallback(const Domain::GameEvent& event, void* userData) {
        auto* recorder = static_cast<Domain::ReplayRecorder*>(userData);
        if (recorder) {
            recorder->recordEvent(event);
        }
    }
    
    Ports::IRenderer& renderer_;
    Ports::IInput& input_;
    Ports::ITimer& timer_;
    Ports::IStorage& storage_;
    uint32_t highscore_;
    PlayGameCallbacks callbacks_;
};

} // namespace UseCases
} // namespace Application
} // namespace Snake

#endif // APPLICATION_USE_CASES_PLAY_GAME_USE_CASE_HPP

/**
 * @file game.hpp
 * @brief Game Aggregate Root - Coordinates Snake + Food + Board
 * 
 * Clean Architecture: Domain Layer - Aggregate Root
 * - Entry point for domain operations
 * - Ensures invariant consistency
 * - Emits Domain Events
 */

#ifndef DOMAIN_ENTITIES_GAME_HPP
#define DOMAIN_ENTITIES_GAME_HPP

#include <cstdint>
#include "domain/entities/snake.hpp"
#include "domain/entities/food.hpp"
#include "domain/value_objects/game_config.hpp"
#include "domain/services/random_service.hpp"

namespace Snake {
namespace Domain {

/**
 * @brief Estados do jogo
 */
enum class GameState : uint8_t {
    NotStarted,
    Playing,
    Paused,
    GameOver
};

/**
 * @brief Eventos de domínio (para observers/replay)
 */
enum class GameEventType : uint8_t {
    GameStarted,
    SnakeMoved,
    FoodEaten,
    FoodSpawned,
    DirectionChanged,
    GamePaused,
    GameResumed,
    GameOver
};

struct GameEvent {
    GameEventType type;
    uint32_t frame;
    Point position;       // Usado para FoodSpawned, SnakeMoved
    Direction direction;  // Usado para DirectionChanged
    uint32_t score;       // Usado para FoodEaten
};

/**
 * @brief Callback para eventos (opcional)
 */
using GameEventCallback = void(*)(const GameEvent&, void* userData);

/**
 * @brief Game Aggregate Root
 */
class Game {
public:
    explicit Game(const GameConfig& config)
        : config_(config), state_(GameState::NotStarted),
          score_(0), frameCount_(0), random_(0),
          eventCallback_(nullptr), eventUserData_(nullptr) {}
    
    /**
     * @brief Inicia o jogo com uma seed
     */
    void start(uint32_t seed) {
        random_ = RandomService(seed);
        score_ = 0;
        frameCount_ = 0;
        
        // Initialize snake at center
        Point startPos(config_.boardWidth() / 2, config_.boardHeight() / 2);
        snake_.initialize(startPos, config_.initialSnakeSize(), Direction::Right);
        
        // Spawn first food
        spawnFood();
        
        state_ = GameState::Playing;
        emitEvent(GameEventType::GameStarted);
    }
    
    /**
     * @brief Atualiza o jogo (um frame)
     * @return false se game over
     */
    bool update() {
        if (state_ != GameState::Playing) return false;
        
        ++frameCount_;
        
        // Check if will eat
        bool willEat = snake_.head().moved(
            DirectionUtils::toDelta(snake_.direction()).x(),
            DirectionUtils::toDelta(snake_.direction()).y()
        ) == food_.position();
        
        // Move the snake
        Point newHead = snake_.move(willEat);
        emitEvent(GameEventType::SnakeMoved, newHead);
        
        // Check wall collision
        if (!newHead.isWithinBounds(config_.boardWidth(), config_.boardHeight())) {
            state_ = GameState::GameOver;
            emitEvent(GameEventType::GameOver);
            return false;
        }
        
        // Check self-collision
        if (snake_.hasSelfCollision()) {
            state_ = GameState::GameOver;
            emitEvent(GameEventType::GameOver);
            return false;
        }
        
        // Ate the food?
        if (willEat) {
            score_ += 10 * static_cast<uint32_t>(config_.difficulty());
            food_.consume();
            
            GameEvent evt;
            evt.type = GameEventType::FoodEaten;
            evt.frame = frameCount_;
            evt.score = score_;
            if (eventCallback_) eventCallback_(evt, eventUserData_);
            
            // Spawn new food
            spawnFood();
        }
        
        return true;
    }
    
    /**
     * @brief Processa input de direção
     */
    bool handleInput(Direction dir) {
        if (state_ != GameState::Playing) return false;
        if (snake_.setDirection(dir)) {
            emitEvent(GameEventType::DirectionChanged, Point(), dir);
            return true;
        }
        return false;
    }
    
    /**
     * @brief Pausa/continua o jogo
     */
    void togglePause() {
        if (state_ == GameState::Playing) {
            state_ = GameState::Paused;
            emitEvent(GameEventType::GamePaused);
        } else if (state_ == GameState::Paused) {
            state_ = GameState::Playing;
            emitEvent(GameEventType::GameResumed);
        }
    }
    
    // Getters
    GameState state() const { return state_; }
    uint32_t score() const { return score_; }
    uint32_t frameCount() const { return frameCount_; }
    const SnakeEntity& snake() const { return snake_; }
    const FoodEntity& food() const { return food_; }
    const GameConfig& config() const { return config_; }
    
    // Event callback
    void setEventCallback(GameEventCallback cb, void* userData = nullptr) {
        eventCallback_ = cb;
        eventUserData_ = userData;
    }

private:
    void spawnFood() {
        Point newPos;
        int attempts = 0;
        const int maxAttempts = 100;
        
        do {
            int16_t x = random_.nextInt(config_.boardWidth());
            int16_t y = random_.nextInt(config_.boardHeight());
            newPos = Point(x, y);
            ++attempts;
        } while (snake_.occupies(newPos) && attempts < maxAttempts);
        
        food_.spawn(newPos);
        emitEvent(GameEventType::FoodSpawned, newPos);
    }
    
    void emitEvent(GameEventType type, Point pos = Point(), Direction dir = Direction::None) {
        if (!eventCallback_) return;
        
        GameEvent evt;
        evt.type = type;
        evt.frame = frameCount_;
        evt.position = pos;
        evt.direction = dir;
        evt.score = score_;
        eventCallback_(evt, eventUserData_);
    }
    
    GameConfig config_;
    GameState state_;
    SnakeEntity snake_;
    FoodEntity food_;
    uint32_t score_;
    uint32_t frameCount_;
    RandomService random_;
    
    GameEventCallback eventCallback_;
    void* eventUserData_;
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_ENTITIES_GAME_HPP

/**
 * @file game_config.hpp
 * @brief GameConfig Value Object - Configurações imutáveis do jogo
 * 
 * Clean Architecture: Domain Layer - Value Object
 */

#ifndef DOMAIN_VALUE_OBJECTS_GAME_CONFIG_HPP
#define DOMAIN_VALUE_OBJECTS_GAME_CONFIG_HPP

#include <cstdint>
#include <string>

namespace Snake {
namespace Domain {

// Forward declarations
class ReplayData;

/**
 * @brief Níveis de dificuldade
 */
enum class Difficulty : uint8_t {
    Easy = 1,
    Normal = 2,
    Hard = 3,
    Insane = 5
};

/**
 * @brief Configuração imutável do jogo
 */
class GameConfig {
public:
    // Valores default
    static constexpr uint8_t DEFAULT_BOARD_WIDTH = 40;
    static constexpr uint8_t DEFAULT_BOARD_HEIGHT = 20;
    static constexpr uint8_t DEFAULT_INITIAL_SNAKE_SIZE = 3;
    static constexpr uint16_t DEFAULT_FRAME_DELAY_MS = 80;
    
    // Builder pattern para construção
    class Builder {
    public:
        Builder& boardSize(uint8_t width, uint8_t height) {
            width_ = width;
            height_ = height;
            return *this;
        }
        
        Builder& difficulty(Difficulty diff) {
            difficulty_ = diff;
            return *this;
        }
        
        Builder& initialSnakeSize(uint8_t size) {
            initialSnakeSize_ = size;
            return *this;
        }
        
        Builder& frameDelayMs(uint16_t delay) {
            frameDelayMs_ = delay;
            return *this;
        }
        
        GameConfig build() const {
            return GameConfig(width_, height_, difficulty_, 
                            initialSnakeSize_, frameDelayMs_);
        }
        
    private:
        uint8_t width_ = DEFAULT_BOARD_WIDTH;
        uint8_t height_ = DEFAULT_BOARD_HEIGHT;
        Difficulty difficulty_ = Difficulty::Normal;
        uint8_t initialSnakeSize_ = DEFAULT_INITIAL_SNAKE_SIZE;
        uint16_t frameDelayMs_ = DEFAULT_FRAME_DELAY_MS;
    };
    
    // Factory methods
    static GameConfig defaultConfig() {
        return Builder().build();
    }
    
    static GameConfig withDifficulty(Difficulty diff) {
        return Builder().difficulty(diff).build();
    }
    
    // Getters
    uint8_t boardWidth() const { return boardWidth_; }
    uint8_t boardHeight() const { return boardHeight_; }
    Difficulty difficulty() const { return difficulty_; }
    uint8_t initialSnakeSize() const { return initialSnakeSize_; }
    uint16_t frameDelayMs() const { return frameDelayMs_; }
    
    // Delay ajustado pela dificuldade
    uint16_t adjustedFrameDelayMs() const {
        switch (difficulty_) {
            case Difficulty::Easy:   return frameDelayMs_ * 3 / 2;  // 50% mais lento
            case Difficulty::Normal: return frameDelayMs_;
            case Difficulty::Hard:   return frameDelayMs_ * 3 / 4;  // 25% mais rápido
            case Difficulty::Insane: return frameDelayMs_ / 2;      // 50% mais rápido
            default: return frameDelayMs_;
        }
    }
    
    // Convert difficulty to string
    std::string difficultyString() const {
        switch (difficulty_) {
            case Difficulty::Easy:   return "Easy";
            case Difficulty::Normal: return "Normal";
            case Difficulty::Hard:   return "Hard";
            case Difficulty::Insane: return "Insane";
            default: return "Unknown";
        }
    }
    
private:
    // Construtor privado para Builder
    GameConfig(uint8_t width, uint8_t height, Difficulty diff,
              uint8_t snakeSize, uint16_t delay)
        : boardWidth_(width), boardHeight_(height), difficulty_(diff),
          initialSnakeSize_(snakeSize), frameDelayMs_(delay) {}
    
    // Construtor padrão privado (para uso em ReplayData)
    GameConfig()
        : boardWidth_(DEFAULT_BOARD_WIDTH), boardHeight_(DEFAULT_BOARD_HEIGHT),
          difficulty_(Difficulty::Normal), initialSnakeSize_(DEFAULT_INITIAL_SNAKE_SIZE),
          frameDelayMs_(DEFAULT_FRAME_DELAY_MS) {}
    
    // ReplayData precisa criar GameConfig vazio
    friend class Snake::Domain::ReplayData;
    
    uint8_t boardWidth_;
    uint8_t boardHeight_;
    Difficulty difficulty_;
    uint8_t initialSnakeSize_;
    uint16_t frameDelayMs_;
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_VALUE_OBJECTS_GAME_CONFIG_HPP

#ifndef SNAKE_CORE_TYPES_H_
#define SNAKE_CORE_TYPES_H_

#include <cstdint>

namespace SnakeCore {

// Direction constants (platform-independent)
constexpr int8_t DIR_NONE  = 0;
constexpr int8_t DIR_UP    = 3;
constexpr int8_t DIR_DOWN  = 2;
constexpr int8_t DIR_LEFT  = 4;
constexpr int8_t DIR_RIGHT = 5;

// Game state
enum class GameState : uint8_t {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    LEADERBOARD
};

// Collision result
enum class CollisionType : uint8_t {
    NONE,
    WALL,
    SELF,
    FOOD
};

// Point structure (minimal, no dependencies)
struct Point {
    int16_t x;
    int16_t y;
    
    Point() : x(0), y(0) {}
    Point(int16_t px, int16_t py) : x(px), y(py) {}
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

// Game configuration
struct GameConfig {
    uint8_t boardWidth;
    uint8_t boardHeight;
    uint8_t difficulty;         // 1=Easy, 2=Normal, 3=Hard, 5=Insane
    uint16_t frameDelayMs;      // Delay between frames
    uint8_t initialSnakeSize;
    Point initialSnakePos;
    int8_t initialDirection;
    
    GameConfig() : 
        boardWidth(80), 
        boardHeight(24),
        difficulty(2),
        frameDelayMs(80),
        initialSnakeSize(3),
        initialSnakePos(5, 7),
        initialDirection(DIR_RIGHT) {}
};

// Score entry for leaderboard
struct ScoreEntry {
    char playerName[32];
    uint32_t score;
    uint16_t snakeSize;
    uint8_t difficulty;
    uint32_t timestamp;
};

// Input event (for replay system)
struct InputEvent {
    int8_t direction;
    uint32_t timestamp;
};

// Callback function types for platform abstraction
typedef void (*RenderCallback)(void* context);
typedef int8_t (*InputCallback)(void* context);
typedef uint32_t (*TimeCallback)(void* context);
typedef void (*NetworkCallback)(void* context, const uint8_t* data, uint16_t length);

} // namespace SnakeCore

#endif

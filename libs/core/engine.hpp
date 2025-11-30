#ifndef SNAKE_CORE_ENGINE_H_
#define SNAKE_CORE_ENGINE_H_

#include "types.hpp"
#include "snake.hpp"
#include "random.hpp"
#include "replay.hpp"

namespace SnakeCore {

// Platform abstraction interface
class IPlatform {
public:
    virtual ~IPlatform() {}
    
    // Timing
    virtual uint32_t getTimeMs() = 0;
    virtual void delay(uint16_t ms) = 0;
    
    // Input
    virtual int8_t getInput() = 0;  // Returns direction or DIR_NONE
    
    // Rendering
    virtual void clear() = 0;
    virtual void drawSnakeHead(int16_t x, int16_t y) = 0;
    virtual void drawSnakeBody(int16_t x, int16_t y) = 0;
    virtual void drawFood(int16_t x, int16_t y) = 0;
    virtual void drawWall(int16_t x, int16_t y) = 0;
    virtual void drawScore(uint32_t score, uint16_t size) = 0;
    virtual void drawGameOver(uint32_t score) = 0;
    virtual void refresh() = 0;
    
    // Board dimensions
    virtual uint8_t getBoardWidth() = 0;
    virtual uint8_t getBoardHeight() = 0;
};

// Core game engine - platform independent
class GameEngine {
private:
    IPlatform* platform;
    Snake snake;
    Random rng;
    ReplayRecorder recorder;
    
    Point foodPos;
    GameConfig config;
    GameState state;
    
    uint32_t score;
    uint32_t frameCount;
    uint32_t gameStartTime;
    uint32_t lastFrameTime;
    
    bool recordingEnabled;

    void spawnFood() {
        int16_t x, y;
        int maxAttempts = 100;
        
        do {
            rng.randomPoint(x, y, 2, config.boardHeight - 2, 1, config.boardWidth - 2);
            maxAttempts--;
        } while (snake.containsPoint(Point(x, y)) && maxAttempts > 0);
        
        foodPos.x = x;
        foodPos.y = y;
        
        if (recordingEnabled) {
            recorder.recordFoodSpawn(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
        }
    }
    
    CollisionType checkCollision(const Point& pos) {
        // Wall collision
        if (pos.x <= 0 || pos.x >= config.boardHeight - 1 ||
            pos.y <= 0 || pos.y >= config.boardWidth - 1) {
            return CollisionType::WALL;
        }
        
        // Self collision
        if (snake.collidesWithSelf(pos)) {
            return CollisionType::SELF;
        }
        
        // Food collision
        if (pos == foodPos) {
            return CollisionType::FOOD;
        }
        
        return CollisionType::NONE;
    }
    
    void renderGame() {
        platform->clear();
        
        // Draw walls
        for (int16_t y = 0; y < config.boardWidth; y++) {
            platform->drawWall(0, y);
            platform->drawWall(config.boardHeight - 1, y);
        }
        for (int16_t x = 1; x < config.boardHeight - 1; x++) {
            platform->drawWall(x, 0);
            platform->drawWall(x, config.boardWidth - 1);
        }
        
        // Draw snake
        for (uint16_t i = 0; i < snake.getSize(); i++) {
            Point seg = snake.getBodySegment(i);
            if (i == snake.getSize() - 1) {
                platform->drawSnakeHead(seg.x, seg.y);
            } else {
                platform->drawSnakeBody(seg.x, seg.y);
            }
        }
        
        // Draw food
        platform->drawFood(foodPos.x, foodPos.y);
        
        // Draw score
        platform->drawScore(score, snake.getSize());
        
        platform->refresh();
    }

public:
    GameEngine(IPlatform* plat) : 
        platform(plat), 
        state(GameState::MENU),
        score(0),
        frameCount(0),
        gameStartTime(0),
        lastFrameTime(0),
        recordingEnabled(true) {}
    
    void setConfig(const GameConfig& cfg) {
        config = cfg;
    }
    
    void enableRecording(bool enable) {
        recordingEnabled = enable;
    }
    
    void startGame(uint32_t seed = 0) {
        // Use provided seed or generate from time
        if (seed == 0) {
            seed = platform->getTimeMs();
        }
        rng.seed(seed);
        
        // Get board dimensions from platform
        config.boardWidth = platform->getBoardWidth();
        config.boardHeight = platform->getBoardHeight();
        
        // Initialize snake
        Point startPos(config.boardHeight / 2, config.boardWidth / 4);
        snake.init(startPos, config.initialSnakeSize, config.initialDirection);
        
        // Initialize game state
        score = 0;
        frameCount = 0;
        gameStartTime = platform->getTimeMs();
        lastFrameTime = gameStartTime;
        state = GameState::PLAYING;
        
        // Spawn first food
        spawnFood();
        
        // Start recording
        if (recordingEnabled) {
            recorder.startRecording(seed, config.difficulty, 
                                   config.boardWidth, config.boardHeight);
        }
        
        // Initial render
        renderGame();
    }
    
    // Main game loop tick - call this repeatedly
    // Returns true if game is still running
    bool tick() {
        if (state != GameState::PLAYING) {
            return false;
        }
        
        uint32_t currentTime = platform->getTimeMs();
        uint32_t elapsed = currentTime - lastFrameTime;
        
        if (elapsed < config.frameDelayMs) {
            return true;
        }
        
        lastFrameTime = currentTime;
        frameCount++;
        
        // Get input
        int8_t input = platform->getInput();
        if (input != DIR_NONE) {
            if (snake.setDirection(input)) {
                if (recordingEnabled) {
                    recorder.recordInput(input, currentTime - gameStartTime);
                }
            }
        }
        
        // Calculate next position
        Point nextHead = snake.calculateNextHead();
        CollisionType collision = checkCollision(nextHead);
        
        switch (collision) {
            case CollisionType::WALL:
            case CollisionType::SELF:
                // Game over
                state = GameState::GAME_OVER;
                
                if (recordingEnabled) {
                    recorder.stopRecording(score, snake.getSize(), 
                                          currentTime - gameStartTime);
                }
                
                platform->drawGameOver(score);
                return false;
                
            case CollisionType::FOOD:
                // Eat food and grow
                snake.grow();
                score += config.difficulty;
                spawnFood();
                break;
                
            case CollisionType::NONE:
                // Normal movement
                snake.move();
                break;
        }
        
        if (recordingEnabled) {
            recorder.advanceFrame();
        }
        
        renderGame();
        return true;
    }
    
    // Getters
    GameState getState() const { return state; }
    uint32_t getScore() const { return score; }
    uint16_t getSnakeSize() const { return snake.getSize(); }
    uint32_t getFrameCount() const { return frameCount; }
    
    const ReplayData& getReplayData() const {
        return recorder.getReplayData();
    }
    
    // For replay validation
    static bool validateReplay(const ReplayData& replay) {
        // This would be implemented on the server side
        // Here we provide a reference implementation
        
        Random rng;
        rng.seed(replay.getSeed());
        
        // Create a minimal snake simulation
        Snake snake;
        Point startPos(replay.getBoardHeight() / 2, replay.getBoardWidth() / 4);
        snake.init(startPos, 3, DIR_RIGHT);
        
        uint32_t score = 0;
        uint8_t difficulty = replay.getDifficulty();
        
        const auto& events = replay.getEvents();
        const auto& foodSpawns = replay.getFoodSpawns();
        
        size_t eventIdx = 0;
        size_t foodIdx = 0;
        
        // Simulate game
        for (uint32_t frame = 0; frame < replay.getTotalFrames(); frame++) {
            // Process inputs for this frame
            while (eventIdx < events.size() && 
                   events[eventIdx].frameNumber == frame) {
                snake.setDirection(events[eventIdx].direction);
                eventIdx++;
            }
            
            Point nextHead = snake.calculateNextHead();
            
            // Check food collision
            if (foodIdx < foodSpawns.size()) {
                Point foodPos(foodSpawns[foodIdx].x, foodSpawns[foodIdx].y);
                if (nextHead == foodPos) {
                    snake.grow();
                    score += difficulty;
                    foodIdx++;
                } else {
                    snake.move();
                }
            } else {
                snake.move();
            }
        }
        
        // Validate results
        return (score == replay.getFinalScore() && 
                snake.getSize() == replay.getFinalSize());
    }
};

} // namespace SnakeCore

#endif

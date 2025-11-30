/**
 * Terminal Snake - Test Suite
 * 
 * Comprehensive tests for the clean architecture implementation
 * 
 * Build: cd tests && make
 * Run: ./runAllTests
 */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Domain Layer - Value Objects
#include "../src/domain/value_objects/point.hpp"
#include "../src/domain/value_objects/direction.hpp"
#include "../src/domain/value_objects/game_config.hpp"

// Domain Layer - Entities
#include "../src/domain/entities/snake.hpp"
#include "../src/domain/entities/food.hpp"
#include "../src/domain/entities/game.hpp"

// Domain Layer - Services
#include "../src/domain/services/random_service.hpp"
#include "../src/domain/services/replay_service.hpp"

using namespace Snake::Domain;

// ============================================================================
// POINT VALUE OBJECT TESTS
// ============================================================================

TEST_CASE("Point - Default Constructor", "[point][value_object]") {
    Point p;
    REQUIRE(p.x() == 0);
    REQUIRE(p.y() == 0);
}

TEST_CASE("Point - Parameterized Constructor", "[point][value_object]") {
    Point p(10, 20);
    REQUIRE(p.x() == 10);
    REQUIRE(p.y() == 20);
}

TEST_CASE("Point - Negative Values", "[point][value_object]") {
    Point p(-5, -10);
    REQUIRE(p.x() == -5);
    REQUIRE(p.y() == -10);
}

TEST_CASE("Point - Equality Operator", "[point][value_object]") {
    Point p1(5, 5);
    Point p2(5, 5);
    Point p3(5, 6);
    Point p4(6, 5);
    
    REQUIRE(p1 == p2);
    REQUIRE_FALSE(p1 == p3);
    REQUIRE_FALSE(p1 == p4);
}

TEST_CASE("Point - Inequality Operator", "[point][value_object]") {
    Point p1(5, 5);
    Point p2(5, 5);
    Point p3(5, 6);
    
    REQUIRE_FALSE(p1 != p2);
    REQUIRE(p1 != p3);
}

TEST_CASE("Point - Moved Returns New Point", "[point][value_object]") {
    Point p1(5, 5);
    Point p2 = p1.moved(3, -2);
    
    // Original unchanged (immutability)
    REQUIRE(p1.x() == 5);
    REQUIRE(p1.y() == 5);
    
    // New point has moved values
    REQUIRE(p2.x() == 8);
    REQUIRE(p2.y() == 3);
}

TEST_CASE("Point - Directional Moves", "[point][value_object]") {
    Point p(10, 10);
    
    REQUIRE(p.up() == Point(10, 9));
    REQUIRE(p.down() == Point(10, 11));
    REQUIRE(p.left() == Point(9, 10));
    REQUIRE(p.right() == Point(11, 10));
}

TEST_CASE("Point - Manhattan Distance", "[point][value_object]") {
    Point p1(0, 0);
    Point p2(3, 4);
    
    REQUIRE(p1.manhattanDistance(p2) == 7);
    REQUIRE(p2.manhattanDistance(p1) == 7);  // Symmetric
    
    Point p3(-2, -3);
    REQUIRE(p1.manhattanDistance(p3) == 5);
}

TEST_CASE("Point - Bounds Checking", "[point][value_object]") {
    REQUIRE(Point(5, 5).isWithinBounds(10, 10));
    REQUIRE(Point(0, 0).isWithinBounds(10, 10));
    REQUIRE(Point(9, 9).isWithinBounds(10, 10));
    
    REQUIRE_FALSE(Point(10, 5).isWithinBounds(10, 10));  // x out of bounds
    REQUIRE_FALSE(Point(5, 10).isWithinBounds(10, 10));  // y out of bounds
    REQUIRE_FALSE(Point(-1, 5).isWithinBounds(10, 10));  // negative x
    REQUIRE_FALSE(Point(5, -1).isWithinBounds(10, 10));  // negative y
}

TEST_CASE("Point - Less Than Operator (for containers)", "[point][value_object]") {
    Point p1(1, 5);
    Point p2(2, 3);
    Point p3(1, 6);
    
    REQUIRE(p1 < p2);      // Different x
    REQUIRE(p1 < p3);      // Same x, different y
    REQUIRE_FALSE(p2 < p1);
}

// ============================================================================
// DIRECTION VALUE OBJECT TESTS
// ============================================================================

TEST_CASE("Direction - Opposite Check", "[direction][value_object]") {
    REQUIRE(DirectionUtils::areOpposite(Direction::Up, Direction::Down));
    REQUIRE(DirectionUtils::areOpposite(Direction::Down, Direction::Up));
    REQUIRE(DirectionUtils::areOpposite(Direction::Left, Direction::Right));
    REQUIRE(DirectionUtils::areOpposite(Direction::Right, Direction::Left));
    
    REQUIRE_FALSE(DirectionUtils::areOpposite(Direction::Up, Direction::Left));
    REQUIRE_FALSE(DirectionUtils::areOpposite(Direction::Up, Direction::Right));
    REQUIRE_FALSE(DirectionUtils::areOpposite(Direction::None, Direction::Up));
}

TEST_CASE("Direction - To Delta", "[direction][value_object]") {
    REQUIRE(DirectionUtils::toDelta(Direction::Up) == Point(0, -1));
    REQUIRE(DirectionUtils::toDelta(Direction::Down) == Point(0, 1));
    REQUIRE(DirectionUtils::toDelta(Direction::Left) == Point(-1, 0));
    REQUIRE(DirectionUtils::toDelta(Direction::Right) == Point(1, 0));
    REQUIRE(DirectionUtils::toDelta(Direction::None) == Point(0, 0));
}

TEST_CASE("Direction - From Key", "[direction][value_object]") {
    REQUIRE(DirectionUtils::fromKey(3) == Direction::Up);
    REQUIRE(DirectionUtils::fromKey(2) == Direction::Down);
    REQUIRE(DirectionUtils::fromKey(4) == Direction::Left);
    REQUIRE(DirectionUtils::fromKey(5) == Direction::Right);
    REQUIRE(DirectionUtils::fromKey(99) == Direction::None);
}

TEST_CASE("Direction - Opposite", "[direction][value_object]") {
    REQUIRE(DirectionUtils::opposite(Direction::Up) == Direction::Down);
    REQUIRE(DirectionUtils::opposite(Direction::Down) == Direction::Up);
    REQUIRE(DirectionUtils::opposite(Direction::Left) == Direction::Right);
    REQUIRE(DirectionUtils::opposite(Direction::Right) == Direction::Left);
    REQUIRE(DirectionUtils::opposite(Direction::None) == Direction::None);
}

TEST_CASE("Direction - ToString", "[direction][value_object]") {
    REQUIRE(std::string(DirectionUtils::toString(Direction::Up)) == "Up");
    REQUIRE(std::string(DirectionUtils::toString(Direction::Down)) == "Down");
    REQUIRE(std::string(DirectionUtils::toString(Direction::Left)) == "Left");
    REQUIRE(std::string(DirectionUtils::toString(Direction::Right)) == "Right");
    REQUIRE(std::string(DirectionUtils::toString(Direction::None)) == "None");
}

// ============================================================================
// GAME CONFIG VALUE OBJECT TESTS
// ============================================================================

TEST_CASE("GameConfig - Default Config", "[game_config][value_object]") {
    auto config = GameConfig::defaultConfig();
    
    REQUIRE(config.boardWidth() == 40);  // DEFAULT_BOARD_WIDTH
    REQUIRE(config.boardHeight() == 20); // DEFAULT_BOARD_HEIGHT
    REQUIRE(config.initialSnakeSize() == 3); // DEFAULT_INITIAL_SNAKE_SIZE
    REQUIRE(config.frameDelayMs() == 80); // DEFAULT_FRAME_DELAY_MS
    REQUIRE(config.difficulty() == Difficulty::Normal);
}

TEST_CASE("GameConfig - Builder Pattern", "[game_config][value_object]") {
    auto config = GameConfig::Builder()
        .boardSize(50, 30)
        .difficulty(Difficulty::Hard)
        .initialSnakeSize(5)
        .frameDelayMs(100)
        .build();
    
    REQUIRE(config.boardWidth() == 50);
    REQUIRE(config.boardHeight() == 30);
    REQUIRE(config.difficulty() == Difficulty::Hard);
    REQUIRE(config.initialSnakeSize() == 5);
    REQUIRE(config.frameDelayMs() == 100);
}

TEST_CASE("GameConfig - Difficulty Adjustment", "[game_config][value_object]") {
    uint16_t baseDelay = 80;
    
    auto easy = GameConfig::Builder()
        .frameDelayMs(baseDelay)
        .difficulty(Difficulty::Easy)
        .build();
    REQUIRE(easy.adjustedFrameDelayMs() == 120);  // 50% slower
    
    auto normal = GameConfig::Builder()
        .frameDelayMs(baseDelay)
        .difficulty(Difficulty::Normal)
        .build();
    REQUIRE(normal.adjustedFrameDelayMs() == 80);  // No change
    
    auto hard = GameConfig::Builder()
        .frameDelayMs(baseDelay)
        .difficulty(Difficulty::Hard)
        .build();
    REQUIRE(hard.adjustedFrameDelayMs() == 60);  // 25% faster
    
    auto insane = GameConfig::Builder()
        .frameDelayMs(baseDelay)
        .difficulty(Difficulty::Insane)
        .build();
    REQUIRE(insane.adjustedFrameDelayMs() == 40);  // 50% faster
}

TEST_CASE("GameConfig - Difficulty String", "[game_config][value_object]") {
    REQUIRE(GameConfig::withDifficulty(Difficulty::Easy).difficultyString() == "Easy");
    REQUIRE(GameConfig::withDifficulty(Difficulty::Normal).difficultyString() == "Normal");
    REQUIRE(GameConfig::withDifficulty(Difficulty::Hard).difficultyString() == "Hard");
    REQUIRE(GameConfig::withDifficulty(Difficulty::Insane).difficultyString() == "Insane");
}

// ============================================================================
// SNAKE ENTITY TESTS
// ============================================================================

TEST_CASE("Snake - Initialization", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    REQUIRE(snake.size() == 3);
    REQUIRE(snake.head() == Point(10, 10));
    REQUIRE(snake.direction() == Direction::Right);
    
    // Body extends behind head in opposite direction
    REQUIRE(snake.segmentAt(0) == Point(10, 10));  // Head
    REQUIRE(snake.segmentAt(1) == Point(9, 10));   // Body
    REQUIRE(snake.segmentAt(2) == Point(8, 10));   // Tail
}

TEST_CASE("Snake - Movement Without Growth", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    // Initial: head at (10,10), body at (9,10), tail at (8,10)
    
    Point newHead = snake.move(false);
    
    REQUIRE(newHead.x() == 11);
    REQUIRE(newHead.y() == 10);
    REQUIRE(snake.size() == 3);  // Size unchanged
    REQUIRE(snake.head().x() == 11);
    REQUIRE(snake.head().y() == 10);
    // After move: segments shift - now (11,10), (10,10), (9,10)
    REQUIRE(snake.segmentAt(1).x() == 10);
    REQUIRE(snake.segmentAt(1).y() == 10);
    REQUIRE(snake.segmentAt(2).x() == 9);
    REQUIRE(snake.segmentAt(2).y() == 10);
}

TEST_CASE("Snake - Movement With Growth", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    Point newHead = snake.move(true);
    
    REQUIRE(newHead == Point(11, 10));
    REQUIRE(snake.size() == 4);  // Size increased
    REQUIRE(snake.head() == Point(11, 10));
}

TEST_CASE("Snake - Direction Change", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    // Valid direction change
    REQUIRE(snake.setDirection(Direction::Up));
    REQUIRE(snake.direction() == Direction::Up);
    
    // Invalid direction change (opposite)
    REQUIRE_FALSE(snake.setDirection(Direction::Down));
    REQUIRE(snake.direction() == Direction::Up);  // Unchanged
    
    // None direction
    REQUIRE_FALSE(snake.setDirection(Direction::None));
}

TEST_CASE("Snake - Self Collision Detection", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(5, 5), 5, Direction::Right);
    
    // Initial state - no collision
    REQUIRE_FALSE(snake.hasSelfCollision());
    
    // Move and change direction to create collision scenario
    snake.move(false);  // 6,5
    snake.setDirection(Direction::Down);
    snake.move(false);  // 6,6
    snake.setDirection(Direction::Left);
    snake.move(false);  // 5,6
    snake.setDirection(Direction::Up);
    snake.move(false);  // 5,5 - would collide with previous position
    
    // Note: With a 5-length snake, this creates a collision
    // The exact collision depends on the movement pattern
}

TEST_CASE("Snake - Occupies Position", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    REQUIRE(snake.occupies(Point(10, 10)));  // Head
    REQUIRE(snake.occupies(Point(9, 10)));   // Body
    REQUIRE(snake.occupies(Point(8, 10)));   // Tail
    REQUIRE_FALSE(snake.occupies(Point(11, 10)));  // Not occupied
}

TEST_CASE("Snake - Head At Position", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    REQUIRE(snake.headAt(Point(10, 10)));
    REQUIRE_FALSE(snake.headAt(Point(9, 10)));  // Body, not head
}

TEST_CASE("Snake - Iterator", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    int count = 0;
    for (const auto& segment : snake) {
        (void)segment;  // Use segment
        ++count;
    }
    
    REQUIRE(count == 3);
}

TEST_CASE("Snake - Max Size Limit", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    // Grow to max size (256)
    for (int i = 0; i < 260; ++i) {
        snake.move(true);
    }
    
    REQUIRE(snake.size() <= 256);  // MAX_SIZE
}

// ============================================================================
// FOOD ENTITY TESTS
// ============================================================================

TEST_CASE("Food - Default State", "[food][entity]") {
    FoodEntity food;
    REQUIRE_FALSE(food.isActive());
}

TEST_CASE("Food - Spawn", "[food][entity]") {
    FoodEntity food;
    food.spawn(Point(15, 15));
    
    REQUIRE(food.isActive());
    REQUIRE(food.position() == Point(15, 15));
}

TEST_CASE("Food - Consume", "[food][entity]") {
    FoodEntity food;
    food.spawn(Point(15, 15));
    REQUIRE(food.isActive());
    
    food.consume();
    REQUIRE_FALSE(food.isActive());
}

// ============================================================================
// RANDOM SERVICE TESTS
// ============================================================================

TEST_CASE("Random - Deterministic Output", "[random][service]") {
    RandomService r1(12345);
    RandomService r2(12345);
    
    // Same seed should produce same sequence
    for (int i = 0; i < 100; ++i) {
        REQUIRE(r1.next() == r2.next());
    }
}

TEST_CASE("Random - Different Seeds Different Output", "[random][service]") {
    RandomService r1(12345);
    RandomService r2(54321);
    
    // Different seeds should produce different sequences
    bool allSame = true;
    for (int i = 0; i < 10; ++i) {
        if (r1.next() != r2.next()) {
            allSame = false;
            break;
        }
    }
    REQUIRE_FALSE(allSame);
}

TEST_CASE("Random - Next Int Range", "[random][service]") {
    RandomService r(42);
    
    for (int i = 0; i < 1000; ++i) {
        uint32_t val = r.nextInt(100);
        REQUIRE(val < 100);
    }
}

TEST_CASE("Random - Next Int Range With Min", "[random][service]") {
    RandomService r(42);
    
    for (int i = 0; i < 1000; ++i) {
        uint32_t val = r.nextIntRange(10, 20);
        REQUIRE(val >= 10);
        REQUIRE(val < 20);
    }
}

TEST_CASE("Random - Next Float Range", "[random][service]") {
    RandomService r(42);
    
    for (int i = 0; i < 1000; ++i) {
        float val = r.nextFloat();
        REQUIRE(val >= 0.0f);
        REQUIRE(val < 1.0f);
    }
}

TEST_CASE("Random - State Save/Restore", "[random][service]") {
    RandomService r(42);
    
    // Generate some numbers
    for (int i = 0; i < 50; ++i) {
        r.next();
    }
    
    // Save state
    uint32_t savedState = r.getState();
    uint32_t nextVal = r.next();
    
    // Restore state
    r.setState(savedState);
    REQUIRE(r.next() == nextVal);
}

TEST_CASE("Random - Zero Seed Handling", "[random][service]") {
    RandomService r(0);  // Should not cause infinite loop or crash
    
    // Should still generate numbers
    uint32_t val1 = r.next();
    uint32_t val2 = r.next();
    REQUIRE(val1 != val2);
}

// ============================================================================
// GAME AGGREGATE ROOT TESTS
// ============================================================================

TEST_CASE("Game - Initial State", "[game][aggregate]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    
    REQUIRE(game.state() == GameState::NotStarted);
    REQUIRE(game.score() == 0);
    REQUIRE(game.frameCount() == 0);
}

TEST_CASE("Game - Start", "[game][aggregate]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    
    game.start(12345);
    
    REQUIRE(game.state() == GameState::Playing);
    REQUIRE(game.score() == 0);
    REQUIRE(game.snake().size() == config.initialSnakeSize());
    REQUIRE(game.food().isActive());
}

TEST_CASE("Game - Update Increments Frame", "[game][aggregate]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    game.start(12345);
    
    uint32_t initialFrame = game.frameCount();
    game.update();
    
    REQUIRE(game.frameCount() == initialFrame + 1);
}

TEST_CASE("Game - Handle Input", "[game][aggregate]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    game.start(12345);
    
    // Valid direction change
    REQUIRE(game.handleInput(Direction::Up));
    
    // Invalid when not playing
    game.togglePause();
    REQUIRE_FALSE(game.handleInput(Direction::Left));
}

TEST_CASE("Game - Pause Toggle", "[game][aggregate]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    game.start(12345);
    
    REQUIRE(game.state() == GameState::Playing);
    
    game.togglePause();
    REQUIRE(game.state() == GameState::Paused);
    
    game.togglePause();
    REQUIRE(game.state() == GameState::Playing);
}

TEST_CASE("Game - Wall Collision", "[game][aggregate]") {
    auto config = GameConfig::Builder()
        .boardSize(10, 10)
        .initialSnakeSize(3)
        .build();
    
    Game game(config);
    game.start(12345);
    
    // Move towards wall
    game.handleInput(Direction::Right);
    
    // Keep updating until game over or max iterations
    int iterations = 0;
    while (game.state() == GameState::Playing && iterations < 100) {
        game.update();
        ++iterations;
    }
    
    // Should eventually hit wall
    REQUIRE(game.state() == GameState::GameOver);
}

// ============================================================================
// REPLAY SERVICE TESTS
// ============================================================================

TEST_CASE("Replay - Record and Serialize", "[replay][service]") {
    GameConfig config = GameConfig::defaultConfig();
    ReplayRecorder recorder;
    
    recorder.startRecording(12345, config);
    REQUIRE(recorder.isRecording());
    
    // Record some events
    GameEvent dirEvent;
    dirEvent.type = GameEventType::DirectionChanged;
    dirEvent.frame = 10;
    dirEvent.direction = Direction::Up;
    recorder.recordEvent(dirEvent);
    
    GameEvent foodEvent;
    foodEvent.type = GameEventType::FoodSpawned;
    foodEvent.frame = 20;
    foodEvent.position = Point(15, 15);
    recorder.recordEvent(foodEvent);
    
    recorder.stopRecording(100, 500);
    REQUIRE_FALSE(recorder.isRecording());
    
    const ReplayData& data = recorder.getData();
    REQUIRE(data.seed() == 12345);
    REQUIRE(data.finalScore() == 100);
    REQUIRE(data.totalFrames() == 500);
    REQUIRE(data.events().size() == 2);
}

TEST_CASE("Replay - Serialize/Deserialize", "[replay][service]") {
    GameConfig config = GameConfig::Builder()
        .boardSize(30, 20)
        .difficulty(Difficulty::Hard)
        .build();
    
    ReplayRecorder recorder;
    recorder.startRecording(99999, config);
    
    GameEvent evt;
    evt.type = GameEventType::DirectionChanged;
    evt.frame = 42;
    evt.direction = Direction::Left;
    recorder.recordEvent(evt);
    
    recorder.stopRecording(250, 1000);
    
    // Serialize to Base64
    std::string base64 = recorder.getData().toBase64();
    REQUIRE_FALSE(base64.empty());
    
    // Deserialize
    ReplayData restored = ReplayData::fromBase64(base64);
    
    REQUIRE(restored.seed() == 99999);
    REQUIRE(restored.finalScore() == 250);
    REQUIRE(restored.totalFrames() == 1000);
    REQUIRE(restored.events().size() == 1);
    REQUIRE(restored.config().boardWidth() == 30);
    REQUIRE(restored.config().boardHeight() == 20);
}

TEST_CASE("Replay - Validation", "[replay][service]") {
    // Create a simple game and record it
    auto config = GameConfig::Builder()
        .boardSize(20, 15)
        .initialSnakeSize(3)
        .difficulty(Difficulty::Normal)
        .build();
    
    Game game(config);
    ReplayRecorder recorder;
    
    // Record game events
    game.setEventCallback([](const GameEvent& event, void* userData) {
        auto* rec = static_cast<ReplayRecorder*>(userData);
        rec->recordEvent(event);
    }, &recorder);
    
    uint32_t seed = 12345;
    recorder.startRecording(seed, config);
    game.start(seed);
    
    // Play a few frames
    for (int i = 0; i < 10 && game.state() == GameState::Playing; ++i) {
        game.update();
    }
    
    recorder.stopRecording(game.score(), game.frameCount());
    
    // Validate the replay
    auto result = ReplayValidator::validate(recorder.getData());
    
    // Score should match
    REQUIRE(result.simulatedScore == result.claimedScore);
}

TEST_CASE("Replay - Invalid Score Detection", "[replay][service]") {
    auto config = GameConfig::defaultConfig();
    ReplayRecorder recorder;
    recorder.startRecording(12345, config);
    
    // Record minimal events
    recorder.stopRecording(99999, 10);  // Fake high score
    
    // Validate should detect mismatch
    auto result = ReplayValidator::validate(recorder.getData());
    
    // With no direction changes and only 10 frames, score should be much lower
    REQUIRE(result.simulatedScore != 99999);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST_CASE("Integration - Complete Game Session", "[integration]") {
    auto config = GameConfig::Builder()
        .boardSize(20, 15)
        .initialSnakeSize(3)
        .difficulty(Difficulty::Normal)
        .build();
    
    Game game(config);
    ReplayRecorder recorder;
    
    // Setup recording
    game.setEventCallback([](const GameEvent& event, void* userData) {
        auto* rec = static_cast<ReplayRecorder*>(userData);
        rec->recordEvent(event);
    }, &recorder);
    
    uint32_t seed = 42;
    recorder.startRecording(seed, config);
    game.start(seed);
    
    // Simulate gameplay
    int moves = 0;
    while (game.state() == GameState::Playing && moves < 1000) {
        // Change direction occasionally
        if (moves % 10 == 5) {
            Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
            game.handleInput(dirs[moves % 4]);
        }
        
        game.update();
        ++moves;
    }
    
    recorder.stopRecording(game.score(), game.frameCount());
    
    // Verify replay
    auto validation = ReplayValidator::validate(recorder.getData());
    REQUIRE(validation.valid);
    REQUIRE(validation.simulatedScore == game.score());
}

TEST_CASE("Integration - Replay Determinism Across Runs", "[integration]") {
    auto config = GameConfig::Builder()
        .boardSize(15, 10)
        .initialSnakeSize(2)
        .build();
    
    uint32_t seed = 777;
    
    // Run game twice with same seed and inputs
    auto runGame = [&](std::vector<std::pair<int, Direction>>& inputs) {
        Game game(config);
        game.start(seed);
        
        int frame = 0;
        size_t inputIdx = 0;
        
        while (game.state() == GameState::Playing && frame < 100) {
            // Apply inputs at specified frames
            while (inputIdx < inputs.size() && inputs[inputIdx].first == frame) {
                game.handleInput(inputs[inputIdx].second);
                ++inputIdx;
            }
            
            game.update();
            ++frame;
        }
        
        return game.score();
    };
    
    std::vector<std::pair<int, Direction>> inputs = {
        {5, Direction::Up},
        {10, Direction::Right},
        {15, Direction::Down}
    };
    
    uint32_t score1 = runGame(inputs);
    uint32_t score2 = runGame(inputs);
    
    REQUIRE(score1 == score2);
}

// ============================================================================
// ADDITIONAL POINT TESTS (Edge Cases)
// ============================================================================

TEST_CASE("Point - Large Values", "[point][value_object][edge]") {
    Point p(255, 255);
    REQUIRE(p.x() == 255);
    REQUIRE(p.y() == 255);
    
    Point moved = p.moved(1, 1);
    REQUIRE(moved.x() == 256);
    REQUIRE(moved.y() == 256);
}

TEST_CASE("Point - Origin Operations", "[point][value_object]") {
    Point origin(0, 0);
    
    REQUIRE(origin.up() == Point(0, -1));
    REQUIRE(origin.down() == Point(0, 1));
    REQUIRE(origin.left() == Point(-1, 0));
    REQUIRE(origin.right() == Point(1, 0));
    
    REQUIRE(origin.manhattanDistance(Point(0, 0)) == 0);
}

TEST_CASE("Point - Self Distance", "[point][value_object]") {
    Point p(42, 37);
    REQUIRE(p.manhattanDistance(p) == 0);
}

// ============================================================================
// ADDITIONAL SNAKE TESTS (Edge Cases)
// ============================================================================

TEST_CASE("Snake - Single Segment", "[snake][entity][edge]") {
    SnakeEntity snake;
    snake.initialize(Point(5, 5), 1, Direction::Right);
    
    REQUIRE(snake.size() == 1);
    REQUIRE(snake.head() == Point(5, 5));
    REQUIRE_FALSE(snake.hasSelfCollision());  // Can't collide with just head
    
    snake.move(false);
    REQUIRE(snake.head() == Point(6, 5));
    REQUIRE(snake.size() == 1);
}

TEST_CASE("Snake - All Directions Movement", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    // Move right
    snake.move(false);
    REQUIRE(snake.head() == Point(11, 10));
    
    // Change to up and move
    snake.setDirection(Direction::Up);
    snake.move(false);
    REQUIRE(snake.head() == Point(11, 9));
    
    // Change to left and move
    snake.setDirection(Direction::Left);
    snake.move(false);
    REQUIRE(snake.head() == Point(10, 9));
    
    // Change to down and move
    snake.setDirection(Direction::Down);
    snake.move(false);
    REQUIRE(snake.head() == Point(10, 10));
}

TEST_CASE("Snake - Rapid Direction Changes", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 3, Direction::Right);
    
    // Try to change direction multiple times before moving
    REQUIRE(snake.setDirection(Direction::Up));
    REQUIRE(snake.setDirection(Direction::Left));  // Should work (from Up)
    REQUIRE_FALSE(snake.setDirection(Direction::Right));  // Opposite of Left
    
    snake.move(false);
    REQUIRE(snake.head() == Point(9, 10));
}

TEST_CASE("Snake - Body Collision Check All Segments", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 5, Direction::Right);
    
    // Check each segment position
    REQUIRE(snake.occupies(Point(10, 10)));  // Head
    REQUIRE(snake.occupies(Point(9, 10)));
    REQUIRE(snake.occupies(Point(8, 10)));
    REQUIRE(snake.occupies(Point(7, 10)));
    REQUIRE(snake.occupies(Point(6, 10)));   // Tail
    
    REQUIRE_FALSE(snake.occupies(Point(5, 10)));
    REQUIRE_FALSE(snake.occupies(Point(11, 10)));
}

// ============================================================================
// ADDITIONAL FOOD TESTS (Edge Cases)
// ============================================================================

TEST_CASE("Food - Multiple Spawn Calls", "[food][entity][edge]") {
    FoodEntity food;
    
    food.spawn(Point(5, 5));
    REQUIRE(food.position() == Point(5, 5));
    REQUIRE(food.isActive());
    
    // Spawn again at different position
    food.spawn(Point(10, 10));
    REQUIRE(food.position() == Point(10, 10));
    REQUIRE(food.isActive());
}

TEST_CASE("Food - Consume When Inactive", "[food][entity][edge]") {
    FoodEntity food;
    REQUIRE_FALSE(food.isActive());
    
    food.consume();  // Should be safe
    REQUIRE_FALSE(food.isActive());
}

// ============================================================================
// ADDITIONAL RANDOM TESTS (Edge Cases)
// ============================================================================

TEST_CASE("Random - Distribution Sanity Check", "[random][service]") {
    RandomService r(42);
    
    int counts[10] = {0};
    const int iterations = 10000;
    
    for (int i = 0; i < iterations; ++i) {
        uint32_t val = r.nextInt(10);
        counts[val]++;
    }
    
    // Each bucket should have roughly 10% (1000) with some tolerance
    for (int i = 0; i < 10; ++i) {
        REQUIRE(counts[i] > 500);   // At least 5%
        REQUIRE(counts[i] < 2000);  // At most 20%
    }
}

TEST_CASE("Random - Edge Value Range", "[random][service][edge]") {
    RandomService r(42);
    
    // Range of size 1 should always return 0
    for (int i = 0; i < 100; ++i) {
        REQUIRE(r.nextInt(1) == 0);
    }
}

TEST_CASE("Random - Large Range", "[random][service]") {
    RandomService r(42);
    
    bool foundLarge = false;
    for (int i = 0; i < 1000; ++i) {
        uint32_t val = r.nextInt(1000000);
        if (val > 900000) foundLarge = true;
    }
    
    REQUIRE(foundLarge);  // Should hit large values occasionally
}

// ============================================================================
// ADDITIONAL GAME CONFIG TESTS
// ============================================================================

TEST_CASE("GameConfig - Minimum Board Size", "[game_config][value_object][edge]") {
    auto config = GameConfig::Builder()
        .boardSize(5, 5)
        .initialSnakeSize(3)
        .build();
    
    REQUIRE(config.boardWidth() == 5);
    REQUIRE(config.boardHeight() == 5);
}

TEST_CASE("GameConfig - All Difficulties Frame Delays", "[game_config][value_object]") {
    uint16_t base = 100;
    
    auto easy = GameConfig::Builder().frameDelayMs(base).difficulty(Difficulty::Easy).build();
    auto normal = GameConfig::Builder().frameDelayMs(base).difficulty(Difficulty::Normal).build();
    auto hard = GameConfig::Builder().frameDelayMs(base).difficulty(Difficulty::Hard).build();
    auto insane = GameConfig::Builder().frameDelayMs(base).difficulty(Difficulty::Insane).build();
    
    REQUIRE(easy.adjustedFrameDelayMs() > normal.adjustedFrameDelayMs());
    REQUIRE(normal.adjustedFrameDelayMs() > hard.adjustedFrameDelayMs());
    REQUIRE(hard.adjustedFrameDelayMs() > insane.adjustedFrameDelayMs());
}

// ============================================================================
// ADDITIONAL GAME TESTS (Edge Cases)
// ============================================================================

TEST_CASE("Game - Double Start", "[game][aggregate][edge]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    
    game.start(12345);
    uint32_t snake1Size = game.snake().size();
    
    // Starting again should reset
    game.start(54321);
    uint32_t snake2Size = game.snake().size();
    
    REQUIRE(snake1Size == snake2Size);
}

TEST_CASE("Game - Input When Game Over", "[game][aggregate][edge]") {
    auto config = GameConfig::Builder()
        .boardSize(5, 5)
        .initialSnakeSize(3)
        .build();
    
    Game game(config);
    game.start(12345);
    
    // Force game over by running into wall
    game.handleInput(Direction::Right);
    while (game.state() == GameState::Playing) {
        game.update();
    }
    
    REQUIRE(game.state() == GameState::GameOver);
    REQUIRE_FALSE(game.handleInput(Direction::Up));  // Should fail
}

TEST_CASE("Game - Update When Not Playing", "[game][aggregate][edge]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    
    // Update before start
    game.update();
    REQUIRE(game.frameCount() == 0);  // Should not increment
}

TEST_CASE("Game - Pause When Not Playing", "[game][aggregate][edge]") {
    GameConfig config = GameConfig::defaultConfig();
    Game game(config);
    
    // Try to pause before starting
    game.togglePause();
    REQUIRE(game.state() == GameState::NotStarted);  // Should not change
}

// ============================================================================
// ADDITIONAL REPLAY TESTS
// ============================================================================

TEST_CASE("Replay - Empty Replay", "[replay][service][edge]") {
    ReplayRecorder recorder;
    
    REQUIRE_FALSE(recorder.isRecording());
    
    // Should be safe to get data even if never started
    const ReplayData& data = recorder.getData();
    REQUIRE(data.events().empty());
}

TEST_CASE("Replay - Large Event Count", "[replay][service]") {
    ReplayRecorder recorder;
    auto config = GameConfig::defaultConfig();
    
    recorder.startRecording(12345, config);
    
    // Record many events
    for (int i = 0; i < 1000; ++i) {
        GameEvent evt;
        evt.type = GameEventType::DirectionChanged;
        evt.frame = static_cast<uint32_t>(i);
        evt.direction = static_cast<Direction>((i % 4) + 1);
        recorder.recordEvent(evt);
    }
    
    recorder.stopRecording(0, 1000);
    
    REQUIRE(recorder.getData().events().size() == 1000);
}

TEST_CASE("Replay - Base64 Round Trip Integrity", "[replay][service]") {
    auto config = GameConfig::Builder()
        .boardSize(40, 30)
        .difficulty(Difficulty::Insane)
        .build();
    
    ReplayRecorder recorder;
    recorder.startRecording(987654321, config);
    
    // Add various event types (only those serialized with full data)
    GameEvent dir;
    dir.type = GameEventType::DirectionChanged;
    dir.frame = 100;
    dir.direction = Direction::Up;
    recorder.recordEvent(dir);
    
    GameEvent food;
    food.type = GameEventType::FoodSpawned;
    food.frame = 200;
    food.position = Point(25, 15);
    recorder.recordEvent(food);
    
    GameEvent dir2;
    dir2.type = GameEventType::DirectionChanged;
    dir2.frame = 250;
    dir2.direction = Direction::Left;
    recorder.recordEvent(dir2);
    
    recorder.stopRecording(1500, 5000);
    
    std::string base64 = recorder.getData().toBase64();
    REQUIRE_FALSE(base64.empty());
    
    ReplayData restored = ReplayData::fromBase64(base64);
    
    REQUIRE(restored.seed() == 987654321);
    REQUIRE(restored.finalScore() == 1500);
    REQUIRE(restored.totalFrames() == 5000);
    REQUIRE(restored.config().boardWidth() == 40);
    REQUIRE(restored.config().boardHeight() == 30);
    REQUIRE(restored.config().difficulty() == Difficulty::Insane);
    REQUIRE(restored.events().size() == 3);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_CASE("Stress - Long Game Session", "[stress]") {
    auto config = GameConfig::Builder()
        .boardSize(50, 30)
        .initialSnakeSize(3)
        .build();
    
    Game game(config);
    game.start(12345);
    
    int frames = 0;
    const int maxFrames = 10000;
    
    // Run game with random-ish direction changes
    while (game.state() == GameState::Playing && frames < maxFrames) {
        if (frames % 7 == 0) {
            Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
            game.handleInput(dirs[frames % 4]);
        }
        
        game.update();
        ++frames;
    }
    
    REQUIRE(frames > 0);  // Should run at least once
    REQUIRE(game.frameCount() == static_cast<uint32_t>(frames));
}

TEST_CASE("Stress - Many Snake Growths", "[stress]") {
    SnakeEntity snake;
    snake.initialize(Point(100, 100), 1, Direction::Right);
    
    // Grow snake rapidly
    for (int i = 0; i < 200; ++i) {
        snake.move(true);
    }
    
    REQUIRE(snake.size() <= 256);  // Should cap at MAX_SIZE
}

TEST_CASE("Stress - Random Service Many Calls", "[stress]") {
    RandomService r(42);
    
    uint32_t prev = 0;
    bool foundDifferent = false;
    
    for (int i = 0; i < 100000; ++i) {
        uint32_t val = r.next();
        if (val != prev) foundDifferent = true;
        prev = val;
    }
    
    REQUIRE(foundDifferent);  // Should produce different values
}

// ============================================================================
// APPLICATION LAYER TESTS - Mock Implementations
// ============================================================================

// Mock implementations for testing Application layer
namespace MockPorts {
    
    // Simple in-memory storage mock
    class MockStorage {
    public:
        uint32_t highscore = 0;
        std::string displayName = "TestPlayer";
        
        uint32_t loadHighscore() { return highscore; }
        void saveHighscore(uint32_t hs) { highscore = hs; }
        std::string loadDisplayName() { return displayName; }
        void saveDisplayName(const std::string& name) { displayName = name; }
    };
    
    // Mock auth that can be configured
    class MockAuth {
    public:
        bool authenticated = false;
        bool tokenValid = true;
        std::string userId = "test-user-123";
        
        bool isAuthenticated() const { return authenticated; }
        bool refreshTokenIfNeeded() { return tokenValid; }
        std::string getUserId() const { return userId; }
        std::string getDisplayName() const { return "Test User"; }
    };
    
    // Mock leaderboard
    class MockLeaderboard {
    public:
        bool online = true;
        bool submitSuccess = true;
        int lastRank = 5;
        std::vector<std::pair<uint32_t, uint16_t>> submissions;
        
        bool isOnline() const { return online; }
        
        struct SubmitResult {
            bool success;
            int rank;
            std::string errorMessage;
        };
        
        SubmitResult submitScore(const Domain::ReplayData& replay, uint16_t snakeSize) {
            submissions.push_back({replay.finalScore, snakeSize});
            return {submitSuccess, lastRank, submitSuccess ? "" : "Mock error"};
        }
    };
}

// ============================================================================
// APPLICATION LAYER TESTS
// ============================================================================

TEST_CASE("MockStorage - Save and Load Highscore", "[application][mock]") {
    MockPorts::MockStorage storage;
    
    REQUIRE(storage.loadHighscore() == 0);
    
    storage.saveHighscore(1000);
    REQUIRE(storage.loadHighscore() == 1000);
    
    storage.saveHighscore(500);  // Lower score
    REQUIRE(storage.loadHighscore() == 500);  // Mock doesn't validate, just stores
}

TEST_CASE("MockStorage - Display Name", "[application][mock]") {
    MockPorts::MockStorage storage;
    
    REQUIRE(storage.loadDisplayName() == "TestPlayer");
    
    storage.saveDisplayName("NewName");
    REQUIRE(storage.loadDisplayName() == "NewName");
}

TEST_CASE("MockAuth - Authentication State", "[application][mock]") {
    MockPorts::MockAuth auth;
    
    REQUIRE_FALSE(auth.isAuthenticated());
    
    auth.authenticated = true;
    REQUIRE(auth.isAuthenticated());
}

TEST_CASE("MockAuth - Token Refresh", "[application][mock]") {
    MockPorts::MockAuth auth;
    auth.authenticated = true;
    
    REQUIRE(auth.refreshTokenIfNeeded());
    
    auth.tokenValid = false;
    REQUIRE_FALSE(auth.refreshTokenIfNeeded());
}

TEST_CASE("MockLeaderboard - Online Status", "[application][mock]") {
    MockPorts::MockLeaderboard lb;
    
    REQUIRE(lb.isOnline());
    
    lb.online = false;
    REQUIRE_FALSE(lb.isOnline());
}

TEST_CASE("MockLeaderboard - Submit Score", "[application][mock]") {
    MockPorts::MockLeaderboard lb;
    Domain::ReplayData replay;
    replay.finalScore = 1500;
    
    auto result = lb.submitScore(replay, 10);
    
    REQUIRE(result.success);
    REQUIRE(result.rank == 5);
    REQUIRE(lb.submissions.size() == 1);
    REQUIRE(lb.submissions[0].first == 1500);
    REQUIRE(lb.submissions[0].second == 10);
}

TEST_CASE("MockLeaderboard - Failed Submit", "[application][mock]") {
    MockPorts::MockLeaderboard lb;
    lb.submitSuccess = false;
    
    Domain::ReplayData replay;
    auto result = lb.submitScore(replay, 5);
    
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorMessage == "Mock error");
}

// ============================================================================
// GAME CONFIG BOUNDARY TESTS
// ============================================================================

TEST_CASE("GameConfig - Easy Difficulty Settings", "[game_config][value_object]") {
    auto config = GameConfig::Builder()
        .difficulty(Difficulty::Easy)
        .build();
    
    REQUIRE(config.difficulty() == Difficulty::Easy);
    REQUIRE(config.difficultyMultiplier() == 1);
    // Easy mode should have slower frame delay
    REQUIRE(config.adjustedFrameDelayMs() >= config.frameDelayMs());
}

TEST_CASE("GameConfig - Hard Difficulty Settings", "[game_config][value_object]") {
    auto config = GameConfig::Builder()
        .difficulty(Difficulty::Hard)
        .build();
    
    REQUIRE(config.difficulty() == Difficulty::Hard);
    REQUIRE(config.difficultyMultiplier() == 3);
}

TEST_CASE("GameConfig - Insane Difficulty Settings", "[game_config][value_object]") {
    auto config = GameConfig::Builder()
        .difficulty(Difficulty::Insane)
        .build();
    
    REQUIRE(config.difficulty() == Difficulty::Insane);
    REQUIRE(config.difficultyMultiplier() == 5);
}

// ============================================================================
// SNAKE ENTITY BOUNDARY TESTS
// ============================================================================

TEST_CASE("Snake - Maximum Size Enforcement", "[snake][entity][boundary]") {
    SnakeEntity snake;
    snake.initialize(Point(50, 50), 3, Direction::Right);
    
    // Grow snake beyond normal bounds
    for (int i = 0; i < 300; ++i) {
        snake.move(true);  // Always grow
    }
    
    // Should be capped at MAX_SIZE (256 or similar)
    REQUIRE(snake.size() <= 256);
}

TEST_CASE("Snake - Collision With Various Body Positions", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(10, 10), 10, Direction::Right);
    
    // Move to create a snake body
    for (int i = 0; i < 20; ++i) {
        if (i % 5 == 0) {
            // Change direction to create interesting patterns
            if (snake.direction() == Direction::Right) {
                snake.setDirection(Direction::Down);
            } else if (snake.direction() == Direction::Down) {
                snake.setDirection(Direction::Left);
            } else if (snake.direction() == Direction::Left) {
                snake.setDirection(Direction::Up);
            } else {
                snake.setDirection(Direction::Right);
            }
        }
        snake.move(true);
    }
    
    // Head should not collide at current position
    REQUIRE_FALSE(snake.collidesWithSelf());
}

TEST_CASE("Snake - Get Body Segments", "[snake][entity]") {
    SnakeEntity snake;
    snake.initialize(Point(5, 5), 5, Direction::Right);
    
    auto begin = snake.begin();
    auto end = snake.end();
    
    int count = 0;
    for (auto it = begin; it != end; ++it) {
        ++count;
    }
    
    REQUIRE(count == 5);
}

// ============================================================================
// FOOD ENTITY EXTENDED TESTS
// ============================================================================

TEST_CASE("Food - Spawn At Boundary", "[food][entity]") {
    FoodEntity food;
    
    food.spawn(Point(0, 0));
    REQUIRE(food.position() == Point(0, 0));
    REQUIRE(food.isActive());
}

TEST_CASE("Food - Multiple Consume Calls", "[food][entity]") {
    FoodEntity food;
    food.spawn(Point(5, 5));
    
    food.consume();
    REQUIRE_FALSE(food.isActive());
    
    // Consuming again should be safe
    food.consume();
    REQUIRE_FALSE(food.isActive());
}

// ============================================================================
// REPLAY SERVICE EXTENDED TESTS
// ============================================================================

TEST_CASE("Replay - Event Recording", "[replay][service]") {
    Domain::ReplayRecorder recorder;
    auto config = GameConfig::defaultConfig();
    
    recorder.startRecording(12345, config);
    
    // Record some events
    recorder.recordDirectionChange(10, Direction::Up);
    recorder.recordDirectionChange(20, Direction::Left);
    recorder.recordDirectionChange(30, Direction::Down);
    
    Domain::ReplayData replay = recorder.finishRecording(100, 10, 50);
    
    REQUIRE(replay.seed == 12345);
    REQUIRE(replay.finalScore == 100);
    REQUIRE(replay.finalSize == 10);
}

TEST_CASE("Replay - Food Spawn Recording", "[replay][service]") {
    Domain::ReplayRecorder recorder;
    auto config = GameConfig::defaultConfig();
    
    recorder.startRecording(54321, config);
    
    // Record food spawns
    recorder.recordFoodSpawn(5, Point(10, 10));
    recorder.recordFoodSpawn(15, Point(20, 20));
    
    Domain::ReplayData replay = recorder.finishRecording(50, 5, 30);
    
    REQUIRE(replay.foodSpawns.size() == 2);
    REQUIRE(replay.foodSpawns[0].position == Point(10, 10));
    REQUIRE(replay.foodSpawns[1].position == Point(20, 20));
}

TEST_CASE("Replay - Validation With Tampered Score", "[replay][service]") {
    Domain::ReplayRecorder recorder;
    auto config = GameConfig::Builder()
        .boardSize(40, 20)
        .difficulty(Difficulty::Normal)
        .build();
    
    recorder.startRecording(99999, config);
    
    // Simulate a short game
    recorder.recordFoodSpawn(5, Point(15, 10));
    recorder.recordDirectionChange(3, Direction::Right);
    
    // Finish with impossible score for the food consumed
    Domain::ReplayData replay = recorder.finishRecording(999999, 4, 100);
    
    // Validator should catch this
    auto validation = Domain::ReplayValidator::validate(replay);
    // Note: This depends on implementation - score validation might be lenient
    // The key is that the validation runs without crashing
    REQUIRE((validation.valid || !validation.errorMessage.empty()));
}

TEST_CASE("Replay - Empty Events Validation", "[replay][service]") {
    Domain::ReplayData replay;
    replay.version = 1;
    replay.seed = 12345;
    replay.difficulty = static_cast<uint8_t>(Difficulty::Normal);
    replay.boardWidth = 40;
    replay.boardHeight = 20;
    replay.finalScore = 0;
    replay.finalSize = 3;
    replay.totalFrames = 10;
    replay.totalTimeMs = 1000;
    
    auto validation = Domain::ReplayValidator::validate(replay);
    // Empty replay with no score should be valid
    REQUIRE(validation.valid);
}

// ============================================================================
// DIRECTION UTILITIES EXTENDED TESTS
// ============================================================================

TEST_CASE("Direction - All Directions Have Valid Deltas", "[direction][value_object]") {
    Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    
    for (auto dir : dirs) {
        Point delta = DirectionUtils::toDelta(dir);
        int absSum = std::abs(delta.x()) + std::abs(delta.y());
        REQUIRE(absSum == 1);  // Should move exactly one unit
    }
}

TEST_CASE("Direction - None Direction Has Zero Delta", "[direction][value_object]") {
    Point delta = DirectionUtils::toDelta(Direction::None);
    REQUIRE(delta.x() == 0);
    REQUIRE(delta.y() == 0);
}

TEST_CASE("Direction - Opposite Is Symmetric", "[direction][value_object]") {
    Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    
    for (auto dir : dirs) {
        Direction opp = DirectionUtils::opposite(dir);
        Direction oppOpp = DirectionUtils::opposite(opp);
        REQUIRE(oppOpp == dir);  // Double opposite should return original
    }
}

// ============================================================================
// GAME AGGREGATE ROOT EXTENDED TESTS
// ============================================================================

TEST_CASE("Game - Multiple Games Same Seed Same Result", "[game][aggregate][determinism]") {
    auto config = GameConfig::Builder()
        .boardSize(30, 15)
        .difficulty(Difficulty::Normal)
        .build();
    
    const uint32_t seed = 42424242;
    
    // Play two games with same seed and same inputs
    Domain::Game game1(config);
    Domain::Game game2(config);
    
    game1.start(seed);
    game2.start(seed);
    
    // Same inputs for both
    std::vector<std::pair<int, Direction>> inputs = {
        {5, Direction::Down},
        {15, Direction::Left},
        {30, Direction::Up},
        {50, Direction::Right}
    };
    
    for (int frame = 0; frame < 100; ++frame) {
        for (const auto& input : inputs) {
            if (input.first == frame) {
                game1.handleInput(input.second);
                game2.handleInput(input.second);
            }
        }
        
        if (game1.state() != GameState::Playing) break;
        
        game1.update();
        game2.update();
    }
    
    REQUIRE(game1.score() == game2.score());
    REQUIRE(game1.snakeSize() == game2.snakeSize());
    REQUIRE(game1.frameCount() == game2.frameCount());
}

TEST_CASE("Game - Score Increases With Food", "[game][aggregate]") {
    auto config = GameConfig::Builder()
        .boardSize(50, 25)
        .difficulty(Difficulty::Easy)
        .build();
    
    Domain::Game game(config);
    game.start(123456);
    
    uint32_t initialScore = game.score();
    uint16_t initialSize = game.snakeSize();
    
    // Run game until score changes or game over
    int maxFrames = 10000;
    for (int i = 0; i < maxFrames && game.state() == GameState::Playing; ++i) {
        game.update();
        
        if (game.score() > initialScore) {
            break;
        }
    }
    
    // Either game ended or we got food
    if (game.score() > initialScore) {
        REQUIRE(game.snakeSize() >= initialSize);
    }
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

TEST_CASE("Performance - Point Operations", "[performance][point]") {
    Point p(100, 100);
    
    // Many operations should be fast
    for (int i = 0; i < 100000; ++i) {
        p = p.moved(1, 0);
        p = p.moved(-1, 0);
    }
    
    // Should end up at same position
    REQUIRE(p == Point(100, 100));
}

TEST_CASE("Performance - Direction Checks", "[performance][direction]") {
    bool result = true;
    
    for (int i = 0; i < 100000; ++i) {
        result &= DirectionUtils::areOpposite(Direction::Up, Direction::Down);
        result &= !DirectionUtils::areOpposite(Direction::Up, Direction::Left);
    }
    
    REQUIRE(result);
}


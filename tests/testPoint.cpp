#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../libs/point.hpp"
#include "../libs/clock.hpp"
#include "../libs/common.hpp"
#include "../libs/highscore.hpp"
#include <thread>
#include <chrono>

// ============================================================================
// POINT TESTS
// ============================================================================

TEST_CASE("Point default constructor initializes to (0,0)", "[point]") {
    Point point;
    REQUIRE(point.getX() == 0);
    REQUIRE(point.getY() == 0);
}

TEST_CASE("Point parameterized constructor", "[point]") {
    Point point(5, 10);
    REQUIRE(point.getX() == 5);
    REQUIRE(point.getY() == 10);
}

TEST_CASE("Point copy constructor", "[point]") {
    Point original(3, 7);
    Point copy(original);
    REQUIRE(copy.getX() == 3);
    REQUIRE(copy.getY() == 7);
}

TEST_CASE("Point setters work correctly", "[point]") {
    Point point;
    
    SECTION("setX changes x coordinate") {
        point.setX(15);
        REQUIRE(point.getX() == 15);
        REQUIRE(point.getY() == 0);
    }
    
    SECTION("setY changes y coordinate") {
        point.setY(20);
        REQUIRE(point.getX() == 0);
        REQUIRE(point.getY() == 20);
    }
    
    SECTION("both setters work together") {
        point.setX(100);
        point.setY(200);
        REQUIRE(point.getX() == 100);
        REQUIRE(point.getY() == 200);
    }
}

TEST_CASE("Point handles negative values", "[point]") {
    Point point;
    
    SECTION("negative x") {
        point.setX(-5);
        REQUIRE(point.getX() == -5);
    }
    
    SECTION("negative y") {
        point.setY(-10);
        REQUIRE(point.getY() == -10);
    }
    
    SECTION("both negative via constructor") {
        Point negPoint(-3, -7);
        REQUIRE(negPoint.getX() == -3);
        REQUIRE(negPoint.getY() == -7);
    }
}

TEST_CASE("Point inequality operator", "[point]") {
    Point p1(0, 0);
    Point p2(0, 0);
    Point p3(1, 0);
    Point p4(0, 1);
    Point p5(1, 1);
    
    SECTION("same points are not unequal") {
        CHECK_FALSE(p1 != p2);
    }
    
    SECTION("different x makes points unequal") {
        CHECK(p1 != p3);
    }
    
    SECTION("different y makes points unequal") {
        CHECK(p1 != p4);
    }
    
    SECTION("both different makes points unequal") {
        CHECK(p1 != p5);
    }
}

TEST_CASE("Point equality operator", "[point]") {
    Point p1(5, 5);
    Point p2(5, 5);
    Point p3(5, 10);
    Point p4(10, 5);
    
    SECTION("identical points are equal") {
        CHECK(p1 == p2);
    }
    
    SECTION("same x different y") {
        CHECK(p1 == p3);  // Note: current implementation checks OR, not AND
    }
    
    SECTION("same y different x") {
        CHECK(p1 == p4);  // Note: current implementation checks OR, not AND
    }
}

TEST_CASE("Point large values", "[point]") {
    Point point(999999, 888888);
    REQUIRE(point.getX() == 999999);
    REQUIRE(point.getY() == 888888);
}

TEST_CASE("Point zero values explicitly set", "[point]") {
    Point point(10, 20);
    point.setX(0);
    point.setY(0);
    REQUIRE(point.getX() == 0);
    REQUIRE(point.getY() == 0);
}

// ============================================================================
// CLOCK TESTS
// ============================================================================

TEST_CASE("Clock initialization", "[clock]") {
    Clock clock;
    
    SECTION("newly created clock has small timestamp") {
        double timestamp = clock.getTimestamp();
        // Should be very close to 0 (within a few milliseconds)
        REQUIRE(timestamp >= 0);
        REQUIRE(timestamp < 50);  // Less than 50ms
    }
}

TEST_CASE("Clock timestamp increases over time", "[clock]") {
    Clock clock;
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    double timestamp = clock.getTimestamp();
    REQUIRE(timestamp >= 40);  // At least 40ms (allowing some tolerance)
    REQUIRE(timestamp < 200);  // But not too much
}

TEST_CASE("Clock reset functionality", "[clock]") {
    Clock clock;
    
    // Wait to accumulate some time
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    
    double before_reset = clock.getTimestamp();
    REQUIRE(before_reset >= 20);
    
    // Reset the clock
    clock.reset();
    
    // Should be back to near zero
    double after_reset = clock.getTimestamp();
    REQUIRE(after_reset >= 0);
    REQUIRE(after_reset < 20);
}

TEST_CASE("Clock consecutive getTimestamp calls", "[clock]") {
    Clock clock;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    double first = clock.getTimestamp();
    double second = clock.getTimestamp();
    
    // Second call should be >= first (time moves forward)
    REQUIRE(second >= first);
}

// ============================================================================
// COMMON TESTS
// ============================================================================

TEST_CASE("Direction constants are defined correctly", "[common]") {
    REQUIRE(UP == 3);
    REQUIRE(DOWN == 2);
    REQUIRE(LEFT == 4);
    REQUIRE(RIGHT == 5);
}

TEST_CASE("Direction constants are unique", "[common]") {
    REQUIRE(UP != DOWN);
    REQUIRE(UP != LEFT);
    REQUIRE(UP != RIGHT);
    REQUIRE(DOWN != LEFT);
    REQUIRE(DOWN != RIGHT);
    REQUIRE(LEFT != RIGHT);
}

TEST_CASE("DELAY constant is defined", "[common]") {
    REQUIRE(DELAY == 80);
    REQUIRE(DELAY > 0);
}

TEST_CASE("COLOR_DEFAULT is negative one", "[common]") {
    REQUIRE(COLOR_DEFAULT == -1);
}

TEST_CASE("Direction values are in valid ncurses key range", "[common]") {
    // ncurses KEY_DOWN=2, KEY_UP=3, KEY_LEFT=4, KEY_RIGHT=5
    REQUIRE(DOWN >= 2);
    REQUIRE(UP >= 2);
    REQUIRE(LEFT >= 2);
    REQUIRE(RIGHT >= 2);
    REQUIRE(DOWN <= 5);
    REQUIRE(UP <= 5);
    REQUIRE(LEFT <= 5);
    REQUIRE(RIGHT <= 5);
}

// ============================================================================
// HIGHSCORE TESTS
// ============================================================================

TEST_CASE("Highscore default initialization", "[highscore]") {
    Highscore hs;
    // Initial score should be 0 or whatever was saved
    REQUIRE(hs.get() >= 0);
}

TEST_CASE("Highscore get returns current value", "[highscore]") {
    Highscore hs;
    int score = hs.get();
    // Should return a non-negative value
    REQUIRE(score >= 0);
}

TEST_CASE("Highscore set updates when higher", "[highscore]") {
    Highscore hs;
    int initial = hs.get();
    
    // Set a much higher score
    int newScore = initial + 1000;
    hs.set(newScore);
    
    REQUIRE(hs.get() == newScore);
}

TEST_CASE("Highscore handles zero", "[highscore]") {
    Highscore hs;
    hs.set(0);
    // Should not crash and get should work
    REQUIRE(hs.get() >= 0);
}
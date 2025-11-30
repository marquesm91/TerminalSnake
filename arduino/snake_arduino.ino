/**
 * Terminal Snake for Arduino/ESP32
 * 
 * Hardware Requirements:
 * - ESP32 or ESP8266 (for WiFi/leaderboard support)
 * - OLED Display (SSD1306 128x64 recommended)
 * - 4 Push buttons for controls
 * 
 * Wiring:
 * - BTN_UP    -> GPIO 2
 * - BTN_DOWN  -> GPIO 3
 * - BTN_LEFT  -> GPIO 4
 * - BTN_RIGHT -> GPIO 5
 * - OLED SDA  -> GPIO 21 (ESP32) or GPIO 4 (ESP8266)
 * - OLED SCL  -> GPIO 22 (ESP32) or GPIO 5 (ESP8266)
 * 
 * Libraries Required:
 * - Adafruit_SSD1306 or U8g2
 * - WiFi (for ESP)
 * - HTTPClient (for ESP)
 */

#ifdef ARDUINO  // Only compile this file for Arduino

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Include core game engine
#include "libs/core/types.hpp"
#include "libs/core/snake.hpp"
#include "libs/core/random.hpp"
#include "libs/core/replay.hpp"

// WiFi credentials (for leaderboard)
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASS "YourWiFiPassword"

// Firebase configuration
#define FIREBASE_URL "https://firestore.googleapis.com/v1/projects/YOUR_PROJECT/databases/(default)/documents"

// Button pins
#define BTN_UP    2
#define BTN_DOWN  3
#define BTN_LEFT  4
#define BTN_RIGHT 5

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define RENDER_SCALE 4

// Game board size (based on display)
#define BOARD_WIDTH  (SCREEN_WIDTH / RENDER_SCALE)
#define BOARD_HEIGHT (SCREEN_HEIGHT / RENDER_SCALE)

// Objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SnakeCore::Snake snake;
SnakeCore::Random rng;
SnakeCore::ReplayRecorder recorder;
SnakeCore::Point foodPos;

// Game state
uint32_t score = 0;
uint32_t frameCount = 0;
uint32_t gameStartTime = 0;
uint32_t lastFrameTime = 0;
bool gameOver = false;
uint8_t difficulty = 2;
uint16_t frameDelay = 150;

// WiFi/Network (ESP only)
#if defined(ESP8266) || defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
bool wifiConnected = false;
#endif

void initButtons() {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
}

void initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true) { delay(1000); }
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

#if defined(ESP8266) || defined(ESP32)
void initWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nConnected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect");
    }
}

bool submitScore() {
    if (!wifiConnected) return false;
    
    HTTPClient http;
    http.begin(String(FIREBASE_URL) + "/leaderboard/arduino_device");
    http.addHeader("Content-Type", "application/json");
    
    // Create replay data
    std::vector<uint8_t> replayData = recorder.getReplayData().serialize();
    
    // Convert to hex string for transmission
    String hexReplay = "";
    for (uint8_t b : replayData) {
        if (b < 16) hexReplay += "0";
        hexReplay += String(b, HEX);
    }
    
    // Create JSON
    String json = "{\"fields\":{";
    json += "\"score\":{\"integerValue\":\"" + String(score) + "\"},";
    json += "\"size\":{\"integerValue\":\"" + String(snake.getSize()) + "\"},";
    json += "\"device\":{\"stringValue\":\"arduino\"},";
    json += "\"replay\":{\"stringValue\":\"" + hexReplay + "\"}";
    json += "}}";
    
    int httpCode = http.PATCH(json);
    http.end();
    
    return (httpCode >= 200 && httpCode < 300);
}
#endif

int8_t readInput() {
    if (digitalRead(BTN_UP) == LOW) return SnakeCore::DIR_UP;
    if (digitalRead(BTN_DOWN) == LOW) return SnakeCore::DIR_DOWN;
    if (digitalRead(BTN_LEFT) == LOW) return SnakeCore::DIR_LEFT;
    if (digitalRead(BTN_RIGHT) == LOW) return SnakeCore::DIR_RIGHT;
    return SnakeCore::DIR_NONE;
}

void spawnFood() {
    int16_t x, y;
    int maxAttempts = 50;
    
    do {
        x = rng.range(1, BOARD_HEIGHT - 2);
        y = rng.range(1, BOARD_WIDTH - 2);
        maxAttempts--;
    } while (snake.containsPoint(SnakeCore::Point(x, y)) && maxAttempts > 0);
    
    foodPos.x = x;
    foodPos.y = y;
    
    recorder.recordFoodSpawn(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
}

void renderGame() {
    display.clearDisplay();
    
    // Draw border
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    
    // Draw snake
    for (uint16_t i = 0; i < snake.getSize(); i++) {
        SnakeCore::Point seg = snake.getBodySegment(i);
        int16_t px = seg.y * RENDER_SCALE;
        int16_t py = seg.x * RENDER_SCALE;
        
        if (i == snake.getSize() - 1) {
            // Head - filled
            display.fillRect(px, py, RENDER_SCALE, RENDER_SCALE, SSD1306_WHITE);
        } else {
            // Body - hollow
            display.drawRect(px + 1, py + 1, RENDER_SCALE - 2, RENDER_SCALE - 2, SSD1306_WHITE);
        }
    }
    
    // Draw food
    int16_t fx = foodPos.y * RENDER_SCALE + RENDER_SCALE/2;
    int16_t fy = foodPos.x * RENDER_SCALE + RENDER_SCALE/2;
    display.fillCircle(fx, fy, RENDER_SCALE/2 - 1, SSD1306_WHITE);
    
    // Draw score (small, at top)
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print(score);
    
    display.display();
}

void showGameOver() {
    display.clearDisplay();
    
    display.setTextSize(2);
    display.setCursor(20, 15);
    display.println("GAME");
    display.setCursor(20, 35);
    display.println("OVER");
    
    display.setTextSize(1);
    display.setCursor(30, 55);
    display.print("Score: ");
    display.print(score);
    
    display.display();
    
    // Submit score if connected
    #if defined(ESP8266) || defined(ESP32)
    if (wifiConnected) {
        display.setCursor(2, 2);
        display.print("Uploading...");
        display.display();
        
        if (submitScore()) {
            display.setCursor(2, 2);
            display.print("Uploaded!   ");
        } else {
            display.setCursor(2, 2);
            display.print("Upload failed");
        }
        display.display();
    }
    #endif
    
    // Wait for button press to restart
    while (readInput() == SnakeCore::DIR_NONE) {
        delay(50);
    }
    delay(200);  // Debounce
}

void startGame() {
    // Initialize RNG with current time
    uint32_t seed = millis();
    rng.seed(seed);
    
    // Initialize snake
    SnakeCore::Point startPos(BOARD_HEIGHT / 2, BOARD_WIDTH / 4);
    snake.init(startPos, 3, SnakeCore::DIR_RIGHT);
    
    // Reset game state
    score = 0;
    frameCount = 0;
    gameStartTime = millis();
    lastFrameTime = gameStartTime;
    gameOver = false;
    
    // Start recording
    recorder.startRecording(seed, difficulty, BOARD_WIDTH, BOARD_HEIGHT);
    
    // Spawn first food
    spawnFood();
    
    // Initial render
    renderGame();
}

void gameLoop() {
    uint32_t currentTime = millis();
    
    if (currentTime - lastFrameTime < frameDelay) {
        return;
    }
    
    lastFrameTime = currentTime;
    frameCount++;
    
    // Read input
    int8_t input = readInput();
    if (input != SnakeCore::DIR_NONE) {
        if (snake.setDirection(input)) {
            recorder.recordInput(input, currentTime - gameStartTime);
        }
    }
    
    // Calculate next position
    SnakeCore::Point nextHead = snake.calculateNextHead();
    
    // Check wall collision
    if (nextHead.x <= 0 || nextHead.x >= BOARD_HEIGHT - 1 ||
        nextHead.y <= 0 || nextHead.y >= BOARD_WIDTH - 1) {
        gameOver = true;
        return;
    }
    
    // Check self collision
    if (snake.collidesWithSelf(nextHead)) {
        gameOver = true;
        return;
    }
    
    // Check food collision
    if (nextHead == foodPos) {
        snake.grow();
        score += difficulty;
        spawnFood();
    } else {
        snake.move();
    }
    
    recorder.advanceFrame();
    renderGame();
}

void setup() {
    Serial.begin(115200);
    
    initButtons();
    initDisplay();
    
    #if defined(ESP8266) || defined(ESP32)
    initWiFi();
    #endif
    
    // Show title
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 10);
    display.println("SNAKE");
    display.setTextSize(1);
    display.setCursor(15, 35);
    display.println("Press any button");
    display.display();
    
    // Wait for button press
    while (readInput() == SnakeCore::DIR_NONE) {
        delay(50);
    }
    delay(200);  // Debounce
    
    startGame();
}

void loop() {
    if (gameOver) {
        // Stop recording
        recorder.stopRecording(score, snake.getSize(), millis() - gameStartTime);
        
        showGameOver();
        startGame();
    } else {
        gameLoop();
    }
}

#endif // ARDUINO

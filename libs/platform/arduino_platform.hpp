#ifndef PLATFORM_ARDUINO_H_
#define PLATFORM_ARDUINO_H_

// This file is only compiled for Arduino targets
#ifdef ARDUINO

#include <Arduino.h>
#include "../core/engine.hpp"

// Optional: Include display library
// #include <Adafruit_SSD1306.h>  // For OLED displays
// #include <U8g2lib.h>           // Alternative display library

namespace SnakePlatform {

// Button pin definitions - adjust for your hardware
#ifndef BTN_UP
#define BTN_UP    2
#define BTN_DOWN  3
#define BTN_LEFT  4
#define BTN_RIGHT 5
#endif

// Display dimensions - adjust for your display
#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64
#endif

// Scale factor for rendering (1 pixel = SCALE x SCALE on display)
#ifndef RENDER_SCALE
#define RENDER_SCALE 4
#endif

class ArduinoPlatform : public SnakeCore::IPlatform {
private:
    uint8_t boardWidth;
    uint8_t boardHeight;
    uint32_t lastButtonState;
    
    // Display object - uncomment and configure for your display
    // Adafruit_SSD1306 display;
    // or
    // U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

public:
    ArduinoPlatform() : 
        boardWidth(DISPLAY_WIDTH / RENDER_SCALE),
        boardHeight(DISPLAY_HEIGHT / RENDER_SCALE),
        lastButtonState(0) {}
    
    void init() {
        // Initialize buttons with internal pull-ups
        pinMode(BTN_UP, INPUT_PULLUP);
        pinMode(BTN_DOWN, INPUT_PULLUP);
        pinMode(BTN_LEFT, INPUT_PULLUP);
        pinMode(BTN_RIGHT, INPUT_PULLUP);
        
        // Initialize display
        // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        // display.clearDisplay();
        // display.display();
        
        // or for U8g2:
        // u8g2.begin();
    }
    
    void shutdown() {
        // Clean up if needed
    }
    
    // IPlatform implementation
    
    uint32_t getTimeMs() override {
        return millis();
    }
    
    void delay(uint16_t ms) override {
        ::delay(ms);
    }
    
    int8_t getInput() override {
        // Read buttons (active low with pull-ups)
        if (digitalRead(BTN_UP) == LOW) {
            return SnakeCore::DIR_UP;
        }
        if (digitalRead(BTN_DOWN) == LOW) {
            return SnakeCore::DIR_DOWN;
        }
        if (digitalRead(BTN_LEFT) == LOW) {
            return SnakeCore::DIR_LEFT;
        }
        if (digitalRead(BTN_RIGHT) == LOW) {
            return SnakeCore::DIR_RIGHT;
        }
        return SnakeCore::DIR_NONE;
    }
    
    void clear() override {
        // display.clearDisplay();
        // or u8g2.clearBuffer();
    }
    
    void drawSnakeHead(int16_t x, int16_t y) override {
        // Draw filled circle for head
        int16_t px = y * RENDER_SCALE;
        int16_t py = x * RENDER_SCALE;
        // display.fillCircle(px + RENDER_SCALE/2, py + RENDER_SCALE/2, RENDER_SCALE/2, WHITE);
        // or u8g2.drawBox(px, py, RENDER_SCALE, RENDER_SCALE);
    }
    
    void drawSnakeBody(int16_t x, int16_t y) override {
        // Draw rectangle for body
        int16_t px = y * RENDER_SCALE;
        int16_t py = x * RENDER_SCALE;
        // display.fillRect(px + 1, py + 1, RENDER_SCALE - 2, RENDER_SCALE - 2, WHITE);
        // or u8g2.drawBox(px + 1, py + 1, RENDER_SCALE - 2, RENDER_SCALE - 2);
    }
    
    void drawFood(int16_t x, int16_t y) override {
        // Draw diamond/cross for food
        int16_t px = y * RENDER_SCALE + RENDER_SCALE/2;
        int16_t py = x * RENDER_SCALE + RENDER_SCALE/2;
        // display.drawPixel(px, py - 1, WHITE);
        // display.drawPixel(px - 1, py, WHITE);
        // display.drawPixel(px, py, WHITE);
        // display.drawPixel(px + 1, py, WHITE);
        // display.drawPixel(px, py + 1, WHITE);
    }
    
    void drawWall(int16_t x, int16_t y) override {
        // Draw wall segment
        int16_t px = y * RENDER_SCALE;
        int16_t py = x * RENDER_SCALE;
        // display.drawRect(px, py, RENDER_SCALE, RENDER_SCALE, WHITE);
    }
    
    void drawScore(uint32_t score, uint16_t size) override {
        // Draw score at top of screen
        // display.setTextSize(1);
        // display.setTextColor(WHITE);
        // display.setCursor(0, 0);
        // display.print("S:");
        // display.print(score);
        (void)score;
        (void)size;
    }
    
    void drawGameOver(uint32_t score) override {
        clear();
        // display.setTextSize(2);
        // display.setTextColor(WHITE);
        // display.setCursor(20, 20);
        // display.println("GAME");
        // display.setCursor(20, 40);
        // display.println("OVER");
        // display.setTextSize(1);
        // display.setCursor(20, 55);
        // display.print("Score: ");
        // display.print(score);
        (void)score;
        refresh();
    }
    
    void refresh() override {
        // display.display();
        // or u8g2.sendBuffer();
    }
    
    uint8_t getBoardWidth() override { return boardWidth; }
    uint8_t getBoardHeight() override { return boardHeight; }
};

// Network module for score submission (ESP8266/ESP32)
#if defined(ESP8266) || defined(ESP32)

#include <WiFi.h>
#include <HTTPClient.h>

class ArduinoNetwork {
private:
    const char* ssid;
    const char* password;
    const char* firebaseUrl;
    String authToken;
    bool connected;

public:
    ArduinoNetwork(const char* wifiSSID, const char* wifiPass, const char* fbUrl) :
        ssid(wifiSSID), password(wifiPass), firebaseUrl(fbUrl), connected(false) {}
    
    bool connect() {
        WiFi.begin(ssid, password);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        
        connected = (WiFi.status() == WL_CONNECTED);
        return connected;
    }
    
    void setAuthToken(const String& token) {
        authToken = token;
    }
    
    bool submitReplay(const SnakeCore::ReplayData& replay, const String& odUserId) {
        if (!connected || authToken.isEmpty()) {
            return false;
        }
        
        HTTPClient http;
        
        String url = String(firebaseUrl) + "/leaderboard/" + odUserId;
        http.begin(url);
        http.addHeader("Authorization", "Bearer " + authToken);
        http.addHeader("Content-Type", "application/json");
        
        // Convert replay to Base64
        String replayData = replay.toBase64().c_str();
        
        // Create JSON payload
        String payload = "{\"fields\":{";
        payload += "\"highscore\":{\"integerValue\":\"" + String(replay.getFinalScore()) + "\"},";
        payload += "\"snakeSize\":{\"integerValue\":\"" + String(replay.getFinalSize()) + "\"},";
        payload += "\"difficulty\":{\"integerValue\":\"" + String(replay.getDifficulty()) + "\"},";
        payload += "\"replayData\":{\"stringValue\":\"" + replayData + "\"}";
        payload += "}}";
        
        int httpCode = http.PATCH(payload);
        http.end();
        
        return (httpCode >= 200 && httpCode < 300);
    }
    
    bool isConnected() const { return connected; }
};

#endif // ESP8266 || ESP32

} // namespace SnakePlatform

#endif // ARDUINO

#endif // PLATFORM_ARDUINO_H_

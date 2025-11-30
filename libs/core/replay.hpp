#ifndef REPLAY_H_
#define REPLAY_H_

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

namespace SnakeCore {

// Represents a single input event in the replay
struct ReplayEvent {
    uint32_t frameNumber;    // Frame when input occurred
    int8_t direction;        // Direction: UP=3, DOWN=2, LEFT=4, RIGHT=5
    uint16_t deltaMs;        // Time since last event in milliseconds
    
    ReplayEvent() : frameNumber(0), direction(0), deltaMs(0) {}
    ReplayEvent(uint32_t frame, int8_t dir, uint16_t delta) 
        : frameNumber(frame), direction(dir), deltaMs(delta) {}
};

// Represents a food spawn event (deterministic with seed)
struct FoodSpawn {
    uint32_t frameNumber;
    uint8_t x;
    uint8_t y;
    
    FoodSpawn() : frameNumber(0), x(0), y(0) {}
    FoodSpawn(uint32_t frame, uint8_t px, uint8_t py) 
        : frameNumber(frame), x(px), y(py) {}
};

// Complete replay data for server-side validation
class ReplayData {
private:
    uint32_t seed;                          // Random seed for deterministic replay
    uint8_t difficulty;                     // Difficulty level
    uint8_t boardWidth;                     // Board dimensions
    uint8_t boardHeight;
    std::vector<ReplayEvent> events;        // All input events
    std::vector<FoodSpawn> foodSpawns;      // All food positions (for validation)
    uint32_t finalScore;
    uint32_t finalSize;
    uint32_t totalFrames;
    uint32_t totalTimeMs;
    std::string gameVersion;

public:
    ReplayData() : seed(0), difficulty(1), boardWidth(80), boardHeight(24),
                   finalScore(0), finalSize(3), totalFrames(0), totalTimeMs(0),
                   gameVersion("2.0") {}
    
    // Setters
    void setSeed(uint32_t s) { seed = s; }
    void setDifficulty(uint8_t d) { difficulty = d; }
    void setBoardSize(uint8_t w, uint8_t h) { boardWidth = w; boardHeight = h; }
    void setFinalScore(uint32_t s) { finalScore = s; }
    void setFinalSize(uint32_t s) { finalSize = s; }
    void setTotalFrames(uint32_t f) { totalFrames = f; }
    void setTotalTimeMs(uint32_t t) { totalTimeMs = t; }
    void setGameVersion(const std::string& v) { gameVersion = v; }
    
    // Getters
    uint32_t getSeed() const { return seed; }
    uint8_t getDifficulty() const { return difficulty; }
    uint8_t getBoardWidth() const { return boardWidth; }
    uint8_t getBoardHeight() const { return boardHeight; }
    uint32_t getFinalScore() const { return finalScore; }
    uint32_t getFinalSize() const { return finalSize; }
    uint32_t getTotalFrames() const { return totalFrames; }
    uint32_t getTotalTimeMs() const { return totalTimeMs; }
    const std::string& getGameVersion() const { return gameVersion; }
    const std::vector<ReplayEvent>& getEvents() const { return events; }
    const std::vector<FoodSpawn>& getFoodSpawns() const { return foodSpawns; }
    
    // Recording
    void addEvent(uint32_t frame, int8_t direction, uint16_t deltaMs) {
        events.emplace_back(frame, direction, deltaMs);
    }
    
    void addFoodSpawn(uint32_t frame, uint8_t x, uint8_t y) {
        foodSpawns.emplace_back(frame, x, y);
    }
    
    void clear() {
        events.clear();
        foodSpawns.clear();
        finalScore = 0;
        finalSize = 3;
        totalFrames = 0;
        totalTimeMs = 0;
    }
    
    // Serialize to compact binary format for transmission
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        
        // Header (16 bytes)
        // Magic number "SNRP" (Snake Replay)
        data.push_back('S');
        data.push_back('N');
        data.push_back('R');
        data.push_back('P');
        
        // Version (1 byte)
        data.push_back(1);
        
        // Seed (4 bytes, big-endian)
        data.push_back((seed >> 24) & 0xFF);
        data.push_back((seed >> 16) & 0xFF);
        data.push_back((seed >> 8) & 0xFF);
        data.push_back(seed & 0xFF);
        
        // Difficulty (1 byte)
        data.push_back(difficulty);
        
        // Board size (2 bytes)
        data.push_back(boardWidth);
        data.push_back(boardHeight);
        
        // Final score (4 bytes)
        data.push_back((finalScore >> 24) & 0xFF);
        data.push_back((finalScore >> 16) & 0xFF);
        data.push_back((finalScore >> 8) & 0xFF);
        data.push_back(finalScore & 0xFF);
        
        // Final size (2 bytes)
        data.push_back((finalSize >> 8) & 0xFF);
        data.push_back(finalSize & 0xFF);
        
        // Total frames (4 bytes)
        data.push_back((totalFrames >> 24) & 0xFF);
        data.push_back((totalFrames >> 16) & 0xFF);
        data.push_back((totalFrames >> 8) & 0xFF);
        data.push_back(totalFrames & 0xFF);
        
        // Total time ms (4 bytes)
        data.push_back((totalTimeMs >> 24) & 0xFF);
        data.push_back((totalTimeMs >> 16) & 0xFF);
        data.push_back((totalTimeMs >> 8) & 0xFF);
        data.push_back(totalTimeMs & 0xFF);
        
        // Number of events (2 bytes)
        uint16_t numEvents = static_cast<uint16_t>(events.size());
        data.push_back((numEvents >> 8) & 0xFF);
        data.push_back(numEvents & 0xFF);
        
        // Events (7 bytes each: 4 frame + 1 dir + 2 delta)
        for (const auto& event : events) {
            data.push_back((event.frameNumber >> 24) & 0xFF);
            data.push_back((event.frameNumber >> 16) & 0xFF);
            data.push_back((event.frameNumber >> 8) & 0xFF);
            data.push_back(event.frameNumber & 0xFF);
            data.push_back(static_cast<uint8_t>(event.direction));
            data.push_back((event.deltaMs >> 8) & 0xFF);
            data.push_back(event.deltaMs & 0xFF);
        }
        
        // Number of food spawns (2 bytes)
        uint16_t numFood = static_cast<uint16_t>(foodSpawns.size());
        data.push_back((numFood >> 8) & 0xFF);
        data.push_back(numFood & 0xFF);
        
        // Food spawns (6 bytes each: 4 frame + 1 x + 1 y)
        for (const auto& food : foodSpawns) {
            data.push_back((food.frameNumber >> 24) & 0xFF);
            data.push_back((food.frameNumber >> 16) & 0xFF);
            data.push_back((food.frameNumber >> 8) & 0xFF);
            data.push_back(food.frameNumber & 0xFF);
            data.push_back(food.x);
            data.push_back(food.y);
        }
        
        return data;
    }
    
    // Deserialize from binary format
    bool deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < 32) return false;
        
        // Check magic number
        if (data[0] != 'S' || data[1] != 'N' || data[2] != 'R' || data[3] != 'P') {
            return false;
        }
        
        // Version check
        if (data[4] != 1) return false;
        
        size_t pos = 5;
        
        // Seed
        seed = (static_cast<uint32_t>(data[pos]) << 24) |
               (static_cast<uint32_t>(data[pos+1]) << 16) |
               (static_cast<uint32_t>(data[pos+2]) << 8) |
               static_cast<uint32_t>(data[pos+3]);
        pos += 4;
        
        // Difficulty
        difficulty = data[pos++];
        
        // Board size
        boardWidth = data[pos++];
        boardHeight = data[pos++];
        
        // Final score
        finalScore = (static_cast<uint32_t>(data[pos]) << 24) |
                     (static_cast<uint32_t>(data[pos+1]) << 16) |
                     (static_cast<uint32_t>(data[pos+2]) << 8) |
                     static_cast<uint32_t>(data[pos+3]);
        pos += 4;
        
        // Final size
        finalSize = (static_cast<uint16_t>(data[pos]) << 8) |
                    static_cast<uint16_t>(data[pos+1]);
        pos += 2;
        
        // Total frames
        totalFrames = (static_cast<uint32_t>(data[pos]) << 24) |
                      (static_cast<uint32_t>(data[pos+1]) << 16) |
                      (static_cast<uint32_t>(data[pos+2]) << 8) |
                      static_cast<uint32_t>(data[pos+3]);
        pos += 4;
        
        // Total time
        totalTimeMs = (static_cast<uint32_t>(data[pos]) << 24) |
                      (static_cast<uint32_t>(data[pos+1]) << 16) |
                      (static_cast<uint32_t>(data[pos+2]) << 8) |
                      static_cast<uint32_t>(data[pos+3]);
        pos += 4;
        
        // Number of events
        uint16_t numEvents = (static_cast<uint16_t>(data[pos]) << 8) |
                             static_cast<uint16_t>(data[pos+1]);
        pos += 2;
        
        // Events
        events.clear();
        for (uint16_t i = 0; i < numEvents && pos + 7 <= data.size(); i++) {
            uint32_t frame = (static_cast<uint32_t>(data[pos]) << 24) |
                             (static_cast<uint32_t>(data[pos+1]) << 16) |
                             (static_cast<uint32_t>(data[pos+2]) << 8) |
                             static_cast<uint32_t>(data[pos+3]);
            int8_t dir = static_cast<int8_t>(data[pos+4]);
            uint16_t delta = (static_cast<uint16_t>(data[pos+5]) << 8) |
                             static_cast<uint16_t>(data[pos+6]);
            events.emplace_back(frame, dir, delta);
            pos += 7;
        }
        
        // Number of food spawns
        if (pos + 2 > data.size()) return false;
        uint16_t numFood = (static_cast<uint16_t>(data[pos]) << 8) |
                           static_cast<uint16_t>(data[pos+1]);
        pos += 2;
        
        // Food spawns
        foodSpawns.clear();
        for (uint16_t i = 0; i < numFood && pos + 6 <= data.size(); i++) {
            uint32_t frame = (static_cast<uint32_t>(data[pos]) << 24) |
                             (static_cast<uint32_t>(data[pos+1]) << 16) |
                             (static_cast<uint32_t>(data[pos+2]) << 8) |
                             static_cast<uint32_t>(data[pos+3]);
            uint8_t x = data[pos+4];
            uint8_t y = data[pos+5];
            foodSpawns.emplace_back(frame, x, y);
            pos += 6;
        }
        
        return true;
    }
    
    // Convert to Base64 for JSON transmission
    std::string toBase64() const {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::vector<uint8_t> data = serialize();
        std::string result;
        
        int i = 0;
        uint8_t char_array_3[3];
        uint8_t char_array_4[4];
        size_t in_len = data.size();
        size_t pos = 0;
        
        while (in_len--) {
            char_array_3[i++] = data[pos++];
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    result += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (int j = i; j < 3; j++)
                char_array_3[j] = '\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (int j = 0; j < i + 1; j++)
                result += base64_chars[char_array_4[j]];
            
            while (i++ < 3)
                result += '=';
        }
        
        return result;
    }
};

// Replay recorder - records game session
class ReplayRecorder {
private:
    ReplayData data;
    uint32_t currentFrame;
    uint32_t lastEventTimeMs;
    bool recording;

public:
    ReplayRecorder() : currentFrame(0), lastEventTimeMs(0), recording(false) {}
    
    void startRecording(uint32_t seed, uint8_t difficulty, uint8_t width, uint8_t height) {
        data.clear();
        data.setSeed(seed);
        data.setDifficulty(difficulty);
        data.setBoardSize(width, height);
        currentFrame = 0;
        lastEventTimeMs = 0;
        recording = true;
    }
    
    void recordInput(int8_t direction, uint32_t currentTimeMs) {
        if (!recording) return;
        
        uint16_t delta = static_cast<uint16_t>(currentTimeMs - lastEventTimeMs);
        data.addEvent(currentFrame, direction, delta);
        lastEventTimeMs = currentTimeMs;
    }
    
    void recordFoodSpawn(uint8_t x, uint8_t y) {
        if (!recording) return;
        data.addFoodSpawn(currentFrame, x, y);
    }
    
    void advanceFrame() {
        if (recording) currentFrame++;
    }
    
    void stopRecording(uint32_t score, uint32_t size, uint32_t totalTimeMs) {
        data.setFinalScore(score);
        data.setFinalSize(size);
        data.setTotalFrames(currentFrame);
        data.setTotalTimeMs(totalTimeMs);
        recording = false;
    }
    
    bool isRecording() const { return recording; }
    const ReplayData& getReplayData() const { return data; }
    uint32_t getCurrentFrame() const { return currentFrame; }
};

// Replay player - replays and validates game session
class ReplayPlayer {
private:
    ReplayData data;
    size_t currentEventIndex;
    size_t currentFoodIndex;
    uint32_t currentFrame;
    uint32_t simulatedScore;
    uint32_t simulatedSize;
    bool valid;
    std::string validationError;

public:
    ReplayPlayer() : currentEventIndex(0), currentFoodIndex(0), currentFrame(0),
                     simulatedScore(0), simulatedSize(3), valid(true) {}
    
    bool loadReplay(const ReplayData& replayData) {
        data = replayData;
        currentEventIndex = 0;
        currentFoodIndex = 0;
        currentFrame = 0;
        simulatedScore = 0;
        simulatedSize = 3;
        valid = true;
        validationError.clear();
        return true;
    }
    
    // Get next input for current frame (returns -1 if no input)
    int8_t getNextInput() {
        const auto& events = data.getEvents();
        if (currentEventIndex < events.size() && 
            events[currentEventIndex].frameNumber == currentFrame) {
            return events[currentEventIndex++].direction;
        }
        return -1;
    }
    
    // Validate food spawn at current frame
    bool validateFoodSpawn(uint8_t x, uint8_t y) {
        const auto& spawns = data.getFoodSpawns();
        if (currentFoodIndex < spawns.size() &&
            spawns[currentFoodIndex].frameNumber == currentFrame) {
            if (spawns[currentFoodIndex].x != x || spawns[currentFoodIndex].y != y) {
                valid = false;
                validationError = "Food position mismatch";
                return false;
            }
            currentFoodIndex++;
        }
        return true;
    }
    
    void advanceFrame() { currentFrame++; }
    void addScore(uint32_t points) { simulatedScore += points; }
    void incrementSize() { simulatedSize++; }
    
    // Validate final results
    bool validateFinalResults() {
        if (simulatedScore != data.getFinalScore()) {
            valid = false;
            validationError = "Score mismatch: expected " + 
                std::to_string(data.getFinalScore()) + 
                ", got " + std::to_string(simulatedScore);
            return false;
        }
        if (simulatedSize != data.getFinalSize()) {
            valid = false;
            validationError = "Size mismatch: expected " +
                std::to_string(data.getFinalSize()) +
                ", got " + std::to_string(simulatedSize);
            return false;
        }
        return true;
    }
    
    bool isValid() const { return valid; }
    const std::string& getValidationError() const { return validationError; }
    uint32_t getSimulatedScore() const { return simulatedScore; }
    uint32_t getSimulatedSize() const { return simulatedSize; }
};

} // namespace SnakeCore

#endif

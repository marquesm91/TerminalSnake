/**
 * @file replay_service.hpp
 * @brief ReplayService - Gravação e validação de replays
 * 
 * Clean Architecture: Domain Layer - Domain Service
 */

#ifndef DOMAIN_SERVICES_REPLAY_SERVICE_HPP
#define DOMAIN_SERVICES_REPLAY_SERVICE_HPP

#include <cstdint>
#include <vector>
#include <string>
#include "domain/value_objects/point.hpp"
#include "domain/value_objects/direction.hpp"
#include "domain/value_objects/game_config.hpp"
#include "domain/entities/game.hpp"

namespace Snake {
namespace Domain {

/**
 * @brief Evento gravado no replay
 */
struct ReplayEvent {
    uint32_t frame;
    GameEventType type;
    union {
        uint8_t directionValue;  // Para DirectionChanged
        struct {
            int16_t x;
            int16_t y;
        } position;              // Para FoodSpawned
    } data;
    
    ReplayEvent() : frame(0), type(GameEventType::GameStarted) {
        data.directionValue = 0;
    }
    
    static ReplayEvent directionChange(uint32_t frame, Direction dir) {
        ReplayEvent e;
        e.frame = frame;
        e.type = GameEventType::DirectionChanged;
        e.data.directionValue = static_cast<uint8_t>(dir);
        return e;
    }
    
    static ReplayEvent foodSpawn(uint32_t frame, Point pos) {
        ReplayEvent e;
        e.frame = frame;
        e.type = GameEventType::FoodSpawned;
        e.data.position.x = pos.x();
        e.data.position.y = pos.y();
        return e;
    }
};

/**
 * @brief Dados completos de um replay
 */
class ReplayData {
public:
    static const uint32_t MAGIC = 0x534E5250; // "SNRP"
    static const uint8_t VERSION = 1;
    
    ReplayData() : seed_(0), finalScore_(0), totalFrames_(0) {}
    
    // Setters
    void setSeed(uint32_t seed) { seed_ = seed; }
    void setConfig(const GameConfig& config) { config_ = config; }
    void setFinalScore(uint32_t score) { finalScore_ = score; }
    void setTotalFrames(uint32_t frames) { totalFrames_ = frames; }
    void setGameVersion(const std::string& version) { gameVersion_ = version; }
    
    void addEvent(const ReplayEvent& event) {
        events_.push_back(event);
    }
    
    void clear() {
        events_.clear();
        seed_ = 0;
        finalScore_ = 0;
        totalFrames_ = 0;
    }
    
    // Getters
    uint32_t seed() const { return seed_; }
    const GameConfig& config() const { return config_; }
    uint32_t finalScore() const { return finalScore_; }
    uint32_t totalFrames() const { return totalFrames_; }
    const std::string& gameVersion() const { return gameVersion_; }
    const std::vector<ReplayEvent>& events() const { return events_; }
    
    /**
     * @brief Serializa para formato binário
     */
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        
        // Header
        appendU32(data, 0x534E5250);  // MAGIC "SNRP"
        data.push_back(1);            // VERSION
        appendU32(data, seed_);
        data.push_back(config_.boardWidth());
        data.push_back(config_.boardHeight());
        data.push_back(static_cast<uint8_t>(config_.difficulty()));
        appendU32(data, finalScore_);
        appendU32(data, totalFrames_);
        
        // Game version (length-prefixed string)
        data.push_back(static_cast<uint8_t>(gameVersion_.size()));
        for (char c : gameVersion_) {
            data.push_back(static_cast<uint8_t>(c));
        }
        
        // Events
        appendU16(data, static_cast<uint16_t>(events_.size()));
        for (const auto& event : events_) {
            appendU32(data, event.frame);
            data.push_back(static_cast<uint8_t>(event.type));
            
            if (event.type == GameEventType::DirectionChanged) {
                data.push_back(event.data.directionValue);
            } else if (event.type == GameEventType::FoodSpawned) {
                appendI16(data, event.data.position.x);
                appendI16(data, event.data.position.y);
            }
        }
        
        return data;
    }
    
    /**
     * @brief Deserializa do formato binário
     */
    static ReplayData deserialize(const std::vector<uint8_t>& data) {
        ReplayData replay;
        size_t pos = 0;
        
        if (data.size() < 20) return replay; // Tamanho mínimo
        
        // Header
        uint32_t magic = readU32(data, pos);
        if (magic != 0x534E5250) return replay;  // MAGIC "SNRP"
        
        uint8_t version = data[pos++];
        if (version != 1) return replay;  // VERSION
        
        replay.seed_ = readU32(data, pos);
        uint8_t width = data[pos++];
        uint8_t height = data[pos++];
        Difficulty diff = static_cast<Difficulty>(data[pos++]);
        replay.config_ = GameConfig::Builder()
            .boardSize(width, height)
            .difficulty(diff)
            .build();
        
        replay.finalScore_ = readU32(data, pos);
        replay.totalFrames_ = readU32(data, pos);
        
        // Game version
        uint8_t versionLen = data[pos++];
        replay.gameVersion_.reserve(versionLen);
        for (uint8_t i = 0; i < versionLen && pos < data.size(); ++i) {
            replay.gameVersion_ += static_cast<char>(data[pos++]);
        }
        
        // Events
        uint16_t eventCount = readU16(data, pos);
        replay.events_.reserve(eventCount);
        
        for (uint16_t i = 0; i < eventCount && pos < data.size(); ++i) {
            ReplayEvent event;
            event.frame = readU32(data, pos);
            event.type = static_cast<GameEventType>(data[pos++]);
            
            if (event.type == GameEventType::DirectionChanged) {
                event.data.directionValue = data[pos++];
            } else if (event.type == GameEventType::FoodSpawned) {
                event.data.position.x = readI16(data, pos);
                event.data.position.y = readI16(data, pos);
            }
            
            replay.events_.push_back(event);
        }
        
        return replay;
    }
    
    /**
     * @brief Codifica em Base64
     */
    std::string toBase64() const {
        static const char* chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        auto data = serialize();
        std::string result;
        result.reserve((data.size() + 2) / 3 * 4);
        
        for (size_t i = 0; i < data.size(); i += 3) {
            uint32_t n = data[i] << 16;
            if (i + 1 < data.size()) n |= data[i + 1] << 8;
            if (i + 2 < data.size()) n |= data[i + 2];
            
            result += chars[(n >> 18) & 0x3F];
            result += chars[(n >> 12) & 0x3F];
            result += (i + 1 < data.size()) ? chars[(n >> 6) & 0x3F] : '=';
            result += (i + 2 < data.size()) ? chars[n & 0x3F] : '=';
        }
        
        return result;
    }
    
    /**
     * @brief Decodifica de Base64
     */
    static ReplayData fromBase64(const std::string& encoded) {
        static const int8_t decode[] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
            -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
        };
        
        std::vector<uint8_t> data;
        data.reserve(encoded.size() * 3 / 4);
        
        uint32_t n = 0;
        int bits = 0;
        
        for (char c : encoded) {
            if (c == '=') break;
            if (static_cast<unsigned char>(c) >= 128) continue;
            int8_t val = decode[static_cast<unsigned char>(c)];
            if (val < 0) continue;
            
            n = (n << 6) | val;
            bits += 6;
            
            if (bits >= 8) {
                bits -= 8;
                data.push_back(static_cast<uint8_t>((n >> bits) & 0xFF));
            }
        }
        
        return deserialize(data);
    }

private:
    static void appendU32(std::vector<uint8_t>& v, uint32_t val) {
        v.push_back((val >> 24) & 0xFF);
        v.push_back((val >> 16) & 0xFF);
        v.push_back((val >> 8) & 0xFF);
        v.push_back(val & 0xFF);
    }
    
    static void appendU16(std::vector<uint8_t>& v, uint16_t val) {
        v.push_back((val >> 8) & 0xFF);
        v.push_back(val & 0xFF);
    }
    
    static void appendI16(std::vector<uint8_t>& v, int16_t val) {
        appendU16(v, static_cast<uint16_t>(val));
    }
    
    static uint32_t readU32(const std::vector<uint8_t>& v, size_t& pos) {
        uint32_t val = (static_cast<uint32_t>(v[pos]) << 24) |
                       (static_cast<uint32_t>(v[pos+1]) << 16) |
                       (static_cast<uint32_t>(v[pos+2]) << 8) |
                       static_cast<uint32_t>(v[pos+3]);
        pos += 4;
        return val;
    }
    
    static uint16_t readU16(const std::vector<uint8_t>& v, size_t& pos) {
        uint16_t val = (static_cast<uint16_t>(v[pos]) << 8) |
                       static_cast<uint16_t>(v[pos+1]);
        pos += 2;
        return val;
    }
    
    static int16_t readI16(const std::vector<uint8_t>& v, size_t& pos) {
        return static_cast<int16_t>(readU16(v, pos));
    }
    
    uint32_t seed_;
    GameConfig config_;
    uint32_t finalScore_;
    uint32_t totalFrames_;
    std::string gameVersion_;
    std::vector<ReplayEvent> events_;
};

/**
 * @brief Gravador de replay
 */
class ReplayRecorder {
public:
    ReplayRecorder() : recording_(false), seed_(0) {}
    
    void startRecording(uint32_t seed, const GameConfig& config) {
        data_.clear();
        data_.setSeed(seed);
        data_.setConfig(config);
        data_.setGameVersion("2.0.0");
        seed_ = seed;
        recording_ = true;
    }
    
    void recordEvent(const GameEvent& event) {
        if (!recording_) return;
        
        if (event.type == GameEventType::DirectionChanged) {
            data_.addEvent(ReplayEvent::directionChange(event.frame, event.direction));
        } else if (event.type == GameEventType::FoodSpawned) {
            data_.addEvent(ReplayEvent::foodSpawn(event.frame, event.position));
        }
    }
    
    void stopRecording(uint32_t finalScore, uint32_t totalFrames) {
        data_.setFinalScore(finalScore);
        data_.setTotalFrames(totalFrames);
        recording_ = false;
    }
    
    const ReplayData& getData() const { return data_; }
    bool isRecording() const { return recording_; }

private:
    ReplayData data_;
    bool recording_;
    uint32_t seed_;
};

/**
 * @brief Validador de replay (re-simula o jogo)
 */
class ReplayValidator {
public:
    struct ValidationResult {
        bool valid;
        uint32_t simulatedScore;
        uint32_t claimedScore;
        std::string errorMessage;
    };
    
    static ValidationResult validate(const ReplayData& replay) {
        ValidationResult result;
        result.claimedScore = replay.finalScore();
        result.valid = false;
        
        // Cria jogo com mesma config
        Game game(replay.config());
        game.start(replay.seed());
        
        // Processa eventos
        size_t eventIndex = 0;
        const auto& events = replay.events();
        
        while (game.state() == GameState::Playing) {
            // Processa eventos deste frame
            while (eventIndex < events.size() && 
                   events[eventIndex].frame == game.frameCount()) {
                
                const auto& event = events[eventIndex];
                if (event.type == GameEventType::DirectionChanged) {
                    Direction dir = static_cast<Direction>(event.data.directionValue);
                    game.handleInput(dir);
                }
                ++eventIndex;
            }
            
            // Atualiza jogo
            if (!game.update()) break;
            
            // Limite de segurança
            if (game.frameCount() > replay.totalFrames() + 100) {
                result.errorMessage = "Replay exceeded frame limit";
                return result;
            }
        }
        
        result.simulatedScore = game.score();
        
        if (result.simulatedScore == result.claimedScore) {
            result.valid = true;
        } else {
            result.errorMessage = "Score mismatch: simulated " + 
                std::to_string(result.simulatedScore) + 
                " vs claimed " + std::to_string(result.claimedScore);
        }
        
        return result;
    }
};

} // namespace Domain
} // namespace Snake

#endif // DOMAIN_SERVICES_REPLAY_SERVICE_HPP

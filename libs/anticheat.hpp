#ifndef ANTICHEAT_H_
#define ANTICHEAT_H_

#include <vector>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>

class AntiCheat {

private:
    std::vector<long> reactionTimes;       // Time between inputs (ms)
    std::vector<int> directionSequence;    // Sequence of directions
    std::chrono::steady_clock::time_point gameStartTime;
    std::chrono::steady_clock::time_point lastInputTime;
    int totalMoves;
    int foodEaten;
    int score;
    int difficulty;
    bool firstInput;
    
    // Secret salt for hashing (in production, this should be more secure)
    const std::string SECRET_SALT = "TSnake_v1.4_AntiCheat_Salt_2025";
    
    double calculateMean(const std::vector<long>& values) const {
        if (values.empty()) return 0.0;
        double sum = 0.0;
        for (long v : values) {
            sum += static_cast<double>(v);
        }
        return sum / static_cast<double>(values.size());
    }
    
    double calculateVariance(const std::vector<long>& values) const {
        if (values.size() < 2) return 0.0;
        double mean = calculateMean(values);
        double variance = 0.0;
        for (long v : values) {
            double diff = static_cast<double>(v) - mean;
            variance += diff * diff;
        }
        return variance / static_cast<double>(values.size() - 1);
    }
    
    double calculateStdDev(const std::vector<long>& values) const {
        return std::sqrt(calculateVariance(values));
    }
    
    std::string sha256(const std::string& input) const {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        
        // Use EVP interface (modern OpenSSL 3.0+)
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
        EVP_DigestUpdate(ctx, input.c_str(), input.length());
        unsigned int len = 0;
        EVP_DigestFinal_ex(ctx, hash, &len);
        EVP_MD_CTX_free(ctx);
        
        std::ostringstream oss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return oss.str();
    }

public:
    AntiCheat() : totalMoves(0), foodEaten(0), score(0), difficulty(1), firstInput(true) {
        reset();
    }
    
    void reset() {
        reactionTimes.clear();
        directionSequence.clear();
        gameStartTime = std::chrono::steady_clock::now();
        lastInputTime = gameStartTime;
        totalMoves = 0;
        foodEaten = 0;
        score = 0;
        firstInput = true;
    }
    
    void setDifficulty(int diff) {
        difficulty = diff;
    }
    
    void recordInput(int direction) {
        auto now = std::chrono::steady_clock::now();
        
        if (!firstInput) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputTime).count();
            reactionTimes.push_back(elapsed);
        }
        firstInput = false;
        
        lastInputTime = now;
        directionSequence.push_back(direction);
        totalMoves++;
    }
    
    void recordFoodEaten() {
        foodEaten++;
    }
    
    void setScore(int s) {
        score = s;
    }
    
    long getGameDuration() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
    }
    
    // Validation result structure
    struct ValidationResult {
        bool valid;
        std::string reason;
        int confidenceScore;  // 0-100, higher is more confident it's legitimate
    };
    
    ValidationResult validate() const {
        ValidationResult result = {true, "OK", 100};
        
        // Need at least some data to validate
        if (reactionTimes.size() < 5) {
            result.confidenceScore = 50;
            return result;
        }
        
        double avgReaction = calculateMean(reactionTimes);
        double stdDev = calculateStdDev(reactionTimes);
        long duration = getGameDuration();
        
        // Check 1: Superhuman reaction times (< 30ms average)
        if (avgReaction < 30) {
            result.valid = false;
            result.reason = "Reaction time too fast";
            result.confidenceScore = 0;
            return result;
        }
        
        // Check 2: Perfect consistency (variance too low = automation)
        // Human inputs have natural variance
        if (reactionTimes.size() > 10 && stdDev < 10) {
            result.valid = false;
            result.reason = "Input pattern too consistent";
            result.confidenceScore = 10;
            return result;
        }
        
        // Check 3: Score impossible for game duration
        // Max theoretical score per second depends on difficulty
        int maxScorePerSecond = difficulty * 2;
        int maxPossibleScore = static_cast<int>(duration) * maxScorePerSecond;
        if (score > maxPossibleScore && duration > 10) {
            result.valid = false;
            result.reason = "Score too high for duration";
            result.confidenceScore = 5;
            return result;
        }
        
        // Check 4: Food eaten vs moves ratio
        // It's impossible to eat food with less than ~3 moves per food on average
        if (foodEaten > 0 && totalMoves > 0) {
            double movesPerFood = static_cast<double>(totalMoves) / static_cast<double>(foodEaten);
            if (movesPerFood < 2) {
                result.valid = false;
                result.reason = "Movement pattern impossible";
                result.confidenceScore = 5;
                return result;
            }
        }
        
        // Check 5: Game too long (> 1 hour = likely automated)
        if (duration > 3600) {
            result.confidenceScore = std::max(0, result.confidenceScore - 30);
            result.reason = "Extended play session";
        }
        
        // Adjust confidence based on metrics
        if (avgReaction < 100) {
            result.confidenceScore = std::max(0, result.confidenceScore - 20);
        }
        if (stdDev < 50 && reactionTimes.size() > 20) {
            result.confidenceScore = std::max(0, result.confidenceScore - 15);
        }
        
        return result;
    }
    
    // Generate a hash of the game session for server-side verification
    std::string generateSessionHash() const {
        std::ostringstream oss;
        oss << score << "|"
            << foodEaten << "|"
            << totalMoves << "|"
            << getGameDuration() << "|"
            << difficulty << "|"
            << reactionTimes.size() << "|"
            << static_cast<long>(calculateMean(reactionTimes)) << "|"
            << SECRET_SALT;
        
        return sha256(oss.str());
    }
    
    // Get session data for submission
    struct SessionData {
        int score;
        int foodEaten;
        int totalMoves;
        long duration;
        int difficulty;
        double avgReactionTime;
        double reactionStdDev;
        std::string sessionHash;
        int confidenceScore;
    };
    
    SessionData getSessionData() const {
        SessionData data;
        data.score = score;
        data.foodEaten = foodEaten;
        data.totalMoves = totalMoves;
        data.duration = getGameDuration();
        data.difficulty = difficulty;
        data.avgReactionTime = calculateMean(reactionTimes);
        data.reactionStdDev = calculateStdDev(reactionTimes);
        data.sessionHash = generateSessionHash();
        data.confidenceScore = validate().confidenceScore;
        return data;
    }
};

#endif

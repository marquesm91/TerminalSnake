/**
 * @file firebase_leaderboard_adapter.hpp
 * @brief Firebase Leaderboard Adapter
 * 
 * Clean Architecture: Infrastructure Layer - Adapter
 * Implements ILeaderboard port for Firebase Firestore
 */

#ifndef INFRASTRUCTURE_EXTERNAL_FIREBASE_LEADERBOARD_ADAPTER_HPP
#define INFRASTRUCTURE_EXTERNAL_FIREBASE_LEADERBOARD_ADAPTER_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <iomanip>

#include "application/ports/leaderboard_port.hpp"
#include "application/ports/auth_port.hpp"
#include "domain/services/replay_service.hpp"

namespace Snake {
namespace Infrastructure {
namespace External {

/**
 * @brief Firebase Leaderboard Adapter
 * 
 * Communicates with Firebase Firestore to:
 * - Fetch top scores
 * - Submit scores with replay data for validation
 * - Get user rank
 */
class FirebaseLeaderboardAdapter : public Application::Ports::ILeaderboard {
public:
    /**
     * @brief Constructor
     * @param auth Reference to auth adapter for tokens
     * @param projectId Firebase project ID
     */
    FirebaseLeaderboardAdapter(Application::Ports::IAuth& auth, 
                               const std::string& projectId = "terminalsnake-leaderboard")
        : auth_(auth), projectId_(projectId), online_(false) {
        firestoreUrl_ = "https://firestore.googleapis.com/v1/projects/" + 
                        projectId_ + "/databases/(default)/documents";
        functionsUrl_ = "https://us-central1-" + projectId_ + ".cloudfunctions.net";
    }
    
    ~FirebaseLeaderboardAdapter() override = default;
    
    /**
     * @brief Fetch top N leaderboard entries
     */
    std::vector<Application::Ports::LeaderboardEntry> fetchTop(uint32_t limit = 20) override {
        std::vector<Application::Ports::LeaderboardEntry> entries;
        
        if (!auth_.isAuthenticated()) {
            online_ = false;
            return entries;
        }
        
        // Build Firestore query URL
        std::string url = firestoreUrl_ + "/leaderboard?orderBy=score desc&pageSize=" + 
                         std::to_string(limit);
        
        auto response = httpGet(url, "Bearer " + auth_.getAccessToken());
        
        if (!response.success) {
            online_ = false;
            return entries;
        }
        
        online_ = true;
        
        // Parse Firestore response
        // Note: Real implementation would use proper JSON parsing
        entries = parseLeaderboardResponse(response.body);
        
        return entries;
    }
    
    /**
     * @brief Submit score with replay for server-side validation
     */
    Application::Ports::SubmitResult submitScore(
        const Domain::ReplayData& replay, 
        uint16_t snakeSize) override {
        
        Application::Ports::SubmitResult result;
        result.success = false;
        result.rank = -1;
        
        if (!auth_.isAuthenticated()) {
            result.errorMessage = "Not authenticated";
            return result;
        }
        
        // Refresh token if needed
        if (!auth_.refreshTokenIfNeeded()) {
            result.errorMessage = "Failed to refresh authentication token";
            return result;
        }
        
        // Serialize replay to Base64
        std::string replayData = replay.toBase64();
        
        // Build request body
        std::ostringstream body;
        body << "{"
             << "\"replayData\":\"" << replayData << "\","
             << "\"snakeSize\":" << snakeSize << ","
             << "\"claimedScore\":" << replay.finalScore() << ","
             << "\"difficulty\":\"" << replay.config().difficultyString() << "\""
             << "}";
        
        // Submit to Cloud Function for validation
        std::string url = functionsUrl_ + "/submitScore";
        auto response = httpPost(url, body.str(), "Bearer " + auth_.getAccessToken());
        
        if (!response.success) {
            result.errorMessage = "Failed to submit score";
            online_ = false;
            return result;
        }
        
        online_ = true;
        
        // Parse response
        result.success = parseJsonBool(response.body, "success");
        result.errorMessage = parseJsonString(response.body, "error");
        result.rank = parseJsonInt(response.body, "rank");
        
        return result;
    }
    
    /**
     * @brief Get user's current rank
     */
    int getUserRank(const std::string& odUserId) override {
        if (!auth_.isAuthenticated()) {
            return -1;
        }
        
        // Query Firestore for user's rank
        std::string url = functionsUrl_ + "/getUserRank?userId=" + urlEncode(odUserId);
        auto response = httpGet(url, "Bearer " + auth_.getAccessToken());
        
        if (!response.success) {
            return -1;
        }
        
        return parseJsonInt(response.body, "rank");
    }
    
    /**
     * @brief Check if online
     */
    bool isOnline() const override {
        return online_;
    }
    
    /**
     * @brief Manually check connection
     */
    bool checkConnection() {
        if (!auth_.isAuthenticated()) {
            online_ = false;
            return false;
        }
        
        // Simple ping to Firestore
        std::string url = firestoreUrl_ + "/leaderboard?pageSize=1";
        auto response = httpGet(url, "Bearer " + auth_.getAccessToken());
        
        online_ = response.success;
        return online_;
    }

private:
    Application::Ports::IAuth& auth_;
    std::string projectId_;
    std::string firestoreUrl_;
    std::string functionsUrl_;
    bool online_;
    
    /**
     * @brief HTTP response structure
     */
    struct HttpResponse {
        bool success;
        std::string body;
        int statusCode;
    };
    
    /**
     * @brief Make HTTP GET request (stub - implement with libcurl)
     */
    HttpResponse httpGet(const std::string& url, const std::string& authHeader) {
        HttpResponse response;
        response.success = false;
        response.statusCode = 0;
        
        // In production, use libcurl:
        // CURL* curl = curl_easy_init();
        // curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // ...
        
        (void)url;
        (void)authHeader;
        
        return response;
    }
    
    /**
     * @brief Make HTTP POST request (stub - implement with libcurl)
     */
    HttpResponse httpPost(const std::string& url, const std::string& body, 
                         const std::string& authHeader) {
        HttpResponse response;
        response.success = false;
        response.statusCode = 0;
        
        (void)url;
        (void)body;
        (void)authHeader;
        
        return response;
    }
    
    /**
     * @brief URL encode a string
     */
    std::string urlEncode(const std::string& str) {
        std::string encoded;
        for (char c : str) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded += c;
            } else if (c == ' ') {
                encoded += '+';
            } else {
                char hex[4];
                std::snprintf(hex, sizeof(hex), "%%%02X", static_cast<unsigned char>(c));
                encoded += hex;
            }
        }
        return encoded;
    }
    
    /**
     * @brief Parse JSON string value
     */
    std::string parseJsonString(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":\"";
        size_t start = json.find(search);
        if (start == std::string::npos) return "";
        
        start += search.length();
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        
        return json.substr(start, end - start);
    }
    
    /**
     * @brief Parse JSON integer value
     */
    int parseJsonInt(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t start = json.find(search);
        if (start == std::string::npos) return 0;
        
        start += search.length();
        while (start < json.length() && !std::isdigit(json[start]) && json[start] != '-') ++start;
        
        bool negative = false;
        if (json[start] == '-') {
            negative = true;
            ++start;
        }
        
        int value = 0;
        while (start < json.length() && std::isdigit(json[start])) {
            value = value * 10 + (json[start] - '0');
            ++start;
        }
        
        return negative ? -value : value;
    }
    
    /**
     * @brief Parse JSON boolean value
     */
    bool parseJsonBool(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t start = json.find(search);
        if (start == std::string::npos) return false;
        
        start += search.length();
        while (start < json.length() && std::isspace(json[start])) ++start;
        
        return json.substr(start, 4) == "true";
    }
    
    /**
     * @brief Parse Firestore leaderboard response
     */
    std::vector<Application::Ports::LeaderboardEntry> parseLeaderboardResponse(
        const std::string& json) {
        
        std::vector<Application::Ports::LeaderboardEntry> entries;
        
        // Parse Firestore documents array
        // Real implementation would use proper JSON parser
        // This is a simplified stub
        
        // Look for "documents" array
        size_t pos = json.find("\"documents\"");
        if (pos == std::string::npos) return entries;
        
        // Extract each document
        size_t docStart = json.find("{\"name\":", pos);
        while (docStart != std::string::npos) {
            size_t docEnd = json.find("{\"name\":", docStart + 1);
            if (docEnd == std::string::npos) docEnd = json.length();
            
            std::string doc = json.substr(docStart, docEnd - docStart);
            
            Application::Ports::LeaderboardEntry entry;
            entry.odUserId = extractFirestoreField(doc, "userId", "stringValue");
            entry.displayName = extractFirestoreField(doc, "displayName", "stringValue");
            entry.photoUrl = extractFirestoreField(doc, "photoUrl", "stringValue");
            entry.difficulty = extractFirestoreField(doc, "difficulty", "stringValue");
            entry.timestamp = extractFirestoreField(doc, "timestamp", "timestampValue");
            
            std::string scoreStr = extractFirestoreField(doc, "score", "integerValue");
            entry.score = scoreStr.empty() ? 0 : static_cast<uint32_t>(std::stoul(scoreStr));
            
            std::string sizeStr = extractFirestoreField(doc, "snakeSize", "integerValue");
            entry.snakeSize = sizeStr.empty() ? 0 : static_cast<uint16_t>(std::stoul(sizeStr));
            
            std::string verifiedStr = extractFirestoreField(doc, "verified", "booleanValue");
            entry.verified = verifiedStr == "true";
            
            entries.push_back(entry);
            
            docStart = json.find("{\"name\":", docEnd);
        }
        
        return entries;
    }
    
    /**
     * @brief Extract Firestore field value
     */
    std::string extractFirestoreField(const std::string& doc, 
                                      const std::string& fieldName,
                                      const std::string& valueType) {
        // Firestore format: "fieldName":{"stringValue":"value"}
        std::string search = "\"" + fieldName + "\":{\"" + valueType + "\":";
        size_t start = doc.find(search);
        if (start == std::string::npos) return "";
        
        start += search.length();
        
        if (valueType == "stringValue" || valueType == "timestampValue") {
            // String value in quotes
            if (doc[start] != '\"') return "";
            ++start;
            size_t end = doc.find("\"", start);
            if (end == std::string::npos) return "";
            return doc.substr(start, end - start);
        } else {
            // Integer or boolean (no quotes)
            size_t end = doc.find("}", start);
            if (end == std::string::npos) return "";
            std::string value = doc.substr(start, end - start);
            // Remove quotes if present
            if (!value.empty() && value[0] == '\"') {
                value = value.substr(1, value.length() - 2);
            }
            return value;
        }
    }
};

} // namespace External
} // namespace Infrastructure
} // namespace Snake

#endif // INFRASTRUCTURE_EXTERNAL_FIREBASE_LEADERBOARD_ADAPTER_HPP

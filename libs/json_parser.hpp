#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>

// Simple JSON parser for our needs (no external dependencies)
// For production, consider using nlohmann/json or rapidjson
class JsonParser {

public:
    // Parse a simple JSON object and return key-value pairs
    static std::map<std::string, std::string> parse(const std::string& json) {
        std::map<std::string, std::string> result;
        
        size_t pos = 0;
        while ((pos = json.find("\"", pos)) != std::string::npos) {
            // Find key
            size_t keyStart = pos + 1;
            size_t keyEnd = json.find("\"", keyStart);
            if (keyEnd == std::string::npos) break;
            
            std::string key = json.substr(keyStart, keyEnd - keyStart);
            
            // Find colon
            size_t colonPos = json.find(":", keyEnd);
            if (colonPos == std::string::npos) break;
            
            // Find value
            size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
            if (valueStart == std::string::npos) break;
            
            std::string value;
            if (json[valueStart] == '"') {
                // String value
                size_t valueEnd = json.find("\"", valueStart + 1);
                if (valueEnd == std::string::npos) break;
                value = json.substr(valueStart + 1, valueEnd - valueStart - 1);
                pos = valueEnd + 1;
            } else if (json[valueStart] == '{' || json[valueStart] == '[') {
                // Nested object or array - skip for simple parser
                pos = valueStart + 1;
                continue;
            } else {
                // Number or boolean
                size_t valueEnd = json.find_first_of(",}\n", valueStart);
                if (valueEnd == std::string::npos) valueEnd = json.length();
                value = json.substr(valueStart, valueEnd - valueStart);
                // Trim whitespace
                while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || 
                       value.back() == '\n' || value.back() == '\r')) {
                    value.pop_back();
                }
                pos = valueEnd;
            }
            
            result[key] = value;
        }
        
        return result;
    }
    
    // Get a string value from JSON
    static std::string getString(const std::string& json, const std::string& key) {
        auto parsed = parse(json);
        auto it = parsed.find(key);
        return (it != parsed.end()) ? it->second : "";
    }
    
    // Get an integer value from JSON
    static int getInt(const std::string& json, const std::string& key) {
        std::string value = getString(json, key);
        if (value.empty()) return 0;
        try {
            return std::stoi(value);
        } catch (...) {
            return 0;
        }
    }
    
    // Create a simple JSON object
    static std::string createObject(const std::map<std::string, std::string>& data) {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& pair : data) {
            if (!first) oss << ",";
            oss << "\"" << pair.first << "\":\"" << pair.second << "\"";
            first = false;
        }
        oss << "}";
        return oss.str();
    }
    
    // Create Firestore document format
    static std::string createFirestoreDocument(const std::map<std::string, std::string>& stringFields,
                                                const std::map<std::string, int>& intFields) {
        std::ostringstream oss;
        oss << "{\"fields\":{";
        bool first = true;
        
        for (const auto& pair : stringFields) {
            if (!first) oss << ",";
            oss << "\"" << pair.first << "\":{\"stringValue\":\"" << pair.second << "\"}";
            first = false;
        }
        
        for (const auto& pair : intFields) {
            if (!first) oss << ",";
            oss << "\"" << pair.first << "\":{\"integerValue\":\"" << pair.second << "\"}";
            first = false;
        }
        
        oss << "}}";
        return oss.str();
    }
    
    // Parse Firestore leaderboard response - returns raw data that caller will convert
    struct RawLeaderboardEntry {
        std::string odUserId;
        std::string displayName;
        int highscore;
        int snakeSize;
        std::string difficulty;
        std::string timestamp;
        int confidenceScore;
    };
    
    static std::vector<RawLeaderboardEntry> parseLeaderboard(const std::string& json) {
        std::vector<RawLeaderboardEntry> entries;
        // Simplified parsing - in production use a proper JSON library
        // This is a placeholder for the actual implementation
        // Parse "documents" array from Firestore response
        (void)json; // Suppress unused parameter warning
        return entries;
    }
};

#endif

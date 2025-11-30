/**
 * @file firebase_auth_adapter.hpp
 * @brief Firebase Authentication Adapter
 * 
 * Clean Architecture: Infrastructure Layer - Adapter
 * Implements IAuth port for Google OAuth 2.0 Device Flow
 */

#ifndef INFRASTRUCTURE_EXTERNAL_FIREBASE_AUTH_ADAPTER_HPP
#define INFRASTRUCTURE_EXTERNAL_FIREBASE_AUTH_ADAPTER_HPP

#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "application/ports/auth_port.hpp"

namespace Snake {
namespace Infrastructure {
namespace External {

/**
 * @brief Firebase configuration
 */
struct FirebaseConfig {
    std::string clientId;
    std::string projectId;
    std::string deviceCodeUrl = "https://oauth2.googleapis.com/device/code";
    std::string tokenUrl = "https://oauth2.googleapis.com/token";
    std::string userInfoUrl = "https://www.googleapis.com/oauth2/v2/userinfo";
    std::string scopes = "openid email profile";
    int pollIntervalSeconds = 5;
    std::string tokenFilePath;
    
    static FirebaseConfig defaultConfig() {
        FirebaseConfig config;
        config.clientId = "YOUR_CLIENT_ID.apps.googleusercontent.com";
        config.projectId = "terminalsnake-leaderboard";
        
        // Get home directory for token storage
        const char* home = std::getenv("HOME");
        if (home) {
            config.tokenFilePath = std::string(home) + "/.tsnake_auth";
        } else {
            config.tokenFilePath = ".tsnake_auth";
        }
        
        return config;
    }
};

/**
 * @brief Device Flow response
 */
struct DeviceFlowResponse {
    std::string deviceCode;
    std::string userCode;
    std::string verificationUrl;
    int expiresIn;
    int interval;
};

/**
 * @brief Token response
 */
struct TokenData {
    std::string accessToken;
    std::string refreshToken;
    std::string idToken;
    int expiresIn;
    std::chrono::system_clock::time_point expiresAt;
};

/**
 * @brief Firebase Authentication Adapter
 * 
 * Implements Google OAuth 2.0 Device Flow for terminal environments.
 * Tokens are persisted to disk for session continuity.
 */
class FirebaseAuthAdapter : public Application::Ports::IAuth {
public:
    explicit FirebaseAuthAdapter(const FirebaseConfig& config = FirebaseConfig::defaultConfig())
        : config_(config), authenticated_(false) {
        loadStoredTokens();
    }
    
    ~FirebaseAuthAdapter() override = default;
    
    /**
     * @brief Check if user is authenticated
     */
    bool isAuthenticated() const override {
        if (!authenticated_) return false;
        
        // Check if token is expired
        auto now = std::chrono::system_clock::now();
        return now < tokens_.expiresAt;
    }
    
    /**
     * @brief Start OAuth Device Flow
     */
    Application::Ports::AuthResult startDeviceFlow() override {
        Application::Ports::AuthResult result;
        result.success = false;
        
        // Build request body
        std::string body = "client_id=" + config_.clientId + 
                          "&scope=" + urlEncode(config_.scopes);
        
        // Make HTTP request (simplified - would use libcurl in production)
        auto response = httpPost(config_.deviceCodeUrl, body);
        
        if (!response.success) {
            result.errorMessage = "Failed to initiate device flow";
            return result;
        }
        
        // Parse response
        deviceFlow_.deviceCode = parseJsonString(response.body, "device_code");
        deviceFlow_.userCode = parseJsonString(response.body, "user_code");
        deviceFlow_.verificationUrl = parseJsonString(response.body, "verification_url");
        deviceFlow_.expiresIn = parseJsonInt(response.body, "expires_in");
        deviceFlow_.interval = parseJsonInt(response.body, "interval");
        
        if (deviceFlow_.userCode.empty()) {
            result.errorMessage = "Invalid device flow response";
            return result;
        }
        
        result.success = true;
        // Store verification URL and code in result for display
        result.errorMessage = deviceFlow_.verificationUrl + " | Code: " + deviceFlow_.userCode;
        return result;
    }
    
    /**
     * @brief Wait for user to complete authentication
     */
    Application::Ports::AuthResult waitForAuth() override {
        Application::Ports::AuthResult result;
        result.success = false;
        
        if (deviceFlow_.deviceCode.empty()) {
            result.errorMessage = "Device flow not initiated";
            return result;
        }
        
        // Poll for token
        int maxAttempts = deviceFlow_.expiresIn / deviceFlow_.interval;
        std::string body = "client_id=" + config_.clientId +
                          "&device_code=" + deviceFlow_.deviceCode +
                          "&grant_type=urn:ietf:params:oauth:grant-type:device_code";
        
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            std::this_thread::sleep_for(
                std::chrono::seconds(deviceFlow_.interval)
            );
            
            auto response = httpPost(config_.tokenUrl, body);
            
            if (response.success) {
                std::string accessToken = parseJsonString(response.body, "access_token");
                if (!accessToken.empty()) {
                    // Success - got tokens
                    tokens_.accessToken = accessToken;
                    tokens_.refreshToken = parseJsonString(response.body, "refresh_token");
                    tokens_.idToken = parseJsonString(response.body, "id_token");
                    tokens_.expiresIn = parseJsonInt(response.body, "expires_in");
                    tokens_.expiresAt = std::chrono::system_clock::now() + 
                                       std::chrono::seconds(tokens_.expiresIn);
                    
                    // Fetch user info
                    fetchUserInfo();
                    
                    // Save tokens
                    saveTokens();
                    
                    authenticated_ = true;
                    result.success = true;
                    result.user = userInfo_;
                    return result;
                }
            }
            
            // Check for specific errors
            std::string error = parseJsonString(response.body, "error");
            if (error == "access_denied" || error == "expired_token") {
                result.errorMessage = "Authentication denied or expired";
                return result;
            }
            // "authorization_pending" means keep polling
        }
        
        result.errorMessage = "Authentication timed out";
        return result;
    }
    
    /**
     * @brief Logout and clear tokens
     */
    void logout() override {
        tokens_ = TokenData();
        userInfo_ = Application::Ports::UserInfo();
        authenticated_ = false;
        
        // Remove token file
        std::remove(config_.tokenFilePath.c_str());
    }
    
    /**
     * @brief Get current user info
     */
    Application::Ports::UserInfo getCurrentUser() const override {
        return userInfo_;
    }
    
    /**
     * @brief Get access token for API calls
     */
    std::string getAccessToken() const override {
        return tokens_.accessToken;
    }
    
    /**
     * @brief Refresh token if needed
     */
    bool refreshTokenIfNeeded() override {
        if (!authenticated_) return false;
        
        auto now = std::chrono::system_clock::now();
        auto fiveMinutes = std::chrono::minutes(5);
        
        // Refresh if expiring within 5 minutes
        if (now + fiveMinutes >= tokens_.expiresAt) {
            return refreshAccessToken();
        }
        
        return true;
    }
    
    /**
     * @brief Get verification URL for display
     */
    std::string getVerificationUrl() const {
        return deviceFlow_.verificationUrl;
    }
    
    /**
     * @brief Get user code for display
     */
    std::string getUserCode() const {
        return deviceFlow_.userCode;
    }

private:
    FirebaseConfig config_;
    bool authenticated_;
    TokenData tokens_;
    DeviceFlowResponse deviceFlow_;
    Application::Ports::UserInfo userInfo_;
    
    /**
     * @brief HTTP response structure
     */
    struct HttpResponse {
        bool success;
        std::string body;
    };
    
    /**
     * @brief Make HTTP POST request (stub - implement with libcurl)
     */
    HttpResponse httpPost(const std::string& url, const std::string& body) {
        HttpResponse response;
        response.success = false;
        
        // This would use libcurl in production
        // For now, return stub response
        (void)url;
        (void)body;
        
        return response;
    }
    
    /**
     * @brief Make HTTP GET request (stub - implement with libcurl)
     */
    HttpResponse httpGet(const std::string& url, const std::string& authHeader) {
        HttpResponse response;
        response.success = false;
        
        (void)url;
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
     * @brief Parse JSON string value (simple implementation)
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
        while (start < json.length() && !std::isdigit(json[start])) ++start;
        
        int value = 0;
        while (start < json.length() && std::isdigit(json[start])) {
            value = value * 10 + (json[start] - '0');
            ++start;
        }
        
        return value;
    }
    
    /**
     * @brief Fetch user info from Google
     */
    void fetchUserInfo() {
        auto response = httpGet(config_.userInfoUrl, "Bearer " + tokens_.accessToken);
        
        if (response.success) {
            userInfo_.odUserId = parseJsonString(response.body, "id");
            userInfo_.displayName = parseJsonString(response.body, "name");
            userInfo_.email = parseJsonString(response.body, "email");
            userInfo_.photoUrl = parseJsonString(response.body, "picture");
        }
    }
    
    /**
     * @brief Refresh access token
     */
    bool refreshAccessToken() {
        if (tokens_.refreshToken.empty()) return false;
        
        std::string body = "client_id=" + config_.clientId +
                          "&refresh_token=" + tokens_.refreshToken +
                          "&grant_type=refresh_token";
        
        auto response = httpPost(config_.tokenUrl, body);
        
        if (response.success) {
            std::string accessToken = parseJsonString(response.body, "access_token");
            if (!accessToken.empty()) {
                tokens_.accessToken = accessToken;
                tokens_.expiresIn = parseJsonInt(response.body, "expires_in");
                tokens_.expiresAt = std::chrono::system_clock::now() + 
                                   std::chrono::seconds(tokens_.expiresIn);
                saveTokens();
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * @brief Save tokens to disk
     */
    void saveTokens() {
        std::ofstream file(config_.tokenFilePath);
        if (file.is_open()) {
            file << tokens_.accessToken << "\n"
                 << tokens_.refreshToken << "\n"
                 << tokens_.idToken << "\n"
                 << tokens_.expiresIn << "\n"
                 << userInfo_.odUserId << "\n"
                 << userInfo_.displayName << "\n"
                 << userInfo_.email << "\n"
                 << userInfo_.photoUrl << "\n";
            file.close();
        }
    }
    
    /**
     * @brief Load tokens from disk
     */
    void loadStoredTokens() {
        std::ifstream file(config_.tokenFilePath);
        if (file.is_open()) {
            std::getline(file, tokens_.accessToken);
            std::getline(file, tokens_.refreshToken);
            std::getline(file, tokens_.idToken);
            
            std::string expiresStr;
            std::getline(file, expiresStr);
            tokens_.expiresIn = std::stoi(expiresStr.empty() ? "0" : expiresStr);
            
            std::getline(file, userInfo_.odUserId);
            std::getline(file, userInfo_.displayName);
            std::getline(file, userInfo_.email);
            std::getline(file, userInfo_.photoUrl);
            
            file.close();
            
            // Assume token is valid if we have data
            if (!tokens_.accessToken.empty()) {
                authenticated_ = true;
                tokens_.expiresAt = std::chrono::system_clock::now() + 
                                   std::chrono::seconds(tokens_.expiresIn);
            }
        }
    }
};

} // namespace External
} // namespace Infrastructure
} // namespace Snake

#endif // INFRASTRUCTURE_EXTERNAL_FIREBASE_AUTH_ADAPTER_HPP

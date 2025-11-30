#ifndef AUTH_H_
#define AUTH_H_

#include <string>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <ncurses.h>
#include "firebase_config.hpp"
#include "http_client.hpp"
#include "json_parser.hpp"

class Auth {

private:
    std::string accessToken;
    std::string refreshToken;
    std::string userId;
    std::string displayName;
    std::string email;
    std::string photoUrl;
    long tokenExpiry;
    bool authenticated;
    HttpClient http;
    
    std::string getTokenFilePath() {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/" + FirebaseConfig::TOKEN_FILE;
        }
        return FirebaseConfig::TOKEN_FILE;
    }
    
    void saveTokens() {
        std::ofstream file(getTokenFilePath());
        if (file.is_open()) {
            file << accessToken << "\n";
            file << refreshToken << "\n";
            file << userId << "\n";
            file << displayName << "\n";
            file << email << "\n";
            file << photoUrl << "\n";
            file << tokenExpiry << "\n";
            file.close();
        }
    }
    
    bool loadTokens() {
        std::ifstream file(getTokenFilePath());
        if (file.is_open()) {
            std::getline(file, accessToken);
            std::getline(file, refreshToken);
            std::getline(file, userId);
            std::getline(file, displayName);
            std::getline(file, email);
            std::getline(file, photoUrl);
            
            std::string expiryStr;
            std::getline(file, expiryStr);
            if (!expiryStr.empty()) {
                tokenExpiry = std::stol(expiryStr);
            }
            file.close();
            
            if (!accessToken.empty() && !refreshToken.empty()) {
                // Check if token is expired
                auto now = std::chrono::system_clock::now();
                auto epoch = now.time_since_epoch();
                long currentTime = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
                
                if (currentTime >= tokenExpiry) {
                    // Token expired, try to refresh
                    return refreshAccessToken();
                }
                authenticated = true;
                return true;
            }
        }
        return false;
    }
    
    bool refreshAccessToken() {
        std::string postData = "client_id=" + FirebaseConfig::CLIENT_ID +
                               "&refresh_token=" + refreshToken +
                               "&grant_type=refresh_token";
        
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/x-www-form-urlencoded";
        
        auto response = http.post(FirebaseConfig::TOKEN_URL, postData, headers);
        
        if (response.success) {
            accessToken = JsonParser::getString(response.body, "access_token");
            int expiresIn = JsonParser::getInt(response.body, "expires_in");
            
            auto now = std::chrono::system_clock::now();
            auto epoch = now.time_since_epoch();
            tokenExpiry = std::chrono::duration_cast<std::chrono::seconds>(epoch).count() + expiresIn;
            
            saveTokens();
            authenticated = true;
            return true;
        }
        
        return false;
    }
    
    bool fetchUserInfo() {
        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + accessToken;
        
        auto response = http.get(FirebaseConfig::USERINFO_URL, headers);
        
        if (response.success) {
            userId = JsonParser::getString(response.body, "id");
            displayName = JsonParser::getString(response.body, "name");
            email = JsonParser::getString(response.body, "email");
            photoUrl = JsonParser::getString(response.body, "picture");
            return true;
        }
        
        return false;
    }

public:
    Auth() : tokenExpiry(0), authenticated(false) {
        // Try to load existing tokens
        loadTokens();
    }
    
    bool isAuthenticated() const { return authenticated; }
    std::string getAccessToken() const { return accessToken; }
    std::string getUserId() const { return userId; }
    std::string getDisplayName() const { return displayName; }
    std::string getEmail() const { return email; }
    std::string getPhotoUrl() const { return photoUrl; }
    
    // Device Flow Authentication
    // Returns true if authentication successful
    bool authenticateWithDeviceFlow() {
        // Step 1: Request device code
        std::string postData = "client_id=" + FirebaseConfig::CLIENT_ID +
                               "&scope=" + FirebaseConfig::SCOPES;
        
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/x-www-form-urlencoded";
        
        auto response = http.post(FirebaseConfig::DEVICE_CODE_URL, postData, headers);
        
        if (!response.success) {
            return false;
        }
        
        std::string deviceCode = JsonParser::getString(response.body, "device_code");
        std::string userCode = JsonParser::getString(response.body, "user_code");
        std::string verificationUrl = JsonParser::getString(response.body, "verification_url");
        int expiresIn = JsonParser::getInt(response.body, "expires_in");
        int interval = JsonParser::getInt(response.body, "interval");
        if (interval <= 0) interval = FirebaseConfig::POLL_INTERVAL;
        
        // Step 2: Display instructions to user
        clear();
        int centerY = LINES / 2;
        int centerX = COLS / 2;
        
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(centerY - 6, centerX - 15, "=== GOOGLE SIGN-IN ===");
        attroff(COLOR_PAIR(1) | A_BOLD);
        
        attron(COLOR_PAIR(5));
        mvprintw(centerY - 3, centerX - 20, "To sign in, open your browser and go to:");
        attroff(COLOR_PAIR(5));
        
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(centerY - 1, centerX - static_cast<int>(verificationUrl.length()) / 2, "%s", verificationUrl.c_str());
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        attron(COLOR_PAIR(5));
        mvprintw(centerY + 1, centerX - 12, "And enter this code:");
        attroff(COLOR_PAIR(5));
        
        attron(COLOR_PAIR(6) | A_BOLD);
        mvprintw(centerY + 3, centerX - static_cast<int>(userCode.length()) / 2, "%s", userCode.c_str());
        attroff(COLOR_PAIR(6) | A_BOLD);
        
        attron(COLOR_PAIR(1));
        mvprintw(centerY + 6, centerX - 15, "Waiting for authentication...");
        attroff(COLOR_PAIR(1));
        
        mvprintw(centerY + 8, centerX - 10, "Press 'q' to cancel");
        
        refresh();
        
        // Step 3: Poll for token
        timeout(1000); // 1 second timeout for getch
        
        auto startTime = std::chrono::steady_clock::now();
        int pollCount = 0;
        
        while (true) {
            // Check if user wants to cancel
            int ch = getch();
            if (ch == 'q' || ch == 'Q') {
                timeout(-1);
                return false;
            }
            
            // Check timeout
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
            if (elapsed >= expiresIn) {
                timeout(-1);
                return false;
            }
            
            // Poll every interval seconds
            pollCount++;
            if (pollCount >= interval) {
                pollCount = 0;
                
                // Update waiting animation
                std::string dots(static_cast<size_t>((elapsed / interval) % 4), '.');
                mvprintw(centerY + 6, centerX + 15, "%-4s", dots.c_str());
                refresh();
                
                // Try to get token
                std::string tokenData = "client_id=" + FirebaseConfig::CLIENT_ID +
                                        "&device_code=" + deviceCode +
                                        "&grant_type=urn:ietf:params:oauth:grant-type:device_code";
                
                auto tokenResponse = http.post(FirebaseConfig::TOKEN_URL, tokenData, headers);
                
                if (tokenResponse.success) {
                    accessToken = JsonParser::getString(tokenResponse.body, "access_token");
                    refreshToken = JsonParser::getString(tokenResponse.body, "refresh_token");
                    int tokenExpiresIn = JsonParser::getInt(tokenResponse.body, "expires_in");
                    
                    auto nowTime = std::chrono::system_clock::now();
                    auto epoch = nowTime.time_since_epoch();
                    tokenExpiry = std::chrono::duration_cast<std::chrono::seconds>(epoch).count() + tokenExpiresIn;
                    
                    // Fetch user info
                    if (fetchUserInfo()) {
                        saveTokens();
                        authenticated = true;
                        timeout(-1);
                        
                        // Show success message
                        clear();
                        attron(COLOR_PAIR(2) | A_BOLD);
                        mvprintw(centerY - 2, centerX - 10, "Welcome, %s!", displayName.c_str());
                        attroff(COLOR_PAIR(2) | A_BOLD);
                        
                        attron(COLOR_PAIR(5));
                        mvprintw(centerY, centerX - 15, "You are now signed in with Google");
                        mvprintw(centerY + 2, centerX - 12, "Press any key to continue...");
                        attroff(COLOR_PAIR(5));
                        
                        refresh();
                        timeout(-1);
                        getch();
                        
                        return true;
                    }
                }
                
                // Check for specific error
                std::string error = JsonParser::getString(tokenResponse.body, "error");
                if (error != "authorization_pending" && error != "slow_down" && !error.empty()) {
                    timeout(-1);
                    return false;
                }
            }
        }
    }
    
    void logout() {
        accessToken.clear();
        refreshToken.clear();
        userId.clear();
        displayName.clear();
        email.clear();
        photoUrl.clear();
        tokenExpiry = 0;
        authenticated = false;
        
        // Remove token file
        std::remove(getTokenFilePath().c_str());
    }
};

#endif

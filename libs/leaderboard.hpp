#ifndef LEADERBOARD_H_
#define LEADERBOARD_H_

#include <string>
#include <vector>
#include <algorithm>
#include <ncurses.h>
#include "firebase_config.hpp"
#include "http_client.hpp"
#include "json_parser.hpp"
#include "auth.hpp"
#include "anticheat.hpp"
#include "core/replay.hpp"

class Leaderboard {

public:
    struct Entry {
        std::string odUserId;
        std::string displayName;
        int highscore;
        int snakeSize;
        std::string difficulty;
        std::string timestamp;
        int confidenceScore;
        
        bool operator<(const Entry& other) const {
            return highscore > other.highscore; // Descending order
        }
    };

private:
    std::vector<Entry> entries;
    HttpClient http;
    Auth* auth;
    bool online;
    std::string lastError;
    
    std::string getDifficultyString(int level) const {
        switch (level) {
            case 1: return "Easy";
            case 2: return "Normal";
            case 3: return "Hard";
            case 5: return "Insane";
            default: return "Normal";
        }
    }
    
    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }

public:
    Leaderboard(Auth* authPtr) : auth(authPtr), online(false) {}
    
    bool isOnline() const { return online; }
    std::string getLastError() const { return lastError; }
    const std::vector<Entry>& getEntries() const { return entries; }
    
    // Fetch leaderboard from Firebase
    bool fetch(int limit = 20) {
        if (!auth || !auth->isAuthenticated()) {
            lastError = "Not authenticated";
            online = false;
            return false;
        }
        
        std::string url = FirebaseConfig::FIRESTORE_URL + "/leaderboard?orderBy=highscore&pageSize=" + std::to_string(limit);
        
        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + auth->getAccessToken();
        headers["Content-Type"] = "application/json";
        
        auto response = http.get(url, headers);
        
        if (!response.success) {
            lastError = "Failed to fetch leaderboard";
            online = false;
            return false;
        }
        
        // Parse response and populate entries
        // Note: This is simplified - actual Firestore response is more complex
        auto rawEntries = JsonParser::parseLeaderboard(response.body);
        entries.clear();
        for (const auto& raw : rawEntries) {
            Entry entry;
            entry.odUserId = raw.odUserId;
            entry.displayName = raw.displayName;
            entry.highscore = raw.highscore;
            entry.snakeSize = raw.snakeSize;
            entry.difficulty = raw.difficulty;
            entry.timestamp = raw.timestamp;
            entry.confidenceScore = raw.confidenceScore;
            entries.push_back(entry);
        }
        
        // Sort by highscore descending
        std::sort(entries.begin(), entries.end());
        
        online = true;
        return true;
    }
    
    // Submit score to Firebase
    bool submitScore(const AntiCheat::SessionData& sessionData, int snakeSize) {
        if (!auth || !auth->isAuthenticated()) {
            lastError = "Not authenticated";
            return false;
        }
        
        // Validate anti-cheat
        if (sessionData.confidenceScore < 30) {
            lastError = "Session validation failed";
            return false;
        }
        
        std::string odUserId = auth->getUserId();
        std::string url = FirebaseConfig::FIRESTORE_URL + "/leaderboard/" + odUserId;
        
        // First, get current score to see if we should update
        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + auth->getAccessToken();
        headers["Content-Type"] = "application/json";
        
        auto getResponse = http.get(url, headers);
        
        int currentHighscore = 0;
        if (getResponse.success) {
            // Parse existing highscore
            currentHighscore = JsonParser::getInt(getResponse.body, "highscore");
        }
        
        // Only submit if new score is higher
        if (sessionData.score <= currentHighscore) {
            lastError = "Score not higher than current highscore";
            return true; // Not an error, just no update needed
        }
        
        // Create Firestore document
        std::map<std::string, std::string> stringFields;
        stringFields["displayName"] = auth->getDisplayName();
        stringFields["odUserId"] = odUserId;
        stringFields["difficulty"] = getDifficultyString(sessionData.difficulty);
        stringFields["timestamp"] = getCurrentTimestamp();
        stringFields["sessionHash"] = sessionData.sessionHash;
        stringFields["photoUrl"] = auth->getPhotoUrl();
        
        std::map<std::string, int> intFields;
        intFields["highscore"] = sessionData.score;
        intFields["snakeSize"] = snakeSize;
        intFields["foodEaten"] = sessionData.foodEaten;
        intFields["totalMoves"] = sessionData.totalMoves;
        intFields["duration"] = static_cast<int>(sessionData.duration);
        intFields["avgReactionTime"] = static_cast<int>(sessionData.avgReactionTime);
        intFields["confidenceScore"] = sessionData.confidenceScore;
        
        std::string body = JsonParser::createFirestoreDocument(stringFields, intFields);
        
        auto response = http.patch(url, body, headers);
        
        if (!response.success) {
            lastError = "Failed to submit score";
            return false;
        }
        
        return true;
    }
    
    // Submit score with replay data for server-side validation
    bool submitScoreWithReplay(const SnakeCore::ReplayData& replay, int snakeSize) {
        if (!auth || !auth->isAuthenticated()) {
            lastError = "Not authenticated";
            return false;
        }
        
        std::string odUserId = auth->getUserId();
        std::string url = FirebaseConfig::FIRESTORE_URL + "/leaderboard/" + odUserId;
        
        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + auth->getAccessToken();
        headers["Content-Type"] = "application/json";
        
        // Check current highscore
        auto getResponse = http.get(url, headers);
        int currentHighscore = 0;
        if (getResponse.success) {
            currentHighscore = JsonParser::getInt(getResponse.body, "highscore");
        }
        
        // Only submit if new score is higher
        if (static_cast<int>(replay.getFinalScore()) <= currentHighscore) {
            lastError = "Score not higher than current highscore";
            return true;
        }
        
        // Create document with replay data
        std::map<std::string, std::string> stringFields;
        stringFields["displayName"] = auth->getDisplayName();
        stringFields["odUserId"] = odUserId;
        stringFields["difficulty"] = getDifficultyString(replay.getDifficulty());
        stringFields["timestamp"] = getCurrentTimestamp();
        stringFields["photoUrl"] = auth->getPhotoUrl();
        stringFields["replayData"] = replay.toBase64();  // Server will validate this
        stringFields["gameVersion"] = replay.getGameVersion();
        
        std::map<std::string, int> intFields;
        intFields["highscore"] = static_cast<int>(replay.getFinalScore());
        intFields["snakeSize"] = snakeSize;
        intFields["totalFrames"] = static_cast<int>(replay.getTotalFrames());
        intFields["duration"] = static_cast<int>(replay.getTotalTimeMs());
        
        std::string body = JsonParser::createFirestoreDocument(stringFields, intFields);
        
        auto response = http.patch(url, body, headers);
        
        if (!response.success) {
            lastError = "Failed to submit score";
            return false;
        }
        
        return true;
    }

    // Display leaderboard in terminal
    void display() {
        clear();
        
        int centerX = COLS / 2;
        int startY = 2;
        
        // Title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(startY, centerX - 12, "=== WORLD LEADERBOARD ===");
        attroff(COLOR_PAIR(1) | A_BOLD);
        
        // Online status
        if (online) {
            attron(COLOR_PAIR(2));
            mvprintw(startY + 1, centerX - 3, "[ONLINE]");
            attroff(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(4));
            mvprintw(startY + 1, centerX - 4, "[OFFLINE]");
            attroff(COLOR_PAIR(4));
        }
        
        // Header
        int tableY = startY + 3;
        int rankX = centerX - 35;
        int nameX = rankX + 6;
        int scoreX = nameX + 22;
        int sizeX = scoreX + 10;
        int diffX = sizeX + 8;
        
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(tableY, rankX, "RANK");
        mvprintw(tableY, nameX, "PLAYER");
        mvprintw(tableY, scoreX, "SCORE");
        mvprintw(tableY, sizeX, "SIZE");
        mvprintw(tableY, diffX, "DIFFICULTY");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        // Divider
        attron(COLOR_PAIR(1));
        mvhline(tableY + 1, rankX, ACS_HLINE, 65);
        attroff(COLOR_PAIR(1));
        
        // Entries
        int entryY = tableY + 2;
        int rank = 1;
        
        for (const auto& entry : entries) {
            if (entryY >= LINES - 4) break;
            
            // Highlight current user
            bool isCurrentUser = auth && (entry.odUserId == auth->getUserId());
            
            if (isCurrentUser) {
                attron(COLOR_PAIR(2) | A_BOLD);
            } else if (rank <= 3) {
                // Gold, Silver, Bronze for top 3
                int color = (rank == 1) ? 6 : (rank == 2) ? 5 : 4;
                attron(COLOR_PAIR(color));
            } else {
                attron(COLOR_PAIR(1));
            }
            
            // Rank with medal emoji for top 3
            std::string rankStr;
            if (rank == 1) rankStr = "1st";
            else if (rank == 2) rankStr = "2nd";
            else if (rank == 3) rankStr = "3rd";
            else rankStr = std::to_string(rank) + "th";
            
            mvprintw(entryY, rankX, "%-4s", rankStr.c_str());
            
            // Truncate name if too long
            std::string displayName = entry.displayName;
            if (displayName.length() > 18) {
                displayName = displayName.substr(0, 15) + "...";
            }
            mvprintw(entryY, nameX, "%-20s", displayName.c_str());
            mvprintw(entryY, scoreX, "%-8d", entry.highscore);
            mvprintw(entryY, sizeX, "%-6d", entry.snakeSize);
            mvprintw(entryY, diffX, "%s", entry.difficulty.c_str());
            
            if (isCurrentUser) {
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else if (rank <= 3) {
                int color = (rank == 1) ? 6 : (rank == 2) ? 5 : 4;
                attroff(COLOR_PAIR(color));
            } else {
                attroff(COLOR_PAIR(1));
            }
            
            entryY++;
            rank++;
        }
        
        if (entries.empty()) {
            attron(COLOR_PAIR(5));
            mvprintw(entryY, centerX - 10, "No entries yet. Be the first!");
            attroff(COLOR_PAIR(5));
        }
        
        // Footer
        attron(COLOR_PAIR(5));
        mvprintw(LINES - 2, centerX - 15, "Press any key to return to menu");
        attroff(COLOR_PAIR(5));
        
        refresh();
        
        timeout(-1);
        getch();
    }
    
    // Quick display for showing user's rank after game
    void showUserRank(int newScore) {
        if (!auth || !auth->isAuthenticated()) return;
        
        fetch();
        
        std::string odUserId = auth->getUserId();
        int rank = 1;
        bool found = false;
        
        for (const auto& entry : entries) {
            if (entry.odUserId == odUserId) {
                found = true;
                break;
            }
            if (entry.highscore > newScore) {
                rank++;
            }
        }
        
        int centerY = LINES / 2;
        int centerX = COLS / 2;
        
        if (found || newScore > 0) {
            attron(COLOR_PAIR(6) | A_BOLD);
            mvprintw(centerY + 4, centerX - 12, "World Rank: #%d", rank);
            attroff(COLOR_PAIR(6) | A_BOLD);
        }
    }
};

#endif

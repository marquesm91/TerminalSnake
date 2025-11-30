/**
 * @file notifications.hpp
 * @brief Real-time notifications and QR code display for Terminal Snake
 */

#ifndef NOTIFICATIONS_HPP
#define NOTIFICATIONS_HPP

#include <ncurses.h>
#include <string>
#include <queue>
#include <chrono>
#include <vector>

namespace Snake {

/**
 * @brief Notification types for visual styling
 */
enum class NotificationType {
    NewScore,       // New leaderboard entry
    Achievement,    // Achievement unlocked
    Info,           // General info
    Warning,        // Warning message
    Error           // Error message
};

/**
 * @brief A notification to display on screen
 */
struct Notification {
    std::string message;
    std::string submessage;
    NotificationType type;
    std::chrono::steady_clock::time_point createdAt;
    int durationMs;
    
    Notification(const std::string& msg, const std::string& sub, 
                 NotificationType t, int duration = 5000)
        : message(msg), submessage(sub), type(t), 
          createdAt(std::chrono::steady_clock::now()), durationMs(duration) {}
    
    bool isExpired() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - createdAt).count();
        return elapsed >= durationMs;
    }
    
    float progress() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - createdAt).count();
        return static_cast<float>(elapsed) / durationMs;
    }
};

/**
 * @brief Manages and displays notifications
 */
class NotificationManager {
private:
    std::queue<Notification> notifications;
    Notification* current = nullptr;
    
    int getColorPair(NotificationType type) const {
        switch (type) {
            case NotificationType::NewScore:    return 2;  // Green
            case NotificationType::Achievement: return 5;  // Yellow
            case NotificationType::Info:        return 1;  // Cyan
            case NotificationType::Warning:     return 5;  // Yellow
            case NotificationType::Error:       return 4;  // Red
            default:                            return 1;
        }
    }
    
    const char* getIcon(NotificationType type) const {
        switch (type) {
            case NotificationType::NewScore:    return "🏆";
            case NotificationType::Achievement: return "⭐";
            case NotificationType::Info:        return "ℹ️";
            case NotificationType::Warning:     return "⚠️";
            case NotificationType::Error:       return "❌";
            default:                            return "•";
        }
    }

public:
    void push(const Notification& notif) {
        notifications.push(notif);
    }
    
    void pushNewScore(const std::string& playerName, int score, const std::string& difficulty) {
        std::string msg = playerName + " scored " + std::to_string(score);
        std::string sub = "Difficulty: " + difficulty;
        push(Notification(msg, sub, NotificationType::NewScore, 5000));
    }
    
    /**
     * @brief Render current notification (call in game loop)
     * @param cornerX Top-right X position
     * @param cornerY Top-right Y position
     */
    void render(int cornerX = -1, int cornerY = 2) {
        // Process queue
        if (!current || current->isExpired()) {
            if (current) {
                delete current;
                current = nullptr;
            }
            if (!notifications.empty()) {
                current = new Notification(notifications.front());
                notifications.pop();
            }
        }
        
        if (!current) return;
        
        // Calculate position (top-right corner by default)
        int width = static_cast<int>(current->message.length()) + 6;
        int height = 4;
        if (cornerX < 0) cornerX = COLS - width - 2;
        
        // Draw notification box with animation
        float progress = current->progress();
        int colorPair = getColorPair(current->type);
        
        // Slide-in animation for first 10%
        int offsetX = 0;
        if (progress < 0.1f) {
            offsetX = static_cast<int>((1.0f - progress / 0.1f) * width);
        }
        // Slide-out animation for last 10%
        else if (progress > 0.9f) {
            offsetX = static_cast<int>((progress - 0.9f) / 0.1f * width);
        }
        
        int x = cornerX + offsetX;
        
        // Draw background box
        attron(COLOR_PAIR(colorPair));
        for (int i = 0; i < height; ++i) {
            mvhline(cornerY + i, x, ' ', width);
        }
        
        // Draw border
        mvaddch(cornerY, x, ACS_ULCORNER);
        mvaddch(cornerY, x + width - 1, ACS_URCORNER);
        mvaddch(cornerY + height - 1, x, ACS_LLCORNER);
        mvaddch(cornerY + height - 1, x + width - 1, ACS_LRCORNER);
        mvhline(cornerY, x + 1, ACS_HLINE, width - 2);
        mvhline(cornerY + height - 1, x + 1, ACS_HLINE, width - 2);
        mvvline(cornerY + 1, x, ACS_VLINE, height - 2);
        mvvline(cornerY + 1, x + width - 1, ACS_VLINE, height - 2);
        
        // Draw content
        attron(A_BOLD);
        mvprintw(cornerY + 1, x + 2, "%s %s", getIcon(current->type), current->message.c_str());
        attroff(A_BOLD);
        
        if (!current->submessage.empty()) {
            mvprintw(cornerY + 2, x + 4, "%s", current->submessage.c_str());
        }
        
        attroff(COLOR_PAIR(colorPair));
    }
    
    void clear() {
        while (!notifications.empty()) notifications.pop();
        if (current) {
            delete current;
            current = nullptr;
        }
    }
    
    ~NotificationManager() {
        clear();
    }
};

/**
 * @brief QR Code generator and display for terminal
 * Uses simple ASCII art representation
 */
class QRCodeDisplay {
private:
    // Simple QR-like pattern generator (not a real QR code, but visually similar)
    // For real QR, you'd need libqrencode
    static std::vector<std::string> generateSimplePattern(const std::string& url) {
        // Create a visual pattern that looks like a QR code
        // In production, use libqrencode for real QR codes
        std::vector<std::string> pattern = {
            "█▀▀▀▀▀█ ▄▄▄▄ █▀▀▀▀▀█",
            "█ ███ █ █▄▀▄ █ ███ █",
            "█ ▀▀▀ █ ▀▄█▀ █ ▀▀▀ █",
            "▀▀▀▀▀▀▀ █▀█▀ ▀▀▀▀▀▀▀",
            "▀▄█▀█▄▀▀▄▀█▄█▀▄▀▄▀██",
            "█▄▀▀█▄▀█▀▀▄█▄▀█▀▄█▄█",
            "▀▀▀▀▀▀▀ █▄▄█▀▄▀█▀▄▀▄",
            "█▀▀▀▀▀█ ▀▄█▄▀▀▄▀▀█▄▀",
            "█ ███ █ ▄▀▄█▀▄█▀▄▀▄▄",
            "█ ▀▀▀ █ █▀█▄▀▀█▄█▀▄█",
            "▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀▀▀"
        };
        return pattern;
    }

public:
    /**
     * @brief Display QR code with URL in terminal
     */
    static void display(const std::string& url, const std::string& title = "Scan to Play Online") {
        clear();
        
        int centerY = LINES / 2;
        int centerX = COLS / 2;
        
        auto pattern = generateSimplePattern(url);
        int qrHeight = static_cast<int>(pattern.size());
        int qrWidth = static_cast<int>(pattern[0].length());
        
        // Box dimensions
        int boxWidth = qrWidth + 8;
        int boxHeight = qrHeight + 10;
        int boxX = centerX - boxWidth / 2;
        int boxY = centerY - boxHeight / 2;
        
        // Draw border
        attron(COLOR_PAIR(1));
        for (int i = 0; i < boxHeight; ++i) {
            mvhline(boxY + i, boxX, ' ', boxWidth);
        }
        
        // Border
        mvaddch(boxY, boxX, ACS_ULCORNER);
        mvaddch(boxY, boxX + boxWidth - 1, ACS_URCORNER);
        mvaddch(boxY + boxHeight - 1, boxX, ACS_LLCORNER);
        mvaddch(boxY + boxHeight - 1, boxX + boxWidth - 1, ACS_LRCORNER);
        mvhline(boxY, boxX + 1, ACS_HLINE, boxWidth - 2);
        mvhline(boxY + boxHeight - 1, boxX + 1, ACS_HLINE, boxWidth - 2);
        mvvline(boxY + 1, boxX, ACS_VLINE, boxHeight - 2);
        mvvline(boxY + 1, boxX + boxWidth - 1, ACS_VLINE, boxHeight - 2);
        attroff(COLOR_PAIR(1));
        
        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(boxY + 1, centerX - static_cast<int>(title.length()) / 2, "%s", title.c_str());
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        // QR Code
        attron(COLOR_PAIR(1));
        int qrX = centerX - qrWidth / 2;
        int qrY = boxY + 3;
        for (size_t i = 0; i < pattern.size(); ++i) {
            mvprintw(qrY + static_cast<int>(i), qrX, "%s", pattern[i].c_str());
        }
        attroff(COLOR_PAIR(1));
        
        // URL
        attron(COLOR_PAIR(5));
        mvprintw(boxY + boxHeight - 4, centerX - static_cast<int>(url.length()) / 2, "%s", url.c_str());
        attroff(COLOR_PAIR(5));
        
        // Instructions
        attron(COLOR_PAIR(1));
        mvprintw(boxY + boxHeight - 2, centerX - 15, "Press any key to go back");
        attroff(COLOR_PAIR(1));
        
        refresh();
        
        // Wait for key
        timeout(-1);
        getch();
    }
    
    /**
     * @brief Generate shareable link with game stats
     */
    static std::string generateShareLink(int score, const std::string& difficulty) {
        // Base URL for the web version
        return "https://terminalsnake-leaderboard.web.app/?score=" + 
               std::to_string(score) + "&diff=" + difficulty;
    }
};

/**
 * @brief Leaderboard display with category filtering
 */
class LeaderboardDisplay {
public:
    struct Entry {
        std::string name;
        int score;
        int rank;
        std::string difficulty;
        bool isCurrentUser;
    };

private:
    std::vector<Entry> entries;
    int selectedCategory;  // 0=All, 1=Easy, 2=Normal, 3=Hard, 4=Insane
    std::vector<std::string> categories = {"All", "Easy", "Normal", "Hard", "Insane"};

public:
    LeaderboardDisplay() : selectedCategory(0) {}
    
    void setEntries(const std::vector<Entry>& e) { entries = e; }
    
    void nextCategory() {
        selectedCategory = (selectedCategory + 1) % static_cast<int>(categories.size());
    }
    
    void prevCategory() {
        selectedCategory = (selectedCategory - 1 + static_cast<int>(categories.size())) % static_cast<int>(categories.size());
    }
    
    std::string getCurrentCategory() const {
        return categories[selectedCategory];
    }
    
    /**
     * @brief Display leaderboard with category tabs
     */
    void display() {
        clear();
        
        int centerX = COLS / 2;
        int boxWidth = 50;
        int boxHeight = 20;
        int boxX = centerX - boxWidth / 2;
        int boxY = 3;
        
        // Draw border
        attron(COLOR_PAIR(1));
        mvaddch(boxY, boxX, ACS_ULCORNER);
        mvaddch(boxY, boxX + boxWidth - 1, ACS_URCORNER);
        mvaddch(boxY + boxHeight - 1, boxX, ACS_LLCORNER);
        mvaddch(boxY + boxHeight - 1, boxX + boxWidth - 1, ACS_LRCORNER);
        mvhline(boxY, boxX + 1, ACS_HLINE, boxWidth - 2);
        mvhline(boxY + boxHeight - 1, boxX + 1, ACS_HLINE, boxWidth - 2);
        mvvline(boxY + 1, boxX, ACS_VLINE, boxHeight - 2);
        mvvline(boxY + 1, boxX + boxWidth - 1, ACS_VLINE, boxHeight - 2);
        attroff(COLOR_PAIR(1));
        
        // Title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(boxY + 1, centerX - 7, "🏆 LEADERBOARD");
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        // Category tabs
        int tabX = boxX + 2;
        int tabY = boxY + 3;
        for (size_t i = 0; i < categories.size(); ++i) {
            if (static_cast<int>(i) == selectedCategory) {
                attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            } else {
                attron(COLOR_PAIR(1));
            }
            mvprintw(tabY, tabX, " %s ", categories[i].c_str());
            tabX += static_cast<int>(categories[i].length()) + 3;
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            attroff(COLOR_PAIR(1));
        }
        
        // Separator line
        attron(COLOR_PAIR(1));
        mvhline(tabY + 1, boxX + 1, ACS_HLINE, boxWidth - 2);
        attroff(COLOR_PAIR(1));
        
        // Header
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(tabY + 2, boxX + 3, "Rank  Player              Score    Diff");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        // Entries
        int entryY = tabY + 4;
        int shown = 0;
        for (const auto& entry : entries) {
            // Filter by category
            if (selectedCategory > 0 && entry.difficulty != categories[selectedCategory]) {
                continue;
            }
            
            if (shown >= 10) break;  // Max 10 entries
            
            // Highlight current user
            if (entry.isCurrentUser) {
                attron(COLOR_PAIR(2) | A_BOLD);
            } else {
                attron(COLOR_PAIR(1));
            }
            
            // Rank medal
            const char* medal = "";
            if (entry.rank == 1) medal = "🥇";
            else if (entry.rank == 2) medal = "🥈";
            else if (entry.rank == 3) medal = "🥉";
            
            mvprintw(entryY + shown, boxX + 3, "%s%-3d  %-18s %6d   %s",
                     medal, entry.rank, entry.name.substr(0, 18).c_str(),
                     entry.score, entry.difficulty.c_str());
            
            attroff(COLOR_PAIR(2) | A_BOLD);
            attroff(COLOR_PAIR(1));
            
            ++shown;
        }
        
        // No entries message
        if (shown == 0) {
            attron(COLOR_PAIR(1));
            mvprintw(entryY + 2, centerX - 10, "No entries in this category");
            attroff(COLOR_PAIR(1));
        }
        
        // Instructions
        attron(COLOR_PAIR(5));
        mvprintw(boxY + boxHeight - 2, boxX + 3, "←/→: Change category  Q: Back  R: Refresh");
        attroff(COLOR_PAIR(5));
        
        refresh();
    }
    
    /**
     * @brief Handle input
     * @return true if should exit, false to continue
     */
    bool handleInput() {
        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                prevCategory();
                return false;
            case KEY_RIGHT:
                nextCategory();
                return false;
            case 'q':
            case 'Q':
                return true;
            case 'r':
            case 'R':
                // Trigger refresh (caller should fetch new data)
                return false;
            default:
                return false;
        }
    }
};

} // namespace Snake

#endif // NOTIFICATIONS_HPP

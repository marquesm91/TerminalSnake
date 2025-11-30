/**
 * @file ncurses_renderer.hpp
 * @brief NCursesRenderer - Adaptador de renderização para terminal
 * 
 * Clean Architecture: Infrastructure Layer - Adapter
 * Hexagonal: Output Adapter (implements Port)
 */

#ifndef INFRASTRUCTURE_ADAPTERS_OUTPUT_NCURSES_RENDERER_HPP
#define INFRASTRUCTURE_ADAPTERS_OUTPUT_NCURSES_RENDERER_HPP

#include <ncurses.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "application/ports/renderer_port.hpp"

namespace Snake {
namespace Infrastructure {
namespace Adapters {

/**
 * @brief Cores para o terminal
 */
enum class TerminalColor {
    Default = 0,
    SnakeHead = 1,
    SnakeBody = 2,
    Food = 3,
    Border = 4,
    Score = 5,
    GameOver = 6,
    MenuSelected = 7,
    MenuNormal = 8
};

/**
 * @brief Renderizador NCurses para terminal
 */
class NCursesRenderer : public Application::Ports::IRenderer {
public:
    NCursesRenderer() : initialized_(false), offsetX_(0), offsetY_(0) {}
    
    ~NCursesRenderer() override {
        if (initialized_) {
            shutdown();
        }
    }
    
    void init() override {
        if (initialized_) return;
        
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        
        // Initialize colors
        if (has_colors()) {
            start_color();
            use_default_colors();
            
            init_pair(static_cast<int>(TerminalColor::SnakeHead), COLOR_GREEN, -1);
            init_pair(static_cast<int>(TerminalColor::SnakeBody), COLOR_GREEN, -1);
            init_pair(static_cast<int>(TerminalColor::Food), COLOR_RED, -1);
            init_pair(static_cast<int>(TerminalColor::Border), COLOR_WHITE, -1);
            init_pair(static_cast<int>(TerminalColor::Score), COLOR_YELLOW, -1);
            init_pair(static_cast<int>(TerminalColor::GameOver), COLOR_RED, -1);
            init_pair(static_cast<int>(TerminalColor::MenuSelected), COLOR_BLACK, COLOR_WHITE);
            init_pair(static_cast<int>(TerminalColor::MenuNormal), COLOR_WHITE, -1);
        }
        
        initialized_ = true;
    }
    
    void shutdown() override {
        if (!initialized_) return;
        
        endwin();
        initialized_ = false;
    }
    
    void beginFrame() override {
        // Calcula offset para centralizar
        offsetX_ = (COLS - 50) / 2;
        offsetY_ = (LINES - 25) / 2;
        if (offsetX_ < 0) offsetX_ = 0;
        if (offsetY_ < 0) offsetY_ = 0;
    }
    
    void endFrame() override {
        refresh();
    }
    
    void clear() override {
        ::clear();
    }
    
    void drawBorder(uint8_t width, uint8_t height) override {
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Border)));
        
        // Cantos
        mvaddch(offsetY_, offsetX_, ACS_ULCORNER);
        mvaddch(offsetY_, offsetX_ + width + 1, ACS_URCORNER);
        mvaddch(offsetY_ + height + 1, offsetX_, ACS_LLCORNER);
        mvaddch(offsetY_ + height + 1, offsetX_ + width + 1, ACS_LRCORNER);
        
        // Linhas horizontais
        for (int x = 1; x <= width; ++x) {
            mvaddch(offsetY_, offsetX_ + x, ACS_HLINE);
            mvaddch(offsetY_ + height + 1, offsetX_ + x, ACS_HLINE);
        }
        
        // Linhas verticais
        for (int y = 1; y <= height; ++y) {
            mvaddch(offsetY_ + y, offsetX_, ACS_VLINE);
            mvaddch(offsetY_ + y, offsetX_ + width + 1, ACS_VLINE);
        }
        
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Border)));
    }
    
    void drawSnake(const Domain::SnakeEntity& snake) override {
        bool first = true;
        for (const auto& segment : snake) {
            if (first) {
                // Cabeça
                attron(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeHead)) | A_BOLD);
                mvaddch(offsetY_ + 1 + segment.y(), 
                       offsetX_ + 1 + segment.x(), '@');
                attroff(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeHead)) | A_BOLD);
                first = false;
            } else {
                // Corpo
                attron(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeBody)));
                mvaddch(offsetY_ + 1 + segment.y(), 
                       offsetX_ + 1 + segment.x(), 'o');
                attroff(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeBody)));
            }
        }
    }
    
    void drawFood(const Domain::FoodEntity& food) override {
        if (!food.isActive()) return;
        
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Food)) | A_BOLD);
        mvaddch(offsetY_ + 1 + food.position().y(),
               offsetX_ + 1 + food.position().x(), '*');
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Food)) | A_BOLD);
    }
    
    void drawScore(uint32_t score, uint32_t highscore) override {
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
        mvprintw(offsetY_ - 1, offsetX_, "Score: %u  High: %u", score, highscore);
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
    }
    
    void drawPaused() override {
        int centerY = LINES / 2;
        int centerX = COLS / 2;
        
        attron(A_BOLD | A_REVERSE);
        mvprintw(centerY, centerX - 4, " PAUSED ");
        attroff(A_BOLD | A_REVERSE);
    }
    
    void drawGameOver(uint32_t score, uint32_t highscore) override {
        int centerY = LINES / 2;
        int centerX = COLS / 2;
        
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::GameOver)) | A_BOLD);
        mvprintw(centerY - 2, centerX - 5, "GAME OVER!");
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::GameOver)) | A_BOLD);
        
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
        mvprintw(centerY, centerX - 8, "Final Score: %u", score);
        
        if (score >= highscore && score > 0) {
            mvprintw(centerY + 1, centerX - 7, "NEW HIGHSCORE!");
        } else {
            mvprintw(centerY + 1, centerX - 6, "High: %u", highscore);
        }
        
        mvprintw(centerY + 3, centerX - 10, "Press any key...");
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
    }
    
    void drawMenu(int selectedOption, uint32_t highscore) override {
        const char* options[] = {
            "Start Game",
            "Leaderboard",
            "Settings",
            "Sign In",
            "Exit"
        };
        const int numOptions = 5;
        
        int centerY = LINES / 2 - numOptions;
        int centerX = COLS / 2;
        
        // Título
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeHead)) | A_BOLD);
        mvprintw(centerY - 4, centerX - 6, "TERMINAL SNAKE");
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeHead)) | A_BOLD);
        
        // Highscore
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
        mvprintw(centerY - 2, centerX - 8, "High Score: %u", highscore);
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
        
        // Opções
        for (int i = 0; i < numOptions; ++i) {
            int y = centerY + i * 2;
            int x = centerX - static_cast<int>(strlen(options[i])) / 2;
            
            if (i == selectedOption) {
                attron(COLOR_PAIR(static_cast<int>(TerminalColor::MenuSelected)));
                mvprintw(y, x - 2, "> %s <", options[i]);
                attroff(COLOR_PAIR(static_cast<int>(TerminalColor::MenuSelected)));
            } else {
                attron(COLOR_PAIR(static_cast<int>(TerminalColor::MenuNormal)));
                mvprintw(y, x, "%s", options[i]);
                attroff(COLOR_PAIR(static_cast<int>(TerminalColor::MenuNormal)));
            }
        }
    }
    
    void drawLeaderboard(const std::vector<Application::Ports::LeaderboardEntry>& entries = {}) override {
        int centerY = LINES / 2;
        int centerX = COLS / 2;
        
        // Title
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeHead)) | A_BOLD);
        mvprintw(3, centerX - 10, "=== WORLD LEADERBOARD ===");
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::SnakeHead)) | A_BOLD);
        
        // Column headers
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Score)) | A_UNDERLINE);
        mvprintw(5, centerX - 25, "%-4s %-20s %-8s %-10s %-8s", 
                 "Rank", "Player", "Score", "Difficulty", "Verified");
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Score)) | A_UNDERLINE);
        
        if (entries.empty()) {
            attron(COLOR_PAIR(static_cast<int>(TerminalColor::MenuNormal)));
            mvprintw(centerY, centerX - 15, "No entries or not connected");
            mvprintw(centerY + 1, centerX - 12, "Sign in to view leaderboard");
            attroff(COLOR_PAIR(static_cast<int>(TerminalColor::MenuNormal)));
        } else {
            // Display entries (max 15 to fit screen)
            int maxEntries = std::min(static_cast<int>(entries.size()), 15);
            for (int i = 0; i < maxEntries; ++i) {
                const auto& entry = entries[static_cast<size_t>(i)];
                int y = 7 + i;
                
                // Truncate display name if needed
                std::string displayName = entry.displayName;
                if (displayName.length() > 18) {
                    displayName = displayName.substr(0, 15) + "...";
                }
                
                // Highlight top 3
                if (i < 3) {
                    attron(COLOR_PAIR(static_cast<int>(TerminalColor::Food)) | A_BOLD);
                } else {
                    attron(COLOR_PAIR(static_cast<int>(TerminalColor::MenuNormal)));
                }
                
                mvprintw(y, centerX - 25, "%-4d %-20s %-8u %-10s %-8s",
                         i + 1,
                         displayName.c_str(),
                         entry.score,
                         entry.difficulty.c_str(),
                         entry.verified ? "Yes" : "No");
                
                if (i < 3) {
                    attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Food)) | A_BOLD);
                } else {
                    attroff(COLOR_PAIR(static_cast<int>(TerminalColor::MenuNormal)));
                }
            }
        }
        
        // Footer
        attron(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
        mvprintw(LINES - 2, centerX - 12, "Press any key to go back");
        attroff(COLOR_PAIR(static_cast<int>(TerminalColor::Score)));
    }
    
    void drawMessage(const char* message) override {
        int centerY = LINES / 2;
        int centerX = COLS / 2 - static_cast<int>(strlen(message)) / 2;
        
        mvprintw(centerY, centerX, "%s", message);
    }
    
    int screenWidth() const override { return COLS; }
    int screenHeight() const override { return LINES; }

private:
    bool initialized_;
    int offsetX_;
    int offsetY_;
};

} // namespace Adapters
} // namespace Infrastructure
} // namespace Snake

#endif // INFRASTRUCTURE_ADAPTERS_OUTPUT_NCURSES_RENDERER_HPP

/**
 * Terminal Snake v3.0
 * 
 * Clean Architecture / Hexagonal implementation
 * 
 * Layers:
 * - Domain: Entities, Value Objects, Domain Services
 * - Application: Use Cases, Ports (interfaces)
 * - Infrastructure: Adapters, Persistence, External Services
 * 
 * Build: make clean_arch
 * Run: ./bin/tsnake_clean
 */

#include <csignal>
#include <iostream>

// Domain Layer
#include "domain/value_objects/game_config.hpp"
#include "domain/entities/game.hpp"

// Application Layer - Use Cases
#include "application/use_cases/play_game_use_case.hpp"

// Infrastructure Layer - Adapters
#include "infrastructure/adapters/output/ncurses_renderer.hpp"
#include "infrastructure/adapters/input/ncurses_input.hpp"
#include "infrastructure/adapters/output/std_timer.hpp"
#include "infrastructure/persistence/file_storage.hpp"

// Firebase adapters (optional - compile with -DWITH_FIREBASE)
#ifdef WITH_FIREBASE
#include "infrastructure/external/firebase_auth_adapter.hpp"
#include "infrastructure/external/firebase_leaderboard_adapter.hpp"
#endif

using namespace Snake;

// Global state for signal handling
volatile sig_atomic_t g_interrupt = 0;
Infrastructure::Adapters::NCursesRenderer* g_renderer = nullptr;

void signalHandler(int /* sig */) {
    g_interrupt = 1;
    if (g_renderer) {
        g_renderer->shutdown();
    }
}

/**
 * @brief Main menu
 */
int showMenu(Infrastructure::Adapters::NCursesRenderer& renderer,
             Infrastructure::Adapters::NCursesInput& input,
             uint32_t highscore) {
    
    int selected = 0;
    const int numOptions = 6;
    
    while (!g_interrupt) {
        renderer.beginFrame();
        renderer.clear();
        renderer.drawMenu(selected, highscore);
        renderer.endFrame();
        
        auto cmd = input.waitForCommand();
        
        switch (cmd) {
            case Application::Ports::InputCommand::MoveUp:
            case Application::Ports::InputCommand::MenuUp:
                selected = (selected - 1 + numOptions) % numOptions;
                break;
                
            case Application::Ports::InputCommand::MoveDown:
            case Application::Ports::InputCommand::MenuDown:
                selected = (selected + 1) % numOptions;
                break;
                
            case Application::Ports::InputCommand::Confirm:
                return selected;
                
            case Application::Ports::InputCommand::Quit:
                return 5;  // Exit
                
            default:
                break;
        }
    }
    
    return 5;  // Exit on interrupt
}

/**
 * @brief Difficulty selection
 */
Domain::Difficulty selectDifficulty(
    Infrastructure::Adapters::NCursesRenderer& renderer,
    Infrastructure::Adapters::NCursesInput& input) {
    
    const char* difficulties[] = {"Easy", "Normal", "Hard", "Insane"};
    const Domain::Difficulty values[] = {
        Domain::Difficulty::Easy,
        Domain::Difficulty::Normal,
        Domain::Difficulty::Hard,
        Domain::Difficulty::Insane
    };
    int selected = 1;  // Normal default
    const int numOptions = 4;
    
    while (!g_interrupt) {
        renderer.beginFrame();
        renderer.clear();
        
        // Title
        int centerY = renderer.screenHeight() / 2 - 4;
        int centerX = renderer.screenWidth() / 2;
        
        mvprintw(centerY - 2, centerX - 10, "Select Difficulty:");
        
        for (int i = 0; i < numOptions; ++i) {
            int y = centerY + i * 2;
            if (i == selected) {
                attron(A_REVERSE);
                mvprintw(y, centerX - 5, "> %s <", difficulties[i]);
                attroff(A_REVERSE);
            } else {
                mvprintw(y, centerX - 4, "%s", difficulties[i]);
            }
        }
        
        renderer.endFrame();
        
        auto cmd = input.waitForCommand();
        
        switch (cmd) {
            case Application::Ports::InputCommand::MoveUp:
                selected = (selected - 1 + numOptions) % numOptions;
                break;
                
            case Application::Ports::InputCommand::MoveDown:
                selected = (selected + 1) % numOptions;
                break;
                
            case Application::Ports::InputCommand::Confirm:
                return values[selected];
                
            case Application::Ports::InputCommand::Quit:
            case Application::Ports::InputCommand::Back:
                return Domain::Difficulty::Normal;
                
            default:
                break;
        }
    }
    
    return Domain::Difficulty::Normal;
}

#ifdef WITH_FIREBASE
/**
 * @brief Show authentication screen with Device Flow
 */
void showAuthScreen(
    Infrastructure::Adapters::NCursesRenderer& renderer,
    Infrastructure::Adapters::NCursesInput& input,
    Infrastructure::External::FirebaseAuthAdapter& auth) {
    
    renderer.beginFrame();
    renderer.clear();
    
    int centerY = renderer.screenHeight() / 2;
    int centerX = renderer.screenWidth() / 2;
    
    if (auth.isAuthenticated()) {
        auto user = auth.getCurrentUser();
        mvprintw(centerY - 2, centerX - 10, "Signed in as:");
        mvprintw(centerY, centerX - static_cast<int>(user.displayName.length()) / 2, 
                 "%s", user.displayName.c_str());
        mvprintw(centerY + 2, centerX - 8, "[L] Logout  [B] Back");
        renderer.endFrame();
        
        while (!g_interrupt) {
            auto cmd = input.waitForCommand();
            if (cmd == Application::Ports::InputCommand::Back || 
                cmd == Application::Ports::InputCommand::Quit) {
                return;
            }
            int ch = getch();
            if (ch == 'l' || ch == 'L') {
                auth.logout();
                renderer.beginFrame();
                renderer.clear();
                mvprintw(centerY, centerX - 8, "Logged out successfully");
                renderer.endFrame();
                input.waitForCommand();
                return;
            }
        }
    } else {
        mvprintw(centerY - 4, centerX - 12, "Google OAuth Sign In");
        mvprintw(centerY - 2, centerX - 15, "Starting device flow...");
        renderer.endFrame();
        
        // Start device flow
        auto result = auth.startDeviceFlow();
        if (result.success) {
            // Parse URL and code from result message
            size_t sep = result.errorMessage.find(" | Code: ");
            std::string url = result.errorMessage.substr(0, sep);
            std::string code = result.errorMessage.substr(sep + 9);
            
            renderer.beginFrame();
            renderer.clear();
            mvprintw(centerY - 4, centerX - 12, "Google OAuth Sign In");
            mvprintw(centerY - 2, centerX - 10, "Visit this URL:");
            mvprintw(centerY, centerX - static_cast<int>(url.length()) / 2, "%s", url.c_str());
            mvprintw(centerY + 2, centerX - 8, "Enter code: %s", code.c_str());
            mvprintw(centerY + 4, centerX - 12, "Waiting for auth...");
            renderer.endFrame();
            
            // Wait for auth (blocking)
            result = auth.waitForAuth();
            
            renderer.beginFrame();
            renderer.clear();
            if (result.success) {
                mvprintw(centerY, centerX - 12, "Signed in as %s!", result.user.displayName.c_str());
            } else {
                mvprintw(centerY, centerX - 10, "Auth failed: %s", result.errorMessage.c_str());
            }
            renderer.endFrame();
            input.waitForCommand();
        } else {
            renderer.beginFrame();
            renderer.clear();
            mvprintw(centerY, centerX - 15, "Failed to start auth: %s", result.errorMessage.c_str());
            renderer.endFrame();
            input.waitForCommand();
        }
    }
}
#endif

int main() {
    // Dependency Injection - create adapters
    Infrastructure::Adapters::NCursesRenderer renderer;
    Infrastructure::Adapters::NCursesInput input;
    Infrastructure::Adapters::StdTimer timer;
    Infrastructure::Persistence::FileStorage storage;
    
#ifdef WITH_FIREBASE
    Infrastructure::External::FirebaseAuthAdapter auth;
    Infrastructure::External::FirebaseLeaderboardAdapter leaderboard(auth);
#endif
    
    g_renderer = &renderer;
    
    // Initialization
    renderer.init();
    signal(SIGINT, signalHandler);
    
    // Create use case
    Application::UseCases::PlayGameUseCase playGame(renderer, input, timer, storage);
    
    // Main loop
    bool running = true;
    while (running && !g_interrupt) {
        uint32_t highscore = storage.loadHighscore();
        int choice = showMenu(renderer, input, highscore);
        
        switch (choice) {
            case 0: {  // Start Game
                // Select difficulty
                Domain::Difficulty diff = selectDifficulty(renderer, input);
                
                // Configure game
                Domain::GameConfig config = Domain::GameConfig::Builder()
                    .boardSize(40, 20)
                    .difficulty(diff)
                    .initialSnakeSize(3)
                    .frameDelayMs(80)
                    .build();
                
                // Time-based seed
                uint32_t seed = timer.currentTimeMs();
                
                // Execute game
                nodelay(stdscr, TRUE);
                auto result = playGame.execute(config, seed);
                nodelay(stdscr, FALSE);
                
#ifdef WITH_FIREBASE
                // Submit to leaderboard if authenticated and new highscore
                if (result.newHighscore && auth.isAuthenticated()) {
                    renderer.beginFrame();
                    renderer.clear();
                    renderer.drawMessage("Submitting score to leaderboard...");
                    renderer.endFrame();
                    
                    auto submitResult = leaderboard.submitScore(result.replay, result.snakeSize);
                    
                    renderer.beginFrame();
                    renderer.clear();
                    if (submitResult.success) {
                        char msg[64];
                        std::snprintf(msg, sizeof(msg), "Rank: #%d on world leaderboard!", submitResult.rank);
                        renderer.drawMessage(msg);
                    } else {
                        renderer.drawMessage("Failed to submit score");
                    }
                    renderer.endFrame();
                    input.waitForCommand();
                }
#endif
                break;
            }
            
            case 1: {  // Leaderboard
#ifdef WITH_FIREBASE
                // Fetch leaderboard from Firebase
                renderer.beginFrame();
                renderer.clear();
                renderer.drawMessage("Loading leaderboard...");
                renderer.endFrame();
                
                auto entries = leaderboard.fetchTop(20);
                
                renderer.beginFrame();
                renderer.clear();
                renderer.drawLeaderboard(entries);
                renderer.endFrame();
#else
                // Offline mode - no leaderboard
                renderer.beginFrame();
                renderer.clear();
                renderer.drawLeaderboard();
                renderer.endFrame();
#endif
                input.waitForCommand();
                break;
            }
                
            case 2:  // Settings
                renderer.beginFrame();
                renderer.clear();
                renderer.drawMessage("Settings coming soon...");
                renderer.endFrame();
                input.waitForCommand();
                break;
                
            case 3:  // Sign In
#ifdef WITH_FIREBASE
                showAuthScreen(renderer, input, auth);
#else
                renderer.beginFrame();
                renderer.clear();
                renderer.drawMessage("Build with -DWITH_FIREBASE to enable authentication");
                renderer.endFrame();
                input.waitForCommand();
#endif
                break;
                
            case 4:  // Share (QR Code)
                renderer.beginFrame();
                renderer.clear();
                renderer.drawMessage("Scan QR Code to share!");
                // TODO: Render actual QR Code
                renderer.endFrame();
                input.waitForCommand();
                break;
                
            case 5:  // Exit
            default:
                running = false;
                break;
        }
    }
    
    // Cleanup
    renderer.shutdown();
    g_renderer = nullptr;
    
    return 0;
}

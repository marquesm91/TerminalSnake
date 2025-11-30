/**
 * Terminal Snake v2.0
 * 
 * A modular snake game with:
 * - Cross-platform support (Terminal/Arduino)
 * - World leaderboard with Firebase
 * - Anti-cheat replay system
 * - Google authentication
 * 
 * Build: make
 * Run: ./bin/tsnake
 */

#include <csignal>
#include <iostream>

// Core game engine (platform independent)
#include "./libs/core/engine.hpp"
#include "./libs/core/replay.hpp"

// Platform-specific implementation
#include "./libs/platform/ncurses_platform.hpp"

// Legacy components (menu, auth, leaderboard)
#include "./libs/menu.hpp"
#include "./libs/highscore.hpp"
#include "./libs/auth.hpp"
#include "./libs/leaderboard.hpp"

using namespace SnakeCore;
using namespace SnakePlatform;

// Global state
volatile sig_atomic_t interruptFlag = 0;
NCursesPlatform* g_platform = nullptr;

void interruptFunction(int /* sig */) {
    interruptFlag = 1;
    if (g_platform) {
        g_platform->shutdown();
    }
}

// Run a single game session using the new modular engine
bool runGameSession(NCursesPlatform& platform, int difficulty, 
                    Auth* auth, Leaderboard* leaderboard, Highscore& highscore) {
    
    // Configure game
    GameConfig config;
    config.difficulty = static_cast<uint8_t>(difficulty);
    config.frameDelayMs = 80;  // Base delay, can be adjusted by difficulty
    
    // Create game engine
    GameEngine engine(&platform);
    engine.setConfig(config);
    engine.enableRecording(true);
    
    // Generate seed (could come from server for extra security)
    uint32_t seed = platform.getTimeMs();
    
    // Start game
    engine.startGame(seed);
    
    // Main game loop
    while (!interruptFlag && engine.tick()) {
        // Game is running
    }
    
    if (interruptFlag) {
        return false;
    }
    
    // Game over - get results
    uint32_t finalScore = engine.getScore();
    uint16_t snakeSize = engine.getSnakeSize();
    
    // Update local highscore
    if (static_cast<int>(finalScore) > highscore.get()) {
        highscore.set(static_cast<int>(finalScore));
    }
    
    // Submit to leaderboard if authenticated
    if (auth && auth->isAuthenticated() && leaderboard) {
        const ReplayData& replay = engine.getReplayData();
        
        // Submit with replay data - server will validate by re-simulating the game
        leaderboard->submitScoreWithReplay(replay, static_cast<int>(snakeSize));
        
        // Show rank (could fetch updated leaderboard)
        leaderboard->showUserRank(static_cast<int>(finalScore));
    }
    
    // Wait for play again decision
    char choice = platform.waitForPlayAgain();
    
    return (choice == 'Y' || choice == '\n');
}

void showMenu(NCursesPlatform& platform) {
    Menu menu;
    Highscore highscore;
    Auth auth;
    Leaderboard leaderboard(&auth);
    
    // Update menu if user is already signed in
    if (auth.isAuthenticated()) {
        menu.setUserSignedIn(true, auth.getDisplayName());
    }
    
    // Set highscore for display
    platform.setHighscore(highscore.get());
    
    nodelay(stdscr, FALSE);  // Enable blocking for menu navigation
    
    bool running = true;
    while (running && !interruptFlag) {
        int choice = menu.showMainMenu(highscore.get());
        
        switch (choice) {
            case 0: // Start Game
                clear();
                nodelay(stdscr, TRUE);  // Disable blocking for game
                
                while (runGameSession(platform, menu.getDifficultyLevel(), 
                                      &auth, &leaderboard, highscore)) {
                    // Reload highscore for next game
                    highscore = Highscore();
                    platform.setHighscore(highscore.get());
                }
                
                nodelay(stdscr, FALSE);  // Re-enable blocking for menu
                // Reload highscore after game
                highscore = Highscore();
                platform.setHighscore(highscore.get());
                break;
            
            case 1: // Leaderboard
                leaderboard.fetch();
                leaderboard.display();
                break;
                
            case 2: // Settings
                while (!menu.showSettings() && !interruptFlag) {
                    // Stay in settings until user presses 'q'
                }
                break;
            
            case 3: // Sign In / Sign Out
                if (auth.isAuthenticated()) {
                    auth.logout();
                    menu.setUserSignedIn(false);
                } else {
                    if (auth.authenticateWithDeviceFlow()) {
                        menu.setUserSignedIn(true, auth.getDisplayName());
                    }
                }
                break;
                
            case 4: // Exit
                running = false;
                break;
                
            default:
                // Continue in menu (animation frame)
                break;
        }
    }
}

int main() {
    // Create platform
    NCursesPlatform platform;
    g_platform = &platform;
    
    // Initialize platform
    platform.init();
    
    // Setup signal handler
    signal(SIGINT, interruptFunction);
    
    // Run main menu
    showMenu(platform);
    
    // Cleanup
    platform.shutdown();
    g_platform = nullptr;
    
    return 0;
}

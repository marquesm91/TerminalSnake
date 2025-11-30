#ifndef FIREBASE_CONFIG_H_
#define FIREBASE_CONFIG_H_

#include <string>

// Firebase Project Configuration
// Replace these with your actual Firebase project values
namespace FirebaseConfig {
    // OAuth 2.0 Client ID (from Google Cloud Console)
    const std::string CLIENT_ID = "YOUR_CLIENT_ID.apps.googleusercontent.com";
    
    // Firebase Project ID
    const std::string PROJECT_ID = "terminalsnake-leaderboard";
    
    // Firestore Database URL
    const std::string FIRESTORE_URL = "https://firestore.googleapis.com/v1/projects/" + PROJECT_ID + "/databases/(default)/documents";
    
    // Google OAuth URLs for Device Flow
    const std::string DEVICE_CODE_URL = "https://oauth2.googleapis.com/device/code";
    const std::string TOKEN_URL = "https://oauth2.googleapis.com/token";
    const std::string USERINFO_URL = "https://www.googleapis.com/oauth2/v2/userinfo";
    
    // OAuth Scopes
    const std::string SCOPES = "openid email profile";
    
    // Polling interval for device flow (seconds)
    const int POLL_INTERVAL = 5;
    
    // Token storage path
    const std::string TOKEN_FILE = ".tsnake_auth";
}

#endif

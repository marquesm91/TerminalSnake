# 🔥 Firebase Setup Guide for Terminal Snake Leaderboard

This guide will help you set up Firebase for the world leaderboard.

## Prerequisites

- Google Account
- Firebase CLI (`npm install -g firebase-tools`)
- Node.js 18+

## Step 1: Create Firebase Project

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click **"Create a project"**
3. Name it: `terminalsnake-leaderboard`
4. Disable Google Analytics (optional, not needed)
5. Click **Create**

## Step 2: Enable Authentication

1. In Firebase Console, go to **Authentication** → **Sign-in method**
2. Enable **Google** provider
3. Set your support email
4. Save

## Step 3: Create OAuth 2.0 Credentials for Device Flow

1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Select your Firebase project
3. Go to **APIs & Services** → **Credentials**
4. Click **Create Credentials** → **OAuth client ID**
5. Choose **TVs and Limited Input devices**
6. Name: `Terminal Snake CLI`
7. Copy the **Client ID** (you'll need this)

## Step 4: Enable Firestore

1. In Firebase Console, go to **Firestore Database**
2. Click **Create database**
3. Choose **Production mode**
4. Select region: `us-central1`

## Step 5: Deploy Cloud Functions

```bash
cd firebase
npm install
firebase login
firebase deploy --only functions
```

## Step 6: Set Firestore Security Rules

In Firebase Console → Firestore → Rules:

```javascript
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    // Leaderboard: anyone can read, only authenticated users can write via functions
    match /leaderboard/{document=**} {
      allow read: if true;
      allow write: if false; // Only via Cloud Functions
    }
    
    // User profiles
    match /users/{userId} {
      allow read: if true;
      allow write: if request.auth != null && request.auth.uid == userId;
    }
  }
}
```

## Step 7: Configure the Game

Create `~/.tsnake_config.json`:

```json
{
  "firebase": {
    "clientId": "YOUR_CLIENT_ID.apps.googleusercontent.com",
    "projectId": "terminalsnake-leaderboard"
  }
}
```

Or set environment variables:

```bash
export TSNAKE_CLIENT_ID="YOUR_CLIENT_ID.apps.googleusercontent.com"
export TSNAKE_PROJECT_ID="terminalsnake-leaderboard"
```

## Step 8: Test the Setup

1. Build with Firebase support:
   ```bash
   make clean_arch CXXFLAGS="-DWITH_FIREBASE"
   ```

2. Run the game:
   ```bash
   ./bin/tsnake_clean
   ```

3. Go to **Sign In** and follow the Device Flow instructions

## Firebase Project Structure

```
terminalsnake-leaderboard/
├── Firestore Collections:
│   ├── leaderboard/
│   │   ├── {odUserId_difficulty}
│   │   │   ├── odUserId: string
│   │   │   ├── displayName: string
│   │   │   ├── photoUrl: string
│   │   │   ├── score: number
│   │   │   ├── snakeSize: number
│   │   │   ├── difficulty: string
│   │   │   ├── timestamp: timestamp
│   │   │   ├── verified: boolean
│   │   │   └── replayHash: string
│   │   └── ...
│   └── users/
│       ├── {odUserId}
│       │   ├── displayName: string
│       │   ├── email: string
│       │   ├── totalGames: number
│       │   └── bestScores: map
│       └── ...
└── Cloud Functions:
    ├── submitScore (validates replay, updates leaderboard)
    ├── getUserRank (gets user's rank per difficulty)
    └── getLeaderboard (paginated leaderboard fetch)
```

## Costs

Firebase has a generous free tier:
- **Firestore**: 50K reads, 20K writes, 20K deletes per day
- **Cloud Functions**: 2M invocations per month
- **Authentication**: Free

For a hobby project, you'll likely stay within free tier.

## Troubleshooting

### "OAuth error: invalid_client"
- Make sure you're using the **TVs and Limited Input devices** client type
- Check the Client ID is correct

### "Permission denied" on Firestore
- Check security rules
- Ensure Cloud Functions are deployed
- Verify authentication is working

### "Function not found"
- Run `firebase deploy --only functions`
- Check function logs: `firebase functions:log`

## Quick Setup Script

```bash
#!/bin/bash
# setup-firebase.sh

echo "🔥 Setting up Firebase for Terminal Snake..."

# Install Firebase CLI
npm install -g firebase-tools

# Login
firebase login

# Initialize project
firebase init firestore functions

# Deploy
firebase deploy

echo "✅ Firebase setup complete!"
echo "Now configure your Client ID in ~/.tsnake_config.json"
```

## Need Help?

- [Firebase Documentation](https://firebase.google.com/docs)
- [OAuth 2.0 Device Flow](https://developers.google.com/identity/protocols/oauth2/limited-input-device)
- [Open an Issue](https://github.com/marquesm91/TerminalSnake/issues)

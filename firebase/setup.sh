#!/bin/bash
# =============================================================================
# Terminal Snake - Firebase Setup Script
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "🐍 Terminal Snake - Firebase Setup"
echo "=================================="
echo ""

# Check Node.js version (requires 18+)
NODE_VERSION=$(node -v 2>/dev/null | cut -d'v' -f2 | cut -d'.' -f1)
if [ -z "$NODE_VERSION" ] || [ "$NODE_VERSION" -lt 18 ]; then
    echo "⚠️  Node.js 18+ is required (current: $(node -v 2>/dev/null || echo 'not installed'))"
    echo ""
    echo "Install Node.js using one of these methods:"
    echo ""
    echo "  # Using nvm (recommended):"
    echo "  curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.1/install.sh | bash"
    echo "  source ~/.bashrc"
    echo "  nvm install 20"
    echo "  nvm use 20"
    echo ""
    exit 1
fi

echo "✅ Node.js $(node -v) detected"

# Install Firebase CLI if needed (locally to avoid permission issues)
if ! npx firebase --version &> /dev/null; then
    echo "📦 Installing Firebase CLI locally..."
    npm install firebase-tools
fi

echo ""
echo "Step 1: Firebase Login"
echo "----------------------"
npx firebase login

echo ""
echo "Step 2: Create/Select Firebase Project"
echo "---------------------------------------"
echo ""
echo "Options:"
echo "  1. Create new project 'terminalsnake-leaderboard'"
echo "  2. Use existing project"
echo ""
read -p "Choose [1/2]: " choice

if [ "$choice" == "1" ]; then
    echo "Creating project..."
    npx firebase projects:create terminalsnake-leaderboard --display-name "Terminal Snake Leaderboard" || true
    npx firebase use terminalsnake-leaderboard
else
    npx firebase use --add
fi

echo ""
echo "Step 3: Install Function Dependencies"
echo "--------------------------------------"
cd functions
npm install
cd ..

echo ""
echo "Step 4: Deploy to Firebase"
echo "--------------------------"

# Try full deploy first, fallback to hosting only if on free plan
if ! npx firebase deploy --only firestore,functions,hosting 2>&1; then
    echo ""
    echo "⚠️  Full deploy failed (requires Blaze plan for Cloud Functions)"
    echo "   Deploying hosting only (free tier)..."
    echo ""
    npx firebase deploy --only hosting
fi

echo ""
echo "✅ Firebase setup complete!"
echo ""
echo "🌐 Your site is live at:"
echo "   https://terminalsnake-leaderboard.web.app"
echo ""
echo "📊 Cloud Functions (requires Blaze plan):"
echo "   - getLeaderboard"
echo "   - getWeeklyLeaderboard"
echo "   - validateScore"
echo "   - playerHeartbeat"
echo "   - getOnlinePlayers"
echo ""
echo "To enable Cloud Functions, upgrade to Blaze plan at:"
echo "   https://console.firebase.google.com/project/terminalsnake-leaderboard/usage/details"
echo ""
echo "Next steps for OAuth (optional):"
echo "1. Go to Google Cloud Console: https://console.cloud.google.com/"
echo "2. Select project: terminalsnake-leaderboard"
echo "3. Go to APIs & Services → Credentials"
echo "4. Create OAuth client ID (TVs and Limited Input devices)"
echo "5. Copy the Client ID"
echo ""
echo "Then configure the game:"
echo "  export TSNAKE_CLIENT_ID='your-client-id.apps.googleusercontent.com'"
echo ""

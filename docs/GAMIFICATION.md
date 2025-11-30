# Gamification & Social Features Plan

## Overview
This document outlines the plan to implement Garmin-like gamification features for Terminal Snake, including Badges, Weekly Challenges, and Social Interactions (Follow/Challenge).

## 1. Badges System (Achievements)
We will track specific events and milestones to award badges.

### Badge Types:
*   **Discovery Badges:**
    *   `First Step`: Play your first game.
    *   `Explorer`: Play on all 3 difficulty levels.
    *   `Terminal Master`: Play via SSH/Terminal client (detected via user agent or client hint).
*   **Performance Badges:**
    *   `Snake Charmer`: Score > 1000 points.
    *   `Pythonista`: Reach snake size > 50.
    *   `Unstoppable`: Play for 10 minutes in a single session.
*   **Streak Badges:**
    *   `Daily Grinder`: Play 3 days in a row.
    *   `Weekend Warrior`: Play on Saturday and Sunday.

### Implementation:
*   **Frontend (WASM/Client):** Track local stats and send "events" to Firebase upon game over.
*   **Backend (Firebase):** Cloud Functions to process events and assign badges to user profiles.
*   **UI:** New "Badges" tab in the Leaderboard section.

## 2. Weekly Challenges
Automated challenges that reset every week (Sunday UTC).

### Examples:
*   "Highest Score this Week"
*   "Total Apples Eaten this Week"
*   "Longest Survival Time"

### Implementation:
*   **Firebase:** Scheduled function to reset weekly stats.
*   **UI:** "Weekly Challenge" banner on the main page.

## 3. Social Features
Allow users to interact with each other.

### Features:
*   **Follow User:** "Star" a player to see them in a "Friends" filter on the leaderboard.
*   **Challenge:** "Beat this Score" button on a user's profile. Generates a shareable link (e.g., `?challenge=USER_ID&score=500`).
    *   When the challenged user plays via this link, the UI shows a "Target: 500" progress bar.

### Implementation:
*   **Data Structure:** `users/{userId}/following` collection.
*   **UI:** Add "Follow" button to leaderboard rows. Add "Friends" filter toggle.

## 4. Social Sharing & Bonuses
Incentivize users to share the game on social media.

### Features:
*   **Share Bonus:** Earn 500 bonus points (one-time) for sharing your high score on Twitter/X, LinkedIn, or Facebook.
*   **Referral Program:** "Invite a Friend" link. If they play, both get a "Recruiter" badge.

### Implementation:
*   **Web:** Use Web Share API or intent links (`twitter.com/intent/tweet?...`).
*   **Verification:** It's hard to verify actual sharing via API without complex permissions.
    *   *Strategy:* We will use a "trust but verify" approach (click tracking) or simply award it on button click for simplicity (common pattern in games).
*   **Terminal:** Display a URL to share: `tsnake.com/share?score=100`.

## Next Steps
1.  Update `docs/index.html` to include the **Badges UI** (mockup).
2.  Define the **Firebase Schema** for badges and social graph.
3.  Implement the **"Challenge" link logic** in the WASM game (parsing URL params).

/**
 * Firebase Cloud Functions for Terminal Snake Leaderboard
 * 
 * Setup:
 * 1. npm install -g firebase-tools
 * 2. firebase login
 * 3. firebase init functions
 * 4. Copy this file to functions/index.js
 * 5. firebase deploy --only functions
 */

const functions = require('firebase-functions');
const admin = require('firebase-admin');

admin.initializeApp();
const db = admin.firestore();

/**
 * Replay data structure (matches C++ implementation)
 */
class ReplayData {
    constructor(base64Data) {
        this.valid = false;
        this.seed = 0;
        this.difficulty = 1;
        this.boardWidth = 80;
        this.boardHeight = 24;
        this.finalScore = 0;
        this.finalSize = 3;
        this.totalFrames = 0;
        this.totalTimeMs = 0;
        this.events = [];
        this.foodSpawns = [];
        
        if (base64Data) {
            this.parse(base64Data);
        }
    }
    
    parse(base64Data) {
        try {
            const buffer = Buffer.from(base64Data, 'base64');
            
            // Check magic number "SNRP"
            if (buffer.length < 32) return;
            if (buffer.toString('utf8', 0, 4) !== 'SNRP') return;
            
            // Version check
            if (buffer[4] !== 1) return;
            
            let pos = 5;
            
            // Seed (4 bytes big-endian)
            this.seed = buffer.readUInt32BE(pos);
            pos += 4;
            
            // Difficulty
            this.difficulty = buffer[pos++];
            
            // Board size
            this.boardWidth = buffer[pos++];
            this.boardHeight = buffer[pos++];
            
            // Final score
            this.finalScore = buffer.readUInt32BE(pos);
            pos += 4;
            
            // Final size
            this.finalSize = buffer.readUInt16BE(pos);
            pos += 2;
            
            // Total frames
            this.totalFrames = buffer.readUInt32BE(pos);
            pos += 4;
            
            // Total time
            this.totalTimeMs = buffer.readUInt32BE(pos);
            pos += 4;
            
            // Number of events
            const numEvents = buffer.readUInt16BE(pos);
            pos += 2;
            
            // Parse events
            for (let i = 0; i < numEvents && pos + 7 <= buffer.length; i++) {
                const frame = buffer.readUInt32BE(pos);
                const direction = buffer.readInt8(pos + 4);
                const delta = buffer.readUInt16BE(pos + 5);
                this.events.push({ frame, direction, delta });
                pos += 7;
            }
            
            // Number of food spawns
            if (pos + 2 > buffer.length) return;
            const numFood = buffer.readUInt16BE(pos);
            pos += 2;
            
            // Parse food spawns
            for (let i = 0; i < numFood && pos + 6 <= buffer.length; i++) {
                const frame = buffer.readUInt32BE(pos);
                const x = buffer[pos + 4];
                const y = buffer[pos + 5];
                this.foodSpawns.push({ frame, x, y });
                pos += 6;
            }
            
            this.valid = true;
        } catch (error) {
            console.error('Failed to parse replay:', error);
            this.valid = false;
        }
    }
}

/**
 * Deterministic PRNG (matches C++ xorshift32)
 */
class Random {
    constructor(seed) {
        this.state = seed || 1;
    }
    
    next() {
        let x = this.state >>> 0;
        x ^= (x << 13) >>> 0;
        x ^= (x >>> 17);
        x ^= (x << 5) >>> 0;
        this.state = x >>> 0;
        return this.state;
    }
    
    range(min, max) {
        if (min >= max) return min;
        const range = max - min + 1;
        return min + (this.next() % range);
    }
}

/**
 * Snake simulation for replay validation
 */
class SnakeSimulator {
    constructor(boardWidth, boardHeight) {
        this.boardWidth = boardWidth;
        this.boardHeight = boardHeight;
        this.body = [];
        this.direction = 5; // DIR_RIGHT
    }
    
    init(startX, startY, size) {
        this.body = [];
        for (let i = 0; i < size; i++) {
            this.body.push({ x: startX, y: startY - (size - 1 - i) });
        }
        this.direction = 5; // RIGHT
    }
    
    setDirection(dir) {
        const opposites = { 2: 3, 3: 2, 4: 5, 5: 4 };
        if (dir >= 2 && dir <= 5 && opposites[this.direction] !== dir) {
            this.direction = dir;
            return true;
        }
        return false;
    }
    
    getHead() {
        return this.body[this.body.length - 1];
    }
    
    calculateNextHead() {
        const head = this.getHead();
        const newHead = { x: head.x, y: head.y };
        
        switch (this.direction) {
            case 3: newHead.x--; break; // UP
            case 2: newHead.x++; break; // DOWN
            case 4: newHead.y--; break; // LEFT
            case 5: newHead.y++; break; // RIGHT
        }
        
        return newHead;
    }
    
    move() {
        const newHead = this.calculateNextHead();
        this.body.push(newHead);
        this.body.shift();
    }
    
    grow() {
        const newHead = this.calculateNextHead();
        this.body.push(newHead);
    }
    
    collidesWithSelf(pos) {
        for (let i = 0; i < this.body.length - 1; i++) {
            if (this.body[i].x === pos.x && this.body[i].y === pos.y) {
                return true;
            }
        }
        return false;
    }
    
    containsPoint(pos) {
        return this.body.some(p => p.x === pos.x && p.y === pos.y);
    }
    
    getSize() {
        return this.body.length;
    }
}

/**
 * Validate a replay by simulating the game
 */
function validateReplay(replay) {
    if (!replay.valid) {
        return { valid: false, reason: 'Invalid replay format' };
    }
    
    const rng = new Random(replay.seed);
    const snake = new SnakeSimulator(replay.boardWidth, replay.boardHeight);
    
    // Initialize snake at center
    const startX = Math.floor(replay.boardHeight / 2);
    const startY = Math.floor(replay.boardWidth / 4);
    snake.init(startX, startY, 3);
    
    let score = 0;
    let eventIdx = 0;
    let foodIdx = 0;
    
    // Get first food position
    let foodPos = null;
    if (replay.foodSpawns.length > 0) {
        foodPos = { x: replay.foodSpawns[0].x, y: replay.foodSpawns[0].y };
        foodIdx = 1;
    }
    
    // Simulate each frame
    for (let frame = 0; frame < replay.totalFrames; frame++) {
        // Process inputs for this frame
        while (eventIdx < replay.events.length && 
               replay.events[eventIdx].frame === frame) {
            snake.setDirection(replay.events[eventIdx].direction);
            eventIdx++;
        }
        
        const nextHead = snake.calculateNextHead();
        
        // Check wall collision
        if (nextHead.x <= 0 || nextHead.x >= replay.boardHeight - 1 ||
            nextHead.y <= 0 || nextHead.y >= replay.boardWidth - 1) {
            // Game over - this is expected at the end
            break;
        }
        
        // Check self collision
        if (snake.collidesWithSelf(nextHead)) {
            break;
        }
        
        // Check food collision
        if (foodPos && nextHead.x === foodPos.x && nextHead.y === foodPos.y) {
            snake.grow();
            score += replay.difficulty;
            
            // Get next food position
            if (foodIdx < replay.foodSpawns.length) {
                foodPos = { 
                    x: replay.foodSpawns[foodIdx].x, 
                    y: replay.foodSpawns[foodIdx].y 
                };
                foodIdx++;
            } else {
                foodPos = null;
            }
        } else {
            snake.move();
        }
    }
    
    // Validate results
    if (score !== replay.finalScore) {
        return { 
            valid: false, 
            reason: `Score mismatch: expected ${replay.finalScore}, got ${score}` 
        };
    }
    
    if (snake.getSize() !== replay.finalSize) {
        return { 
            valid: false, 
            reason: `Size mismatch: expected ${replay.finalSize}, got ${snake.getSize()}` 
        };
    }
    
    return { valid: true, reason: 'OK' };
}

/**
 * Cloud Function: Validate and submit score
 * 
 * Called when a new leaderboard entry is created/updated
 */
exports.validateScore = functions.firestore
    .document('leaderboard/{odUserId}')
    .onWrite(async (change, context) => {
        const data = change.after.data();
        if (!data) return null; // Deleted
        
        const odUserId = context.params.odUserId;
        
        // Check if replay data is present
        const replayBase64 = data.replayData;
        if (!replayBase64) {
            console.log(`No replay data for user ${odUserId}`);
            // Allow entries without replay (legacy or local-only scores)
            // But mark them as unverified
            await change.after.ref.update({ verified: false });
            return null;
        }
        
        // Parse and validate replay
        const replay = new ReplayData(replayBase64);
        const validation = validateReplay(replay);
        
        if (!validation.valid) {
            console.log(`Invalid replay for user ${odUserId}: ${validation.reason}`);
            
            // Delete invalid entry
            await change.after.ref.delete();
            return null;
        }
        
        // Mark as verified
        await change.after.ref.update({ 
            verified: true,
            validatedAt: admin.firestore.FieldValue.serverTimestamp()
        });
        
        console.log(`Validated score ${data.highscore} for user ${odUserId}`);
        return null;
    });

/**
 * Cloud Function: Get top scores
 * 
 * HTTP endpoint to fetch leaderboard (only verified scores)
 */
exports.getLeaderboard = functions.https.onRequest(async (req, res) => {
    // Enable CORS for GitHub Pages
    res.set('Access-Control-Allow-Origin', '*');
    res.set('Access-Control-Allow-Methods', 'GET');
    
    if (req.method === 'OPTIONS') {
        res.status(204).send('');
        return;
    }
    
    try {
        const limit = parseInt(req.query.limit) || 20;
        const period = req.query.period || 'all'; // 'week', 'month', 'all'
        
        let query = db.collection('leaderboard')
            .where('verified', '==', true);
        
        // Filter by time period
        if (period === 'week') {
            const weekAgo = new Date(Date.now() - 7 * 24 * 60 * 60 * 1000);
            query = query.where('timestamp', '>=', weekAgo);
        } else if (period === 'month') {
            const monthAgo = new Date(Date.now() - 30 * 24 * 60 * 60 * 1000);
            query = query.where('timestamp', '>=', monthAgo);
        }
        
        const snapshot = await query
            .orderBy('timestamp', 'desc')
            .orderBy('highscore', 'desc')
            .limit(limit)
            .get();
        
        const entries = [];
        snapshot.forEach(doc => {
            const data = doc.data();
            entries.push({
                odUserId: doc.id,
                displayName: data.displayName || 'Anonymous',
                photoUrl: data.photoUrl || null,
                highscore: data.highscore || 0,
                snakeSize: data.snakeSize || 3,
                difficulty: data.difficulty || 'Normal',
                timestamp: data.timestamp ? data.timestamp.toDate().toISOString() : null
            });
        });
        
        // Sort by highscore after fetching
        entries.sort((a, b) => b.highscore - a.highscore);
        
        res.json({ 
            entries,
            period,
            fetchedAt: new Date().toISOString()
        });
    } catch (error) {
        console.error('Failed to get leaderboard:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

/**
 * HTTP endpoint for public weekly leaderboard (for GitHub Pages)
 * No authentication required - read-only public data
 */
exports.getWeeklyLeaderboard = functions.https.onRequest(async (req, res) => {
    // Enable CORS for GitHub Pages
    res.set('Access-Control-Allow-Origin', '*');
    res.set('Access-Control-Allow-Methods', 'GET');
    res.set('Cache-Control', 'public, max-age=300'); // Cache for 5 minutes
    
    if (req.method === 'OPTIONS') {
        res.status(204).send('');
        return;
    }
    
    try {
        const limit = parseInt(req.query.limit) || 10;
        const weekAgo = new Date(Date.now() - 7 * 24 * 60 * 60 * 1000);
        
        // Get top scores from the past week
        const snapshot = await db.collection('leaderboard')
            .where('verified', '==', true)
            .orderBy('highscore', 'desc')
            .limit(50) // Get more to filter by date
            .get();
        
        const entries = [];
        snapshot.forEach(doc => {
            const data = doc.data();
            const timestamp = data.timestamp ? data.timestamp.toDate() : null;
            
            // Only include entries from the past week
            if (timestamp && timestamp >= weekAgo) {
                entries.push({
                    rank: 0, // Will be set below
                    displayName: data.displayName || 'Anonymous',
                    photoUrl: data.photoUrl || null,
                    score: data.highscore || 0,
                    difficulty: data.difficulty || 'Normal',
                    timestamp: timestamp.toISOString()
                });
            }
        });
        
        // Sort by score and assign ranks
        entries.sort((a, b) => b.score - a.score);
        entries.slice(0, limit).forEach((entry, index) => {
            entry.rank = index + 1;
        });
        
        res.json({ 
            entries: entries.slice(0, limit),
            period: 'week',
            totalPlayers: entries.length,
            fetchedAt: new Date().toISOString()
        });
    } catch (error) {
        console.error('Failed to get weekly leaderboard:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

/**
 * Cloud Function: Clean up old/invalid entries
 * 
 * Runs daily to remove suspicious entries
 */
exports.cleanupLeaderboard = functions.pubsub
    .schedule('every 24 hours')
    .onRun(async (context) => {
        const cutoff = new Date();
        cutoff.setDate(cutoff.getDate() - 30); // Remove entries older than 30 days without verification
        
        const snapshot = await db.collection('leaderboard')
            .where('verified', '==', false)
            .where('timestamp', '<', cutoff)
            .get();
        
        const batch = db.batch();
        snapshot.forEach(doc => {
            batch.delete(doc.ref);
        });
        
        await batch.commit();
        console.log(`Cleaned up ${snapshot.size} old unverified entries`);
        
        return null;
    });

// ============================================================================
// Online Players Tracking
// ============================================================================

/**
 * Register player presence (heartbeat)
 * Players send heartbeat every 30 seconds
 */
exports.playerHeartbeat = functions.https.onRequest(async (req, res) => {
    res.set('Access-Control-Allow-Origin', '*');
    res.set('Access-Control-Allow-Methods', 'POST, OPTIONS');
    res.set('Access-Control-Allow-Headers', 'Content-Type');
    
    if (req.method === 'OPTIONS') {
        res.status(204).send('');
        return;
    }
    
    if (req.method !== 'POST') {
        res.status(405).json({ error: 'Method not allowed' });
        return;
    }
    
    try {
        const { playerId, platform, version } = req.body || {};
        
        if (!playerId) {
            res.status(400).json({ error: 'Missing playerId' });
            return;
        }
        
        // Update or create presence document
        await db.collection('presence').doc(playerId).set({
            lastSeen: admin.firestore.FieldValue.serverTimestamp(),
            platform: platform || 'unknown',
            version: version || '1.0.0',
            ip: req.ip || 'unknown'
        }, { merge: true });
        
        // Get current online count (last 2 minutes)
        const twoMinutesAgo = new Date(Date.now() - 2 * 60 * 1000);
        const onlineSnapshot = await db.collection('presence')
            .where('lastSeen', '>=', twoMinutesAgo)
            .count()
            .get();
        
        res.json({ 
            success: true,
            onlinePlayers: onlineSnapshot.data().count
        });
    } catch (error) {
        console.error('Heartbeat error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

/**
 * Get online players count
 */
exports.getOnlinePlayers = functions.https.onRequest(async (req, res) => {
    res.set('Access-Control-Allow-Origin', '*');
    res.set('Access-Control-Allow-Methods', 'GET');
    res.set('Cache-Control', 'public, max-age=10'); // Cache for 10 seconds
    
    if (req.method === 'OPTIONS') {
        res.status(204).send('');
        return;
    }
    
    try {
        const twoMinutesAgo = new Date(Date.now() - 2 * 60 * 1000);
        
        // Count online players
        const onlineSnapshot = await db.collection('presence')
            .where('lastSeen', '>=', twoMinutesAgo)
            .count()
            .get();
        
        // Get platform breakdown
        const presenceSnapshot = await db.collection('presence')
            .where('lastSeen', '>=', twoMinutesAgo)
            .get();
        
        const platforms = {};
        presenceSnapshot.forEach(doc => {
            const platform = doc.data().platform || 'unknown';
            platforms[platform] = (platforms[platform] || 0) + 1;
        });
        
        res.json({
            online: onlineSnapshot.data().count,
            platforms,
            timestamp: new Date().toISOString()
        });
    } catch (error) {
        console.error('Get online players error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

/**
 * Cleanup old presence records (run hourly)
 */
exports.cleanupPresence = functions.pubsub
    .schedule('every 1 hours')
    .onRun(async (context) => {
        const tenMinutesAgo = new Date(Date.now() - 10 * 60 * 1000);
        
        const snapshot = await db.collection('presence')
            .where('lastSeen', '<', tenMinutesAgo)
            .get();
        
        const batch = db.batch();
        snapshot.forEach(doc => {
            batch.delete(doc.ref);
        });
        
        await batch.commit();
        console.log(`Cleaned up ${snapshot.size} stale presence records`);
        
        return null;
    });

// ============================================================================
// Leaderboard by Category (Difficulty)
// ============================================================================

/**
 * Get leaderboard filtered by difficulty category
 */
exports.getLeaderboardByCategory = functions.https.onRequest(async (req, res) => {
    res.set('Access-Control-Allow-Origin', '*');
    res.set('Access-Control-Allow-Methods', 'GET');
    res.set('Cache-Control', 'public, max-age=60');
    
    if (req.method === 'OPTIONS') {
        res.status(204).send('');
        return;
    }
    
    try {
        const limit = parseInt(req.query.limit) || 10;
        const category = req.query.category || 'all'; // 'all', 'Easy', 'Normal', 'Hard', 'Insane'
        const period = req.query.period || 'all'; // 'week', 'month', 'all'
        
        let query = db.collection('leaderboard')
            .where('verified', '==', true);
        
        // Filter by difficulty if not 'all'
        if (category !== 'all') {
            query = query.where('difficulty', '==', category);
        }
        
        // Time period filter
        let cutoffDate = null;
        if (period === 'week') {
            cutoffDate = new Date(Date.now() - 7 * 24 * 60 * 60 * 1000);
        } else if (period === 'month') {
            cutoffDate = new Date(Date.now() - 30 * 24 * 60 * 60 * 1000);
        }
        
        const snapshot = await query
            .orderBy('highscore', 'desc')
            .limit(100) // Get more to filter
            .get();
        
        const entries = [];
        snapshot.forEach(doc => {
            const data = doc.data();
            const timestamp = data.timestamp ? data.timestamp.toDate() : null;
            
            // Apply time filter
            if (cutoffDate && timestamp && timestamp < cutoffDate) {
                return;
            }
            
            entries.push({
                rank: 0,
                displayName: data.displayName || 'Anonymous',
                photoUrl: data.photoUrl || null,
                score: data.highscore || 0,
                snakeSize: data.snakeSize || 3,
                difficulty: data.difficulty || 'Normal',
                timestamp: timestamp ? timestamp.toISOString() : null,
                verified: data.verified || false
            });
        });
        
        // Assign ranks
        entries.slice(0, limit).forEach((entry, index) => {
            entry.rank = index + 1;
        });
        
        res.json({
            entries: entries.slice(0, limit),
            category,
            period,
            totalInCategory: entries.length,
            fetchedAt: new Date().toISOString()
        });
    } catch (error) {
        console.error('Failed to get leaderboard by category:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

/**
 * Get recent scores (for real-time notifications)
 * Returns scores submitted in the last N seconds
 */
exports.getRecentScores = functions.https.onRequest(async (req, res) => {
    res.set('Access-Control-Allow-Origin', '*');
    res.set('Access-Control-Allow-Methods', 'GET');
    res.set('Cache-Control', 'no-cache');
    
    if (req.method === 'OPTIONS') {
        res.status(204).send('');
        return;
    }
    
    try {
        const seconds = parseInt(req.query.seconds) || 30;
        const limit = parseInt(req.query.limit) || 5;
        
        const cutoff = new Date(Date.now() - seconds * 1000);
        
        const snapshot = await db.collection('leaderboard')
            .where('verified', '==', true)
            .where('timestamp', '>=', cutoff)
            .orderBy('timestamp', 'desc')
            .limit(limit)
            .get();
        
        const scores = [];
        snapshot.forEach(doc => {
            const data = doc.data();
            scores.push({
                displayName: data.displayName || 'Anonymous',
                score: data.highscore || 0,
                difficulty: data.difficulty || 'Normal',
                timestamp: data.timestamp ? data.timestamp.toDate().toISOString() : null
            });
        });
        
        res.json({
            scores,
            fetchedAt: new Date().toISOString()
        });
    } catch (error) {
        console.error('Failed to get recent scores:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

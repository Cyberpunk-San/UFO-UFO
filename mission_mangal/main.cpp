#include "raylib.h"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

Texture2D monsterTexture;
Texture2D backgroundTexture;
Texture2D ufoTexture;
Texture2D asteroidTexture;
Texture2D planetTexture;      
Texture2D alienTexture;        
Texture2D shieldTexture;       
Sound shootSound;
Sound explosionSound;
Sound lifeLostSound;
Sound shieldSound;

enum GameLevel {
    LEVEL_1_ASTEROIDS,
    LEVEL_2_ALIENS,
    LEVEL_3_BOSS,
};
const int BOSS_GATE_COUNT = 7;
const float BOSS_GATE_TIME = 10.0f;
const int BOSS_HEALTH = 100;
class Bullet {
public:
    Vector2 pos;
    Vector2 velocity;
    bool active = true;
    float radius = 5.0f;
    Color color = YELLOW;
    
    Bullet(Vector2 pos, Vector2 direction) : pos(pos) {
        float length = sqrtf(direction.x*direction.x + direction.y*direction.y);
        if (length > 0) {
            velocity.x = direction.x/length * 15.0f;
            velocity.y = direction.y/length * 15.0f;
        }
        PlaySound(shootSound);
    }
    
    void Update() { 
        pos.x += velocity.x;
        pos.y += velocity.y;
        
        if (pos.x < 0 || pos.x > GetScreenWidth() || pos.y < 0 || pos.y > GetScreenHeight()) {
            active = false;
        }
    }
    
    void Draw() { 
        DrawCircleV(pos, radius, color);
    }
};

class AlienBullet {
    public:
        Vector2 pos;
        Vector2 velocity;
        bool active = true;
        float radius = 5.0f;
        Color color = RED;
        
        AlienBullet(Vector2 pos, Vector2 target) : pos(pos) {
            Vector2 direction = {target.x - pos.x, target.y - pos.y};
            float length = sqrtf(direction.x*direction.x + direction.y*direction.y);
            if (length > 0) {
                velocity.x = direction.x/length * 15.0f; 
                velocity.y = direction.y/length * 15.0f;
            }
        }
    void Update() { 
        pos.x += velocity.x;
        pos.y += velocity.y;
        
        if (pos.x < 0 || pos.x > GetScreenWidth() || pos.y < 0 || pos.y > GetScreenHeight()) {
            active = false;
        }
    }
    
    void Draw() { 
        DrawCircleV(pos, radius, color);
    }
};

class Player {
private:
    float width = 64.0f;   
    float height = 57.0f;  
    float shieldRadius = 80.0f;
    float shieldEnergy = 100.0f;
    float maxShieldEnergy = 100.0f;
    float shieldRechargeRate = 10.0f; 
    float shieldDrainRate = 30.0f;    
    bool shieldActive = false;
    
public:
    Vector2 pos = {400, 300};
    std::vector<Bullet> bullets;
    float speed = 5;
    bool invulnerable = false;
    float invulnerabilityTimer = 0.0f;
    float collisionCooldown = 0.0f;
    
    void Update() {
        // Movement controls
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) pos.y -= speed;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) pos.y += speed;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) pos.x -= speed;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) pos.x += speed;
        
        // Screen boundaries for new size
        pos.x = fmaxf(width/2, fminf(GetScreenWidth() - width/2, pos.x));
        pos.y = fmaxf(height/2, fminf(GetScreenHeight() - height/2, pos.y));
        
        // Shooting
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            bullets.emplace_back(pos, Vector2{mouse.x-pos.x, mouse.y-pos.y});
        }
        
        // Shield activation
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && shieldEnergy > 0) {
            if (!shieldActive) {
                PlaySound(shieldSound);
            }
            shieldActive = true;
        } else {
            shieldActive = false;
        }
        
        // Shield energy management
        if (shieldActive) {
            shieldEnergy -= shieldDrainRate * GetFrameTime();
            if (shieldEnergy <= 0) {
                shieldEnergy = 0;
                shieldActive = false;
            }
        } else {
            shieldEnergy += shieldRechargeRate * GetFrameTime();
            if (shieldEnergy > maxShieldEnergy) {
                shieldEnergy = maxShieldEnergy;
            }
        }
        
        // Update bullets
        for (auto& bullet : bullets) bullet.Update();
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
                     [](const Bullet& b) { return !b.active; }), bullets.end());
        
        // Update timers
        if (invulnerable) {
            invulnerabilityTimer -= GetFrameTime();
            if (invulnerabilityTimer <= 0) invulnerable = false;
        }
        
        if (collisionCooldown > 0) collisionCooldown -= GetFrameTime();
    }
    
    void Draw() {
        Vector2 mouse = GetMousePosition();
        float angle = atan2f(mouse.y - pos.y, mouse.x - pos.x);
        
        Color color = invulnerable ? 
            (sinf(invulnerabilityTimer * 20) > 0 ? WHITE : Color{255, 255, 255, 128}) : 
            WHITE;
        
        DrawTexturePro(ufoTexture,
            Rectangle{0, 0, (float)ufoTexture.width, (float)ufoTexture.height},
            Rectangle{pos.x, pos.y, width, height},
            Vector2{width/2, height/2},  
            angle * RAD2DEG,
            color);
        
        //shield if active
        if (shieldActive) {
            DrawTexturePro(shieldTexture,
                Rectangle{0, 0, (float)shieldTexture.width, (float)shieldTexture.height},
                Rectangle{pos.x, pos.y, shieldRadius*2, shieldRadius*2},
                Vector2{shieldRadius, shieldRadius},
                0,
                Color{255, 255, 255, (unsigned char)(shieldEnergy/maxShieldEnergy * 255)});
        }
        
        for (auto& bullet : bullets) bullet.Draw();
        
        // shield energy bar
        DrawRectangle(10, GetScreenHeight() - 30, (int)(200 * (shieldEnergy/maxShieldEnergy)), 20, BLUE);
        DrawRectangleLines(10, GetScreenHeight() - 30, 200, 20, WHITE);
        DrawText("Shield:", 10, GetScreenHeight() - 50, 20, WHITE);
    }
    
    void Hit() {
        if (collisionCooldown <= 0 && !shieldActive) {
            invulnerable = true;
            invulnerabilityTimer = 2.0f;
            collisionCooldown = 0.5f;
            PlaySound(lifeLostSound);
        }
    }
    
    float GetCollisionRadius() const {
        return (width + height) / 4.0f;  
    }
    
    bool IsShieldActive() const {
        return shieldActive;
    }
    
    float GetShieldRadius() const {
        return shieldRadius;
    }
};

class Asteroid {
private:
    float rotation = 0;
    float rotationSpeed = GetRandomValue(-50, 50) / 100.0f;
    
public:
    Vector2 pos;
    Vector2 velocity;
    float radius = 30;
    bool active = true;
    
    Asteroid() {
        // Random spawn position at edges
        if (GetRandomValue(0, 1) == 0) {
            pos.x = (float)GetRandomValue(0, GetScreenWidth());
            pos.y = (GetRandomValue(0, 1) ? -radius : (float)GetScreenHeight() + radius);
        } else {
            pos.y = (float)GetRandomValue(0, GetScreenHeight());
            pos.x = (GetRandomValue(0, 1) ? -radius : (float)GetScreenWidth() + radius);
        }
        
        // Movement toward center with random speed
        Vector2 target = {GetScreenWidth()/2.0f - pos.x, GetScreenHeight()/2.0f - pos.y};
        float length = sqrtf(target.x*target.x + target.y*target.y);
        if (length > 0) {
            velocity.x = target.x/length * (2.0f + GetRandomValue(0, 10)/10.0f);
            velocity.y = target.y/length * (2.0f + GetRandomValue(0, 10)/10.0f);
        }
    }
    
    void Update() {
        pos.x += velocity.x;
        pos.y += velocity.y;
        rotation += rotationSpeed;
        
        // Deactivate if out of screen
        if (pos.x < -100 || pos.x > GetScreenWidth() + 100 || 
            pos.y < -100 || pos.y > GetScreenHeight() + 100) {
            active = false;
        }
    }
    
    void Draw() {

        float aspect = (float)asteroidTexture.height/(float)asteroidTexture.width;
        float drawWidth = radius*2;
        float drawHeight = radius*2 * aspect;
        
        DrawTexturePro(asteroidTexture,
            Rectangle{0, 0, (float)asteroidTexture.width, (float)asteroidTexture.height},
            Rectangle{pos.x, pos.y, drawWidth, drawHeight},
            Vector2{drawWidth/2, drawHeight/2},
            rotation,
            WHITE);
    }
};

class Alien {
    private:
        float width = 60.0f;
        float height = 61.0f;
        float shootTimer = 0.0f;
        float shootInterval = 2.0f + GetRandomValue(0, 10)/5.0f;
        std::vector<AlienBullet> bullets;
        
    public:
        Vector2 pos;
        Vector2 velocity;
        bool active = true;
        
        Alien() {
            // Spawn from right side
            pos.x = (float)GetScreenWidth() + width/2;
            pos.y = (float)GetRandomValue(height, GetScreenHeight() - height);
            
            // Move left with random speed
            velocity.x = -(3.0f + GetRandomValue(0, 10)/5.0f);
            velocity.y = GetRandomValue(-20, 20)/10.0f;
        }
        
        void Update(Player& player) {
            pos.x += velocity.x;
            pos.y += velocity.y;
            
            // Update shooting timer
            shootTimer += GetFrameTime();
            if (shootTimer >= shootInterval) {
                bullets.emplace_back(pos, player.pos);
                shootTimer = 0;
                shootInterval = 2.0f + GetRandomValue(0, 10)/5.0f;
            }
            
            // Update bullets
            for (auto& bullet : bullets) bullet.Update();
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
                         [](const AlienBullet& b) { return !b.active; }), bullets.end());
            
            // Deactivate if out of screen
            if (pos.x < -width || pos.y < -height || pos.y > GetScreenHeight() + height) {
                active = false;
            }
        }
        
        void Draw() {
            // alien
            DrawTexturePro(alienTexture,
                Rectangle{0, 0, (float)alienTexture.width, (float)alienTexture.height},
                Rectangle{pos.x, pos.y, width, height},
                Vector2{width/2, height/2},
                0,
                WHITE);
            
            // bullets
            for (auto& bullet : bullets) bullet.Draw();
        }
        
        Rectangle GetRect() const {
            return Rectangle{pos.x - width/2, pos.y - height/2, width, height};
        }
        
        std::vector<AlienBullet>& GetBullets() {
            return bullets;
        }
    };
class BinaryGate {
        public:
            Vector2 pos;
            std::string gateType;  
            int inputA, inputB;    
            bool solved = false;
            float timer;          
            bool isFlashing = false;
        
            BinaryGate(Vector2 pos, std::string type, float timeLimit) 
                : pos(pos), gateType(type), timer(timeLimit) {
                inputA = GetRandomValue(0, 1);
                inputB = (type != "NOT") ? GetRandomValue(0, 1) : 0;
            }
        
            int GetCorrectAnswer() {
                if (gateType == "AND")  return inputA && inputB;
                if (gateType == "OR")   return inputA || inputB;
                if (gateType == "XOR")  return inputA ^ inputB;
                if (gateType == "NAND") return !(inputA && inputB);
                if (gateType == "NOR")  return !(inputA || inputB);
                if (gateType == "NOT")  return !inputA;
                if (gateType == "XNOR") return !(inputA ^ inputB);
                if (gateType == "IF A=1") return inputA == 1;
                if (gateType == "IF B=0") return inputB == 0;
                if (gateType == "IF A XOR B") return (inputA ^ inputB) == 1;
                return 0;
            }
        
            void Update() {
                if (!solved) {
                    timer -= GetFrameTime();
                    if (timer < 2.0f) isFlashing = (int)(timer * 10) % 2 == 0;
                }
            }
        
            void Draw() {
                // More visible gate design
                float radius = 40.0f;
                Color baseColor = solved ? GREEN : (isFlashing ? RED : SKYBLUE);
                
                // Main circle with outline
                DrawCircleV(pos, radius, baseColor);
                DrawCircleLines(pos.x, pos.y, radius, BLACK);
                
                // Better text display
                int fontSize = 20;
                DrawText(gateType.c_str(), 
                        pos.x - MeasureText(gateType.c_str(), fontSize)/2, 
                        pos.y - 30, 
                        fontSize, BLACK);
                        
                // inputs more clearly with labels
                if (gateType != "NOT") {
                    DrawText(TextFormat("A:%d", inputA), 
                            pos.x - 30 - MeasureText(TextFormat("%d", inputA), fontSize)/2, 
                            pos.y + 10, 
                            fontSize, BLACK);
                    DrawText(TextFormat("B:%d", inputB), 
                            pos.x + 30 - MeasureText(TextFormat("%d", inputB), fontSize)/2, 
                            pos.y + 10, 
                            fontSize, BLACK);
                } else {
                    DrawText(TextFormat("IN:%d", inputA), 
                            pos.x - MeasureText(TextFormat("IN:%d", inputA), fontSize)/2, 
                            pos.y + 10, 
                            fontSize, BLACK);
                }
                
                if (!solved) {
                    DrawText(TextFormat("%.3f", timer), 
                            pos.x - MeasureText(TextFormat("%.1f", timer), fontSize)/2, 
                            pos.y - 60, 
                            fontSize, WHITE);
                }
            }
        };
class QuantumCore {
    private:
        // Logic states
        int currentA = 0;
        int currentB = 0;
        float logicChangeTimer = 0;
        float logicChangeInterval = 5.0f;
        
        // Vulnerability
        bool isVulnerable = false;
        float vulnerableTimer = 0;
        float vulnerableDuration = 2.0f;
        float vulnerableCooldown = 8.0f;
        
        // Attacks
        float attackTimer = 0;
        float specialAttackTimer = 0;
        
        // Gates
        std::vector<BinaryGate> activeGates;
        float gateSpawnTimer = 0;
        int gatesSolvedThisPhase = 0;
        const int gatesRequiredForPhase = 4; 
        
        // Feedback system
        float feedbackTimer = 0.0f;
        const float FEEDBACK_DURATION = 0.5f;
        Vector2 lastQuestionPos = {0, 0};
        int lastCorrectAnswer = -1;
        bool showFeedback = false;
        
    public:
        Vector2 pos;
        int health = BOSS_HEALTH;
        std::vector<std::pair<std::string, float>> activeLasers;
        
        QuantumCore(int screenWidth, int screenHeight) { 
            pos = {screenWidth/2.0f, screenHeight/4.0f};
            currentA = GetRandomValue(0, 1);
            currentB = GetRandomValue(0, 1);
        }
        
        void SpawnGate() {
            if (activeGates.size() >= 3) return; 
            
            std::string gateTypes[3] = {"NAND", "NOR", "NOT"}; 
            Vector2 gatePos = {
                pos.x + cosf(gateSpawnTimer) * 200.0f,
                pos.y + 150.0f + sinf(gateSpawnTimer * 2) * 100.0f
            };
            
            std::string type = gateTypes[GetRandomValue(0, 2)]; 
            activeGates.emplace_back(gatePos, type, 10.0f); 
            gateSpawnTimer = 0.0f;
        }
        
        bool CheckGateSolution(BinaryGate& gate, int answer) {
            return answer == gate.GetCorrectAnswer();
        }
        
        void ProcessGateHit(BinaryGate& gate, int answer) {
            if (gate.solved) return;
            
            bool correct = CheckGateSolution(gate, answer);
            if (correct) {
                gate.solved = true;
                gatesSolvedThisPhase++;
                lastCorrectAnswer = gate.GetCorrectAnswer();
                showFeedback = true;
                feedbackTimer = FEEDBACK_DURATION;
                lastQuestionPos = gate.pos;
                
                // Remove the gate immediately
                activeGates.erase(std::remove_if(activeGates.begin(), activeGates.end(),
                    [](const BinaryGate& g) { return g.solved; }), activeGates.end());
                
                // Make boss vulnerable if enough gates solved
                if (gatesSolvedThisPhase >= gatesRequiredForPhase) {
                    isVulnerable = true;
                    vulnerableTimer = 0;
                    gatesSolvedThisPhase = 0;
                }
            } else {
                // Wrong answer penalty
                lastCorrectAnswer = gate.GetCorrectAnswer();
                showFeedback = true;
                feedbackTimer = FEEDBACK_DURATION;
                lastQuestionPos = gate.pos;
            }
        }
        
        bool CheckLaserCondition(const std::string& condition) {
            if (condition == "IF A=1") return currentA == 1;
            if (condition == "IF B=0") return currentB == 0;
            if (condition == "IF A XOR B") return (currentA ^ currentB) == 1;
            if (condition == "IF A AND B") return (currentA && currentB) == 1;
            if (condition == "IF A OR B") return (currentA || currentB) == 1;
            if (condition == "IF NOT A") return !currentA;
            if (condition == "IF A NAND B") return !(currentA && currentB);
            return false;
        }
        
        void LaunchAttack() {
            // Phase 2 - Double attacks
            activeLasers.emplace_back(
                (const char*[]){"IF A AND B", "IF A OR B", "IF NOT A"}[GetRandomValue(0, 2)], 
                2.5f
            );
            activeLasers.emplace_back(
                (const char*[]){"IF A AND B", "IF A OR B", "IF NOT A"}[GetRandomValue(0, 2)], 
                2.5f
            );
        }
        
        void LaunchSpecialAttack() {
            // Create a logic gate pattern attack
            std::string gateType = (const char*[]){"NAND", "NOR", "NOT"}[GetRandomValue(0, 2)];
            
            if (gateType == "NAND") {
                activeLasers.emplace_back("IF A NAND B", 4.0f);
                activeLasers.emplace_back("IF A=1", 4.0f);
                activeLasers.emplace_back("IF B=1", 4.0f);
            }
            else if (gateType == "NOR") {
                activeLasers.emplace_back("IF A NOR B", 4.0f);
                activeLasers.emplace_back("IF A=0", 4.0f);
                activeLasers.emplace_back("IF B=0", 4.0f);
            }
        }
        
        void Update() {
            if (health <= 0) return;
            
            // Change logic states periodically
            logicChangeTimer += GetFrameTime();
            if (logicChangeTimer >= logicChangeInterval) {
                currentA = GetRandomValue(0, 1);
                currentB = GetRandomValue(0, 1);
                logicChangeTimer = 0;
                logicChangeInterval = 3.0f + GetRandomValue(0, 4);
                
                // Spawn a gate when logic changes
                SpawnGate();
            }
            
            // Handle vulnerability windows
            vulnerableTimer += GetFrameTime();
            if (vulnerableTimer >= vulnerableCooldown) {
                isVulnerable = true;
                vulnerableTimer = 0;
            }
            else if (isVulnerable && vulnerableTimer >= vulnerableDuration) {
                isVulnerable = false;
            }
            
            // Spawn gates periodically
            gateSpawnTimer += GetFrameTime();
            if (gateSpawnTimer >= 4.0f) {
                SpawnGate();
            }
            
            // Update active gates
            for (auto& gate : activeGates) gate.Update();
            activeGates.erase(std::remove_if(activeGates.begin(), activeGates.end(),
                [](const BinaryGate& g) { return g.timer <= 0 && !g.solved; }), activeGates.end());
            
            // Update lasers
            for (auto it = activeLasers.begin(); it != activeLasers.end(); ) {
                it->second -= GetFrameTime();
                if (it->second <= 0) {
                    it = activeLasers.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Update feedback
            if (showFeedback) {
                feedbackTimer -= GetFrameTime();
                if (feedbackTimer <= 0) {
                    showFeedback = false;
                }
            }
            
            // Regular attacks
            attackTimer += GetFrameTime();
            if (attackTimer >= 3.0f) {
                LaunchAttack();
                attackTimer = 0;
            }
            
            // Special attacks
            specialAttackTimer += GetFrameTime();
            if (specialAttackTimer >= 15.0f) {
                LaunchSpecialAttack();
                specialAttackTimer = 0;
            }
        }
        
        bool Hit() {
            if (!isVulnerable || health <= 0) return false;
    
            // Deal 5 damage when health is <= 30, otherwise 10 damage
            health -= (health <= 30) ? 5 : 10;
            
            // Reset vulnerability after hit
            isVulnerable = false;
            vulnerableTimer = 0;
            
            if (health <= 0) {
                health = 0;
                return true;
            }
            
            return false;
        }
        
        void Draw() {
            if (health <= 0) return;
    
            // boss core with phase 2 color
            Color coreColor = Color{200, 0, 200, 255};
            if (isVulnerable) coreColor = YELLOW;
            
            // Pulsing core effect
            float pulse = sinf(GetTime() * 3.0f) * 0.2f + 1.0f;
            DrawCircleV(pos, 70 * pulse, coreColor);
            DrawCircleLines(pos.x, pos.y, 70, WHITE);
            
            // Inner core showing current logic state
            Color innerColor = (currentA || currentB) ? GREEN : RED;
            DrawCircleV(pos, 30 * pulse, innerColor);
            
            // Health bar
            DrawRectangle(pos.x - 50, pos.y - 120, 100, 10, RED);
            DrawRectangle(pos.x - 50, pos.y - 120, (int)(100 * (health/(float)BOSS_HEALTH)), 10, GREEN);
            
            // Current logic state
            DrawText(TextFormat("A:%d B:%d", currentA, currentB), 
                pos.x - 30, pos.y - 50, 20, WHITE);
            
            // phase indicator
            DrawText("PHASE 2", 
                pos.x - 30, pos.y - 90, 
                20, WHITE);
            
            // feedback for last answered question
            if (showFeedback) {
                const float pulseSize = 20.0f * (feedbackTimer / FEEDBACK_DURATION);
                Color feedbackColor = (lastCorrectAnswer == 1) ? YELLOW : RED;
                
                DrawCircleLines(lastQuestionPos.x, lastQuestionPos.y, 
                              40 + pulseSize, feedbackColor);
                DrawText(lastCorrectAnswer == 1 ? "1" : "0",
                       lastQuestionPos.x - 5, lastQuestionPos.y - 10, 
                       20, feedbackColor);
            }
            
            // Draw active gates
            for (auto& gate : activeGates) gate.Draw();
            
            // Draw active lasers
            float angleStep = 360.0f / (activeLasers.size() + 1);
            float radius = 120.0f;
            
            for (size_t i = 0; i < activeLasers.size(); i++) {
                float angle = angleStep * i * DEG2RAD;
                Vector2 laserPos = {
                    pos.x + cosf(angle) * radius,
                    pos.y + sinf(angle) * radius
                };
                
                // laser effect
                float laserPulse = sinf(GetTime() * 5) * 0.2f + 1.0f;
                Color laserColor = CheckLaserCondition(activeLasers[i].first) ? 
                                  ColorAlpha(RED, 0.8f) : 
                                  ColorAlpha(BLUE, 0.5f);
                
                DrawLineEx(laserPos, pos, 3.0f * laserPulse, laserColor);
                
                // condition text
                DrawText(activeLasers[i].first.c_str(), 
                        laserPos.x - MeasureText(activeLasers[i].first.c_str(), 15)/2, 
                        laserPos.y - 20, 
                        15, WHITE);
                        
                // timer
                float timerRadius = 15.0f * (activeLasers[i].second / 3.0f);
                DrawCircleLines(laserPos.x, laserPos.y, 15, RED);
                DrawCircle(laserPos.x, laserPos.y, timerRadius, ColorAlpha(RED, 0.5f));
            }
            
            // instructions
            if (isVulnerable) {
                DrawText("CORE VULNERABLE!", pos.x - 70, pos.y + 100, 20, YELLOW);
            } else {
                DrawText("SOLVE GATES TO MAKE CORE VULNERABLE", 
                       GetScreenWidth()/2 - MeasureText("SOLVE GATES TO MAKE CORE VULNERABLE", 20)/2, 
                       GetScreenHeight() - 50, 20, WHITE);
            }
        }
        
        std::vector<BinaryGate>& GetActiveGates() { return activeGates; }
        bool IsVulnerable() const { return isVulnerable; }
        bool IsDefeated() const { return health <= 0; }
    };
    int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "UFO Shooter");
    InitAudioDevice();
    SetTargetFPS(60);
    
    // Load assets
    backgroundTexture = LoadTexture("./resources/background.png");
    ufoTexture = LoadTexture("./resources/ufo.png");
    asteroidTexture = LoadTexture("./resources/asteroid.png");
    planetTexture = LoadTexture("./resources/planet.png");
    alienTexture = LoadTexture("./resources/alien.png");
    shieldTexture = LoadTexture("./resources/shield.png");
    shootSound = LoadSound("./resources/shoot.wav");
    explosionSound = LoadSound("./resources/explosion.wav");
    lifeLostSound = LoadSound("./resources/life_lost.wav");
    shieldSound = LoadSound("./resources/shield.wav");        
    
    Player player;
    std::vector<Asteroid> asteroids;
    std::vector<Alien> aliens;
    int score2=0;
    int score = 0;
    int lives = 5;
    float spawnTimer = 0;
    float gameTimer = 0;
    bool gameOver = false;
    bool levelComplete = false;
    int highScore = 0;
    GameLevel currentLevel = LEVEL_1_ASTEROIDS;
    
    // Level 3 specific variables
    std::vector<BinaryGate> gates;
    bool bossPhase = false;
    QuantumCore boss(screenWidth, screenHeight);
    bool bossDefeated = false;
    int gatesSolved = 0;
    float gateSpawnTimer = 0.0f;
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        if (!gameOver && !levelComplete) {
            gameTimer += deltaTime;
            spawnTimer += deltaTime;
            
            // Level-specific updates
            switch (currentLevel) {
                case LEVEL_1_ASTEROIDS: {
                    // Level 1 - Asteroids
                    float spawnInterval = fmaxf(0.3f, 1.0f - gameTimer / 60.0f);
                    if (spawnTimer >= spawnInterval) {
                        asteroids.emplace_back();
                        spawnTimer = 0;
                    }
                    
                    for (auto& asteroid : asteroids) asteroid.Update();
                    asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(), 
                                   [](const Asteroid& a) { return !a.active; }), asteroids.end());
                    
                    // Bullet-asteroid collision
                    for (auto b = player.bullets.begin(); b != player.bullets.end(); ) {
                        bool hit = false;
                        for (auto a = asteroids.begin(); a != asteroids.end(); ) {
                            if (CheckCollisionCircles(b->pos, b->radius, a->pos, a->radius)) {
                                score += 10;
                                a = asteroids.erase(a);
                                hit = true;
                                PlaySound(explosionSound);
                                break;
                            } else {
                                ++a;
                            }
                        }
                        if (hit) {
                            b = player.bullets.erase(b);
                        } else {
                            ++b;
                        }
                    }
                    
                    // Player-asteroid collision
                    if (!player.invulnerable && player.collisionCooldown <= 0) {
                        for (auto a = asteroids.begin(); a != asteroids.end(); ) {
                            if (CheckCollisionCircles(player.pos, player.GetCollisionRadius(), a->pos, a->radius)) {
                                if (!player.IsShieldActive()) {
                                    lives--;
                                    player.Hit();
                                }
                                
                                Vector2 diff = {player.pos.x - a->pos.x, player.pos.y - a->pos.y};
                                float length = sqrtf(diff.x*diff.x + diff.y*diff.y);
                                if (length > 0) {
                                    Vector2 pushDirection = {diff.x/length, diff.y/length};
                                    player.pos.x += pushDirection.x * 20.0f;
                                    player.pos.y += pushDirection.y * 20.0f;
                                    a->pos.x -= pushDirection.x * 20.0f;
                                    a->pos.y -= pushDirection.y * 20.0f;
                                    a->velocity.x *= 0.7f;
                                    a->velocity.y *= 0.7f;
                                }
                                
                                if (lives <= 0) {
                                    if (score >= 500) {
                                        levelComplete = true;
                                        score2 = score;
                                    } else {
                                        gameOver = true;
                                    }
                                    highScore = fmax(highScore, score);
                                }
                                break;
                            } else {
                                ++a;
                            }
                        }
                    }
                    break;
                }
                
                case LEVEL_2_ALIENS: {
                    // Level 2 - Aliens
                    float spawnInterval = fmaxf(0.5f, 2.0f - gameTimer / 60.0f);
                    if (spawnTimer >= spawnInterval) {
                        aliens.emplace_back();
                        spawnTimer = 0;
                    }
                    
                    for (auto& alien : aliens) alien.Update(player);
                    aliens.erase(std::remove_if(aliens.begin(), aliens.end(), 
                                   [](const Alien& a) { return !a.active; }), aliens.end());
                    
                    // Bullet-alien collision
                    for (auto b = player.bullets.begin(); b != player.bullets.end(); ) {
                        bool hit = false;
                        for (auto a = aliens.begin(); a != aliens.end(); ) {
                            if (CheckCollisionCircleRec(b->pos, b->radius, a->GetRect())) {
                                score += 20;
                                a = aliens.erase(a);
                                hit = true;
                                PlaySound(explosionSound);
                                break;
                            } else {
                                ++a;
                            }
                        }
                        if (hit) {
                            b = player.bullets.erase(b);
                        } else {
                            ++b;
                        }
                    }
                    
                    // Player-alien collision
                    if (!player.invulnerable && player.collisionCooldown <= 0) {
                        for (auto a = aliens.begin(); a != aliens.end(); ) {
                            if (CheckCollisionCircleRec(player.pos, player.GetCollisionRadius(), a->GetRect())) {
                                if (!player.IsShieldActive()) {
                                    lives--;
                                    player.Hit();
                                }
                                
                                if (lives <= 0) {
                                    if (score >= score+500) {
                                        levelComplete = true;
                                    } else {
                                        gameOver = true;
                                    }
                                    highScore = fmax(highScore, score);
                                }
                                break;
                            } else {
                                ++a;
                            }
                        }
                    }
                    
                    // Alien bullet collision with player
                    for (auto& alien : aliens) {
                        for (auto& bullet : alien.GetBullets()) {
                            if (CheckCollisionCircles(bullet.pos, bullet.radius, player.pos, 
                                                    player.IsShieldActive() ? player.GetShieldRadius() : player.GetCollisionRadius())) {
                                if (!player.IsShieldActive()) {
                                    lives--;
                                    player.Hit();
                                }
                                bullet.active = false;
                                
                                if (lives <= 0) {
                                    if (score >= score2+500) {
                                        levelComplete = true;
                                    } else {
                                        gameOver = true;
                                    }
                                    highScore = fmax(highScore, score);
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
                
                case LEVEL_3_BOSS: {
                    // Level 3 - Boss
                    gateSpawnTimer += deltaTime;
                    
                    // Spawn gates if not in boss phase
                    if (!bossPhase) {
                        if (gates.empty() || (gateSpawnTimer >= 5.0f && gates.size() < BOSS_GATE_COUNT)) {
                            std::string gateTypes[7] = {"AND", "OR", "XOR", "NAND", "NOR", "NOT", "XNOR"};
                            Vector2 gatePos = {
                                (float)GetRandomValue(100, screenWidth - 100),
                                (float)GetRandomValue(150, screenHeight - 150)
                            };
                            gates.emplace_back(gatePos, gateTypes[GetRandomValue(0, 6)], BOSS_GATE_TIME);
                            gateSpawnTimer = 0.0f;
                        }
                    }
                    
                    // Update gates
                    for (auto& gate : gates) gate.Update();
                    
                    // Count solved gates
                    gatesSolved = 0;
                    for (auto& gate : gates) {
                        if (gate.solved) gatesSolved++;
                    }
                    
                    // Transition to boss phase when all gates solved
                    if (gatesSolved >= BOSS_GATE_COUNT && !bossPhase) {
                        bossPhase = true;
                        gates.clear();
                    }
                    
                    // Boss phase logic
                    if (bossPhase && !bossDefeated) {
                        boss.Update();
                        
                        // bullet-boss collision
                        for (auto b = player.bullets.begin(); b != player.bullets.end(); ) {
                            bool bulletHit = false;
                            
                            // bullet hit the boss core
                            if (CheckCollisionCircles(b->pos, b->radius, boss.pos, 70)) {
                                if (boss.Hit()) {
                                    bossDefeated = true;
                                    score += 1000;
                                    PlaySound(explosionSound);
                                } else {
                                    score += 50;
                                }
                                bulletHit = true;
                            }
                            // bullet hit boss gates
                            else {
                                for (auto& gate : boss.GetActiveGates()) {
                                    if (!gate.solved && CheckCollisionCircles(b->pos, b->radius, gate.pos, 40)) {
                                        int answer = (b->pos.y < gate.pos.y) ? 1 : 0; // Top = 1, Bottom = 0
                                        boss.ProcessGateHit(gate, answer);
                                        bulletHit = true;
                                        break;
                                    }
                                }
                            }
                            
                            if (bulletHit) {
                                b = player.bullets.erase(b);
                            } else {
                                ++b;
                            }
                        }
                        
                        // Boss attacks player
                        if (!player.invulnerable && CheckCollisionCircles(player.pos, player.GetCollisionRadius(), boss.pos, 70)) {
                            if (!player.IsShieldActive()) {
                                lives--;
                                player.Hit();
                                if (lives <= 0) {
                                    gameOver = true;
                                    highScore = fmax(highScore, score);
                                }
                            }
                        }
                        
                        // boss laser conditions
                        for (auto& laser : boss.activeLasers) {
                            if (boss.CheckLaserCondition(laser.first)) {
                                if (CheckCollisionCircles(player.pos, player.GetCollisionRadius(), boss.pos, 
                                                        player.IsShieldActive() ? player.GetShieldRadius() : player.GetCollisionRadius())) {
                                    if (!player.IsShieldActive()) {
                                        lives--;
                                        player.Hit();
                                        if (lives <= 0) {
                                            gameOver = true;
                                            highScore = fmax(highScore, score);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // bullet-gate collisions (only if not in boss phase)
                    if (!bossPhase) {
                        for (auto b = player.bullets.begin(); b != player.bullets.end(); ) {
                            bool hit = false;
                            for (auto& gate : gates) {
                                if (!gate.solved && CheckCollisionCircles(b->pos, b->radius, gate.pos, 40)) {
                                    int answer = (b->pos.y < gate.pos.y) ? 1 : 0; // Top = 1, Bottom = 0
                                    if (answer == gate.GetCorrectAnswer()) {
                                        gate.solved = true;
                                        score += 100;
                                        PlaySound(explosionSound);
                                    } else {
                                        lives--;
                                        aliens.emplace_back();
                                        PlaySound(lifeLostSound);
                                    }
                                    hit = true;
                                    break;
                                }
                            }
                            if (hit) b = player.bullets.erase(b);
                            else ++b;
                        }
                    }
                    
                    for (auto& alien : aliens) alien.Update(player);
                    aliens.erase(std::remove_if(aliens.begin(), aliens.end(), 
                                [](const Alien& a) { return !a.active; }), aliens.end());
                    
                    for (auto& alien : aliens) {
                        for (auto& bullet : alien.GetBullets()) {
                            if (CheckCollisionCircles(bullet.pos, bullet.radius, player.pos, 
                                                    player.IsShieldActive() ? player.GetShieldRadius() : player.GetCollisionRadius())) {
                                if (!player.IsShieldActive()) {
                                    lives--;
                                    player.Hit();
                                    if (lives <= 0) {
                                        gameOver = true;
                                        highScore = fmax(highScore, score);
                                    }
                                }
                                bullet.active = false;
                                break;
                            }
                        }
                    }
                    
                    if (bossDefeated && aliens.empty()) {
                        levelComplete = true;
                        highScore = fmax(highScore, score);
                    }
                    break;
                }
            }
            
            player.Update();
        } else if (levelComplete) {
            // Level complete screen
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                Rectangle nextLevelButton = {screenWidth/2 - 100, 350, 200, 50};
                
                if (CheckCollisionPointRec(mouse, nextLevelButton)) {
                    levelComplete = false;
                    
                    // lives for next level 
                    int newLives = fmax(1, score / 100);
                    
                    switch (currentLevel) {
                        case LEVEL_1_ASTEROIDS:
                            currentLevel = LEVEL_2_ALIENS;
                            player = Player();
                            asteroids.clear();
                            lives = newLives;
                            spawnTimer = 0;
                            gameTimer = 0;
                            break;
                        case LEVEL_2_ALIENS:
                            currentLevel = LEVEL_3_BOSS;
                            player = Player();
                            aliens.clear();
                            gates.clear();
                            bossPhase = false;
                            bossDefeated = false;
                            boss.health = BOSS_HEALTH;
                            gatesSolved = 0;
                            lives = newLives;
                            spawnTimer = 0;
                            gameTimer = 0;
                            break;
                        case LEVEL_3_BOSS:
                            // Game is complete, restart from level 1
                            currentLevel = LEVEL_1_ASTEROIDS;
                            player = Player();
                            asteroids.clear();
                            aliens.clear();
                            gates.clear();
                            bossPhase = false;
                            bossDefeated = false;
                            score = 0;
                            lives = 5;
                            spawnTimer = 0;
                            gameTimer = 0;
                            break;
                    }
                }
            }
        } else if (gameOver) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                Rectangle restartButton = {screenWidth/2 - 100, 350, 200, 50};
                
                if (CheckCollisionPointRec(mouse, restartButton)) {
                    // Reset game state
                    player = Player();
                    asteroids.clear();
                    aliens.clear();
                    gates.clear();
                    bossPhase = false;
                    bossDefeated = false;
                    boss.health = BOSS_HEALTH;
                    gatesSolved = 0;
                    score = 0;
                    lives = 5;
                    gameOver = false;
                    levelComplete = false;
                    currentLevel = LEVEL_1_ASTEROIDS;
                    spawnTimer = 0;
                    gameTimer = 0;
                }
            }
        }
        
        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (!gameOver && !levelComplete) {
            switch (currentLevel) {
                case LEVEL_1_ASTEROIDS:
                    DrawTexturePro(backgroundTexture,
                        Rectangle{0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                        Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                        Vector2{0, 0},
                        0,
                        WHITE);
                    break;
                case LEVEL_2_ALIENS:
                    DrawTexturePro(planetTexture,
                        Rectangle{0, 0, (float)planetTexture.width, (float)planetTexture.height},
                        Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                        Vector2{0, 0},
                        0,
                        WHITE);
                    break;
                case LEVEL_3_BOSS:
                    // Space background 
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, Color{10, 10, 30, 255}, Color{30, 10, 50, 255});
                    
                    // Stars
                    for (int i = 0; i < 150; i++) {
                        float x = sinf(gameTimer + i) * screenWidth/2 + screenWidth/2;
                        float y = cosf(gameTimer * 0.5f + i * 2) * screenHeight/2 + screenHeight/2;
                        float size = 2.0f + sinf(gameTimer * 2 + i) * 1.0f;
                        DrawCircle(x, y, size, WHITE);
                    }
                    break;
            }
            
            // level-specific entities
            switch (currentLevel) {
                case LEVEL_1_ASTEROIDS:
                    for (auto& asteroid : asteroids) asteroid.Draw();
                    break;
                case LEVEL_2_ALIENS:
                    for (auto& alien : aliens) alien.Draw();
                    break;
                case LEVEL_3_BOSS:
                    // gates if not in boss phase
                    if (!bossPhase) {
                        for (auto& gate : gates) gate.Draw();
                        DrawText(TextFormat("Gates: %d/%d", gatesSolved, BOSS_GATE_COUNT), 10, 130, 20, YELLOW);
                    }
                    
                    // boss if in boss phase
                    if (bossPhase && !bossDefeated) {
                        boss.Draw();
                        DrawText(TextFormat("BOSS: %d/%d HP", boss.health, BOSS_HEALTH), 10, 130, 20, RED);
                    }
                    
                    // aliens if any exist
                    for (auto& alien : aliens) alien.Draw();
                    
                    // instructions
                    if (!bossPhase) {
                        DrawText("SHOOT GATES TO SOLVE LOGIC PUZZLES", 100, 50, 20, WHITE);
                        DrawText("TOP HALF = 1 (TRUE)", screenWidth/2 - 100, 80, 20, GREEN);
                        DrawText("BOTTOM HALF = 0 (FALSE)", screenWidth/2 - 120, 110, 20, RED);
                    } else if (!bossDefeated) {
                        DrawText("DESTROY THE QUANTUM CORE!", screenWidth/2 - 150, 50, 20, WHITE);
                    }
                    break;
            }
            
            player.Draw();
            
            // HUD
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Lives: %d", lives), 10, 40, 20, WHITE);
            DrawText(TextFormat("Time: %.1f", gameTimer), 10, 70, 20, WHITE);
            DrawText(TextFormat("Level: %d", currentLevel + 1), 10, 100, 20, WHITE);
            
            // score needed for next level
            if (currentLevel == LEVEL_1_ASTEROIDS) {
                DrawText(TextFormat("Target: %d", 500), 10, 130, 20, score >= 500 ? GREEN : YELLOW);
            } else if (currentLevel == LEVEL_2_ALIENS) {
                DrawText(TextFormat("Target: %d", score2+500), 10, 130, 20, score >= score2+500 ? GREEN : YELLOW);
            }
            
            // Lives for next level
            if ((currentLevel == LEVEL_1_ASTEROIDS && score >= 500) || 
                (currentLevel == LEVEL_2_ALIENS && score >= score2+500)) {
                int projectedLives = fmax(1, score / 100);
                DrawText(TextFormat("Next Level Lives: %d", projectedLives), 10, 160, 20, GREEN);
            }
        } else if (levelComplete) {
            // Level complete screen
            DrawText("LEVEL COMPLETE!", screenWidth/2 - MeasureText("LEVEL COMPLETE!", 40)/2, 150, 40, GREEN);
            DrawText(TextFormat("Final Score: %d", score), 
                   screenWidth/2 - MeasureText(TextFormat("Final Score: %d", score), 30)/2, 220, 30, WHITE);
            
            // Lives in next level
            int nextLives = fmax(1, score / 100);
            DrawText(TextFormat("You earned %d lives for next level", nextLives),
                   screenWidth/2 - MeasureText(TextFormat("You earned %d lives for next level", nextLives), 20)/2,
                   260, 20, YELLOW);
            
            // Next level button
            DrawRectangle(screenWidth/2 - 100, 350, 200, 50, GREEN);
            if (currentLevel == LEVEL_3_BOSS) {
                DrawText("PLAY AGAIN", screenWidth/2 - MeasureText("PLAY AGAIN", 20)/2, 365, 20, BLACK);
            } else {
                DrawText("NEXT LEVEL", screenWidth/2 - MeasureText("NEXT LEVEL", 20)/2, 365, 20, BLACK);
            }
            DrawText("Click to continue", screenWidth/2 - MeasureText("Click to continue", 20)/2, 420, 20, WHITE);
        } else if (gameOver) {
            // Game over screen
            DrawText("GAME OVER! ", screenWidth/2 - MeasureText("GAME OVER", 40)/2, 150, 40, RED);
            DrawText(TextFormat("Final Score: %d", score), 
                   screenWidth/2 - MeasureText(TextFormat("Final Score: %d", score), 30)/2, 220, 30, WHITE);
            DrawText(TextFormat("High Score: %d", highScore), 
                   screenWidth/2 - MeasureText(TextFormat("High Score: %d", highScore), 30)/2, 260, 30, YELLOW);
            
            // Restart button
            DrawRectangle(screenWidth/2 - 100, 350, 200, 50, GREEN);
            DrawText("RESTART", screenWidth/2 - MeasureText("RESTART", 20)/2, 365, 20, BLACK);
            DrawText("Click to restart", screenWidth/2 - MeasureText("Click to restart", 20)/2, 420, 20, WHITE);
        }
        
        EndDrawing();
    }
    
    // Cleanup
    UnloadTexture(backgroundTexture);
    UnloadTexture(ufoTexture);
    UnloadTexture(asteroidTexture);
    UnloadTexture(planetTexture);
    UnloadTexture(alienTexture);
    UnloadTexture(shieldTexture);
    UnloadTexture(monsterTexture);
    UnloadSound(shootSound);
    UnloadSound(explosionSound);
    UnloadSound(lifeLostSound);
    UnloadSound(shieldSound);
    CloseAudioDevice();
    CloseWindow();  
    return 0;
}
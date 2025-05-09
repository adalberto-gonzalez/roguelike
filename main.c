/*******************************************************************************************
*
*   Roguelike (Vampire survivors)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define MAX_ORBS 256
#define ORB_RADIUS 70.0f
#define MAX_ENEMIES 256
#define ENEMY_RADIUS 15.0f
#define MAX_PROJECTILES 256
#define PROJECTILE_RADIUS 5.0f
#define PROJECTILE_SPEED 4.0f
#define MAX_EN_ANI 64

const Color Naranja = (Color){ 255, 111, 0, 255 }; // Ensure this color is correctly initialized
const Color RojoOscuro = (Color) { 198, 18, 0, 255 };
const Color VerdeOscuro = (Color) { 44, 66, 55, 255 };
const Color AzulOscuro = (Color) { 20, 30, 45, 255 };
const Color Amarillo = (Color) { 255, 204, 0, 255 };
const Color Crema = (Color) { 255, 240, 220, 255 };


typedef struct Player {
    Vector2 position;
    float speed;
    float acceleration;
    float radius;
    int health;
    int damage;
    int level;
    int experience;
    int maxHealth;
} Player;

typedef struct Orb{
    Vector2 position;
    Color color;
    float radius;
    bool enabled;
} Orb;

typedef struct Enemy{
    Vector2 position;
    float speed;
    float radius;
    int health;
    float maxHealth;
    bool enabled;
} Enemy;

typedef struct Projectile{
    Vector2 position;
    float speed;
    float radius;
    int damage;
    bool enabled;
    Vector2 direction;
} Projectile;

typedef struct Ability {
    Texture2D icon;
    const char *name;
    const char *description;
} Ability;

typedef struct Animation {
    Vector2 position;
    int currentFrame;
    int frames;
    float elapsedTime;
    bool active;
    Color tint;
} Animation;

//----------------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------------
Camera2D camera = { 0 };
Player player = { 0 };
Vector2 squarePosition = { 0 };
Vector2 mousePosition = (Vector2){ 0 };
float radiusMultiplier = 1.0f;
Orb orbs[MAX_ORBS] = { 0 };
int orbsCount = 0;
Enemy enemies[MAX_ENEMIES] = { 0 };
int enemiesCount = 0;
Projectile projectiles[MAX_PROJECTILES] = { 0 };
int projectilesCount = 0;
Animation enemyAnimations[MAX_EN_ANI] = { 0 };
int enemyAnimationsCount = 0;

// Estadisticas
int enemiesKilled = 0;
int orbsCollected = 0;
int projectilesFired = 0;
int projectilesHit = 0;

// Habilidades
bool selectedIndex = false;
int abilityDamage = 20;
float abilityMultiplier = 1.0f;
Ability abilities[18] = { 0 };
Ability acquiredAbilities[18] = { 0 };
bool resurrect = false;
bool resurrected = false;
float shootVelocity = 1.0f;

//UI
bool upgradeMenu = false;
bool deathScreen = false;
int selectedAbility = 0;
int index[3] = { 0 };

// Textures
Texture2D noiseTexture;
Texture2D crosshair;
Texture2D uiCorner;
Texture2D bullet;
Texture2D saw;
Texture2D skullSmoke[6];

// Sound & Music
Music music = { 0 };
Sound shoot = { 0 };

//----------------------------------------------------------------------------------
// Funciones
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);          // Update and draw one frame
void GenOrbs(Vector2 position, int amount);
void GenEnemies(Vector2 position, int amount);
void GenProjectiles(Vector2 position, Vector2 direction, int amount);
void DrawOrbs(Orb *orbs, int amount);
void DrawEnemies(Enemy *enemies, int amount);
void DrawProjectiles(Projectile *projectiles, int amount);
void OrbCollision(Orb *orbs);
void EnemyCollision(Enemy *enemies);
void ProjectileCollision(Projectile *projectiles, Enemy *enemies);
void PlayerTakeDamage(int damage);
void PushEnemiesAway(Enemy *enemies);
void enemiesSpawn(Enemy *enemies);
void UpdateProjectiles(Projectile *projectiles, int amount);
void enemyTrigger(Enemy *enemies, Vector2 position);
void ally(Enemy *enemies);
void enableUpgradeMenu();
void disableUpgradeMenu();
void UpdateDrawAnimations(Animation *animations, int amount, Texture2D textures[]);
void startEnemyAnimation(Vector2 position, Color tint);
//----------------------------------------------------------------------------------
// Main
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 720;
    
    if(GetMonitorHeight(0) > 1280 || GetMonitorWidth(0) > 720) {
        screenWidth = GetMonitorWidth(0); 
        screenHeight = GetMonitorHeight(0);
    }

    // Player
    player.position = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    player.speed = 2.0f;
    player.acceleration = 1.0f;
    player.radius = 10.0f;
    player.health = 5;
    player.damage = 10;
    player.level = 1;
    player.experience = 0;
    player.maxHealth = 5;

    // Habilidades
    abilities[0].icon = LoadTexture("textures/ability1.png");
    abilities[0].name = "Resurrect";
    abilities[0].description = "Resucita al morir";
    abilities[1].icon = LoadTexture("textures/ability2.png");
    abilities[1].name = "Disparo Mejorado";
    abilities[1].description = "Incrementa el daño un 10%%";
    abilities[2].icon = LoadTexture("textures/ability3.png");
    abilities[2].name = "Movimiento Agil";
    abilities[2].description = "Incrementa la velocidad un 10%%";
    abilities[3].icon = LoadTexture("textures/ability4.png");
    abilities[3].name = "Regeneración";
    abilities[3].description = "Cura 1 vida cada 2 minutos";
    abilities[4].icon = LoadTexture("textures/ability5.png");
    abilities[4].name = "Multidisparo";
    abilities[4].description = "Dispara un proyectil extra";
    abilities[5].icon = LoadTexture("textures/ability6.png");
    abilities[5].name = "Mejores amigos";
    abilities[5].description = "Invoca un aliado que dispara un \nproyectil cada 0.5 segundos";
    abilities[6].icon = LoadTexture("textures/ability7.png");
    abilities[6].name = "Tormenta de Balas";
    abilities[6].description = "Dispara 6 proyectiles en todas \ndirecciones cada 3 segundos";
    abilities[7].icon = LoadTexture("textures/ability8.png");
    abilities[7].name = "Berserker";
    abilities[7].description = "Incrementa el daño un 50%% y la \nvelocidad un 25%% durante 30 segundos después de recibir daño";
    abilities[8].icon = LoadTexture("textures/ability9.png");
    abilities[8].name = "Explosión";
    abilities[8].description = "Causa 10 de daño a todos los enemigos \ncercanos al recibir daño";
    abilities[9].icon = LoadTexture("textures/ability10.png");
    abilities[9].name = "Iman de Orbes";
    abilities[9].description = "Aumenta el rango de recogida de orbes un 25%%";
    abilities[10].icon = LoadTexture("textures/ability11.png");
    abilities[10].name = "Disparo Rápido";
    abilities[10].description = "Incrementa la velocidad de disparo un 25%%";
    abilities[11].icon = LoadTexture("textures/ability12.png");
    abilities[11].name = "Desde la tumba";
    abilities[11].description = "Los enemigos muertos disparan 3 \nproyectiles al morir";

    InitWindow(screenWidth, screenHeight, "Roguelike");
    InitAudioDevice();

    // Carga de texturas
    noiseTexture = LoadTexture("textures/noise_overlay.png");
    crosshair = LoadTexture("textures/crosshair.png");
    uiCorner = LoadTexture("textures/ui_corner.png");
    bullet = LoadTexture("textures/bullet.png");
    saw = LoadTexture("textures/saw.png");

    for (int i = 0; i < (sizeof(skullSmoke) / sizeof(skullSmoke[0])); i++)
    {
        skullSmoke[i] = LoadTexture(TextFormat("textures/Skull_Smoke/%d.png", i+1));
    }
    

    // Carga de sonidos
    music = LoadMusicStream("sound/ganymede.ogg");
    shoot = LoadSound("sound/shoot.ogg");
    PlayMusicStream(music);

    // Inicializar orbs, enemies, projectiles
    for (int i = 0; i < MAX_ORBS; i++) {
        orbs[i].position = (Vector2){ -100000, -100000 };
        orbs[i].radius = ORB_RADIUS  * radiusMultiplier;
        orbs[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        orbs[i].enabled = false;
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].position = (Vector2){ -100000, -100000 };
        enemies[i].radius = ENEMY_RADIUS;
        enemies[i].health = 50;
        enemies[i].maxHealth = 50;
        enemies[i].speed = 2.0f;
        enemies[i].enabled = false;
    }
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].position = (Vector2){ -100000, -100000 };
        projectiles[i].radius = PROJECTILE_RADIUS;
        projectiles[i].speed = PROJECTILE_SPEED;
        projectiles[i].enabled = false;
    }
    for (int i = 0; i < MAX_EN_ANI; i++) {
        enemyAnimations[i].position = (Vector2){ -100000, -100000 };
        enemyAnimations[i].currentFrame = 0;
        enemyAnimations[i].frames = 6;
        enemyAnimations[i].elapsedTime = 0.0f;
        enemyAnimations[i].active = false;
        enemyAnimations[i].tint = WHITE;
    }

    camera.target = (Vector2){ player.position.x, player.position.y };
    camera.offset = (Vector2){ (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    camera.zoom = 1.0f;
    //ToggleFullscreen();
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }

    CloseWindow();                  // Close window and OpenGL context
    CloseAudioDevice();           // Close audio device
    UnloadMusicStream(music); // Unload music stream
    UnloadTexture(noiseTexture); // Unload texture
    UnloadTexture(crosshair); // Unload texture
    UnloadTexture(uiCorner); // Unload texture

    return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    camera.target = (Vector2){ player.position.x, player.position.y };
    Vector2 direction = (Vector2){ 0, 0 };
    
    mousePosition = GetMousePosition();
    UpdateMusicStream(music);

    if(player.health <= 0 && !resurrect) {
        deathScreen = true;
    }
    if(player.experience >= player.level * 10) {
        player.level += 1;
        player.experience = 0;
        upgradeMenu = true;
    }

    // Timers
    static float timer = 0.0f;
    static float totalGameTime = 0.0f;
    static float timeSinceLastClick = 0.0f;
    static float SpawnTimer = 0.0f;
    timeSinceLastClick += GetFrameTime();
    totalGameTime += GetFrameTime();
    timer += GetFrameTime();
    SpawnTimer += GetFrameTime();
    
    if (timer >= 0.5f) {
        timer = 0.0f;
        ally(enemies);
    }

    // Spawner
    if (SpawnTimer >= (-0.195*SpawnTimer)+1 && (-0.195*SpawnTimer)+1 > 0) {
        SpawnTimer = 0.0f;
        enemiesSpawn(enemies);
        enemiesSpawn(enemies);
        enemiesCount += 2;
    }

    if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !upgradeMenu && !deathScreen) {
        if (enemiesCount < MAX_ENEMIES) {
            GenOrbs(GetScreenToWorld2D(mousePosition, camera), 1);
            orbsCount++;
        }
    }
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && timeSinceLastClick >= 0.3f*shootVelocity && !upgradeMenu && !deathScreen) {
        timeSinceLastClick = 0.0f;
        if (projectilesCount < MAX_PROJECTILES) {
            GenProjectiles(player.position, GetScreenToWorld2D(mousePosition, camera), 1);
            projectilesCount++;
            PlaySound(shoot);
        }else{
            projectilesCount = 0;
        }
    }
    HideCursor();

    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    BeginMode2D(camera);
        ClearBackground(AzulOscuro);

        //Player
        DrawRectangleRounded((Rectangle){ player.position.x - 10.0f, player.position.y - 20.0f, 20, 40 }, 1.0f, 10, Naranja);
        DrawRing((Vector2){ player.position.x, player.position.y }, player.radius - 2, player.radius, 0, 360, 32, VerdeOscuro);
        
        DrawOrbs(orbs, MAX_ORBS);
        DrawEnemies(enemies, MAX_ENEMIES);
        DrawProjectiles(projectiles, MAX_PROJECTILES);

        if(!upgradeMenu && !deathScreen) {
            UpdateProjectiles(projectiles, MAX_PROJECTILES);
            UpdateDrawAnimations(enemyAnimations, MAX_EN_ANI, skullSmoke);
            // Collision logic
            OrbCollision(orbs);
            EnemyCollision(enemies);
            ProjectileCollision(projectiles, enemies);
            if(IsKeyDown(KEY_W)) direction.y -= player.speed * player.acceleration;
            if(IsKeyDown(KEY_S)) direction.y += player.speed * player.acceleration;
            if(IsKeyDown(KEY_A)) direction.x -= player.speed * player.acceleration;
            if(IsKeyDown(KEY_D)) direction.x += player.speed * player.acceleration;
            Vector2Normalize(direction);
            player.position.x = Clamp(player.position.x, -5000.0f, 5000.0f);
            player.position.y = Clamp(player.position.y, -5000.0f, 5000.0f);
            player.position = Vector2Add(player.position, direction);
        }

        // limit
        DrawRectangleLines(-5000, -5000, 10000, 10000, RojoOscuro);
        
        if(1){
            static float rotationAngle = 0.0f;
            rotationAngle += 1.0f * 3.0f;

            Vector2 orbitPosition = {
                player.position.x + 100 * cosf(DEG2RAD * rotationAngle),
                player.position.y + 100 * sinf(DEG2RAD * rotationAngle)
            };

            DrawTexturePro(saw, 
                (Rectangle){ 0, 0, (float)saw.width, (float)saw.height }, 
                (Rectangle){ orbitPosition.x - saw.width/2, orbitPosition.y - saw.height/2, (float)saw.width, (float)saw.height }, 
                (Vector2){ 0, 0 }, 
                rotationAngle, 
                Amarillo);

            static int frameCounter = 0;
            frameCounter++;
            if (frameCounter >= 10) {
                frameCounter = 0;
                enemyTrigger(enemies, orbitPosition);
            }
        }
    EndMode2D();
    //-----------------------------------------------------------------------------------
        // UI
    //-----------------------------------------------------------------------------------

    if(upgradeMenu) {
        enableUpgradeMenu();
    }else {
        disableUpgradeMenu();
    }
    DrawTexturePro(noiseTexture,
        (Rectangle){ 0, 0, (float)noiseTexture.width/2, (float)-noiseTexture.height/2 },
        (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() },
        (Vector2){ 0, 0 }, 0.0f,
        Fade(WHITE, 0.15f)
    );
    DrawText(TextFormat("Exp:%d ", player.experience), 10, 30, 20, Amarillo);
    DrawText(TextFormat("Level:%d ", player.level), 10, 60, 20, Amarillo);
    DrawText(TextFormat("Health:%d ", player.health), 10, 90, 20, Amarillo);
    DrawText(TextFormat("x:%.0f, y:%.0f ", player.position.x, player.position.y), 10, 120, 20, Amarillo);
    DrawText(TextFormat("Projectiles:%d ", projectilesCount), 10, 150, 20, Amarillo);
    DrawText(TextFormat("Enemies:%d ", enemiesCount), 10, 180, 20, Amarillo);
    DrawText(TextFormat("Orbs:%d ", orbsCount), 10, 210, 20, Amarillo);
    int seconds = (int)totalGameTime;
    if (seconds > 60) {
        seconds = seconds % 60;
    }
    DrawText(TextFormat("%2.0f:%2.0f", totalGameTime/60.0f, seconds), 10, 240, 20, Amarillo);
    
    DrawTexture(crosshair, mousePosition.x - crosshair.width/2, mousePosition.y - crosshair.height/2, RojoOscuro);

    // Death screen
    if(deathScreen) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.5f));
        DrawRectangleRounded((Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 250, 400, 500 }, 0.1f, 10, Amarillo);
        DrawText("YOU DIED", GetScreenWidth()/2 - MeasureText("YOU DIED", 20)/2, GetScreenHeight()/2 - 200 + 20, 20, AzulOscuro);
        DrawText("Press R to respawn", GetScreenWidth()/2 - MeasureText("Press R to respawn", 20)/2, GetScreenHeight()/2 - 200 + 60, 20, AzulOscuro);
        if (IsKeyPressed(KEY_R)) {
            deathScreen = false;
            player.position = (Vector2){ 0, 0 };
            player.health = 5;
            player.maxHealth = 5;
            player.level = 1;
            player.experience = 0;
            resurrect = false;
            resurrected = true;
            for(int i=0; i < MAX_ENEMIES; i++) {
                enemies[i].enabled = false;
                enemies[i].position = (Vector2){ -100000, -100000 };
            }
            for(int i=0; i < MAX_PROJECTILES; i++) {
                projectiles[i].enabled = false;
                projectiles[i].position = (Vector2){ -100000, -100000 };
            }
            for(int i=0; i < MAX_ORBS; i++) {
                orbs[i].enabled = false;
                orbs[i].position = (Vector2){ -100000, -100000 };
            }
            enemiesCount = 0;
            projectilesCount = 0;
            orbsCount = 0;
            upgradeMenu = false;
        }
    }

    DrawFPS(10, 10);
    EndDrawing();
    //----------------------------------------------------------------------------------
}

void GenOrbs(Vector2 position, int amount) {
    if(orbsCount >= MAX_ORBS) {
        orbsCount = 0;
    }
    for (int i = orbsCount; i < amount+orbsCount; i++) {
        float distance = GetRandomValue(0, 500) / 100.0f;
        orbs[i].position = (Vector2){
            position.x += distance,
            position.y += distance
        };
        orbs[i].radius = ORB_RADIUS * radiusMultiplier;
        orbs[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        orbs[i].enabled = true;
    }
}

void GenEnemies(Vector2 position, int amount) {
    for (int i = enemiesCount; i < amount+enemiesCount; i++) {
        float distance = GetRandomValue(0, 500) / 100.0f;
        enemies[i].position = (Vector2){
            position.x += distance,
            position.y += distance
        };
        enemies[i].radius = ENEMY_RADIUS;
        enemies[i].health = 50;
        enemies[i].maxHealth = 50;
        enemies[i].speed = 2.0f;
        enemies[i].enabled = true;
    }
}

void GenProjectiles(Vector2 position, Vector2 direction, int amount) {
    for (int i = projectilesCount; i < amount+projectilesCount; i++) {
        if (i < MAX_PROJECTILES) {
            projectiles[i].position = position;
            projectiles[i].radius = PROJECTILE_RADIUS;
            projectiles[i].speed = PROJECTILE_SPEED;
            projectiles[i].damage = player.damage;
            projectiles[i].enabled = true;
        }else{
            projectilesCount = 0;
        }
        

        //Calcular direccion
        projectiles[i].direction = Vector2Normalize(Vector2Subtract(direction, projectiles[i].position));
    }
}

void DrawOrbs(Orb *orbs, int amount) {
    for (int i = 0; i < amount; i++) {
        if(orbs[i].enabled){
            DrawCircle(orbs[i].position.x, orbs[i].position.y, 5, orbs[i].color);
            DrawRing((Vector2){ orbs[i].position.x, orbs[i].position.y }, orbs[i].radius-2, orbs[i].radius, 0, 360, 32, VerdeOscuro);
        }
    }
}
void DrawEnemies(Enemy *enemies, int amount) {
    for (int i = 0; i < amount; i++) {
        if(enemies[i].enabled){
            DrawRectangleRounded((Rectangle){ enemies[i].position.x-10.0f, enemies[i].position.y-12.5f, 20, 35 }, 1.0f, 10, RojoOscuro);
            DrawRing((Vector2){ enemies[i].position.x, enemies[i].position.y }, enemies[i].radius-2, enemies[i].radius, 0, 360, 32, VerdeOscuro);
            DrawRectangle(enemies[i].position.x - 10.0f, enemies[i].position.y - 20.0f, (enemies[i].health/enemies[i].maxHealth) * 20, 5, RojoOscuro);
        }
    }
}

void DrawProjectiles(Projectile *projectiles, int amount) {
    for (int i = 0; i < amount; i++) {
        if(projectiles[i].enabled){
            DrawTexture(bullet, projectiles[i].position.x - bullet.width/2, projectiles[i].position.y - bullet.height/2, Amarillo);
            }
    }
}

void OrbCollision(Orb *orbs) {
    for (int i = 0; i < MAX_ORBS; i++) {
        if(orbs[i].enabled){
            if(CheckCollisionCircles(player.position, player.radius, orbs[i].position, orbs[i].radius)){

                Vector2 direction = Vector2Subtract(player.position, orbs[i].position);
                float distance = Vector2Length(direction);

                if (distance > 0.0f) {
                    direction = Vector2Scale(Vector2Normalize(direction), 4.0f);
                    orbs[i].position = Vector2Add(orbs[i].position, direction);
                }

                // Orbe esta cerca del jugador
                if (distance <= 2.0f) {
                    orbs[i].position = (Vector2){ -100000, -100000 };
                    player.experience += 1;
                }
            }
        }
    }
}

void EnemyCollision(Enemy *enemies) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if(enemies[i].enabled){
            Vector2 direction = Vector2Subtract(player.position, enemies[i].position);
            float distance = Vector2Length(direction);

            if (distance > 0.0f) {
                direction = Vector2Scale(Vector2Normalize(direction), enemies[i].speed);
                enemies[i].position = Vector2Add(enemies[i].position, direction);
            }
            if(CheckCollisionCircles(player.position, player.radius, enemies[i].position, enemies[i].radius)){

                // Enemigo esta cerca del jugador
                if (distance <= enemies[i].radius + player.radius) {
                    enemies[i].position = (Vector2){ -100000, -100000 };
                    enemies[i].speed = 0.0f;
                    enemies[i].enabled = false;
                    PlayerTakeDamage(1);
                }
            }
        }
    }
}

void ProjectileCollision(Projectile *projectiles, Enemy *enemies) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if(projectiles[i].enabled){
            // Colision con enemigos
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if(enemies[j].enabled){
                    if(CheckCollisionCircles(projectiles[i].position, projectiles[i].radius, enemies[j].position, enemies[j].radius)){
                        enemies[j].health -= projectiles[i].damage;
                        projectiles[i].enabled = false;
                        projectiles[i].position = (Vector2){ -100000, -100000 };
                        if (enemies[j].health <= 0) {
                            enemies[j].enabled = false;
                            enemiesKilled++;
                            GenOrbs(enemies[j].position, 1);
                            orbsCount += 1;
                            orbsCollected++;
                            startEnemyAnimation(enemies[j].position, Amarillo);

                            GenProjectiles(enemies[j].position, Vector2Add(enemies[j].position, (Vector2){ cosf(30 * DEG2RAD), sinf(30 * DEG2RAD) }), 1);
                            projectilesCount++;
                            GenProjectiles(enemies[j].position, Vector2Add(enemies[j].position, (Vector2){ cosf(150 * DEG2RAD), sinf(150 * DEG2RAD) }), 1);
                            projectilesCount++;
                            GenProjectiles(enemies[j].position, Vector2Add(enemies[j].position, (Vector2){ cosf(270 * DEG2RAD), sinf(270 * DEG2RAD) }), 1);
                            projectilesCount++;
                            enemies[j].position = (Vector2){ -100000, -100000 };
                        }
                    }
                }
            }
        }
    }
}

void PlayerTakeDamage(int damage) {
    PushEnemiesAway(enemies);
    player.health -= damage;
    if (player.health <= 0) {
        player.health = 0;
        if(resurrect && !resurrected) {
            // Resucitar al jugador
            player.position = (Vector2){ 0, 0 };
            player.health = 5;
            player.maxHealth = 5;
            player.level = 1;
            player.experience = 0;
            resurrect = false;
            resurrected = true;
            for(int i=0; i < MAX_ENEMIES; i++) {
                enemies[i].enabled = false;
                enemies[i].position = (Vector2){ -100000, -100000 };
            }
            for(int i=0; i < MAX_PROJECTILES; i++) {
                projectiles[i].enabled = false;
                projectiles[i].position = (Vector2){ -100000, -100000 };
            }
            for(int i=0; i < MAX_ORBS; i++) {
                orbs[i].enabled = false;
                orbs[i].position = (Vector2){ -100000, -100000 };
            }
            enemiesCount = 0;
            projectilesCount = 0;
            orbsCount = 0;
            upgradeMenu = false;
        }
    }
}

void PushEnemiesAway(Enemy *enemies) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].enabled) {
            Vector2 direction = Vector2Subtract(enemies[i].position, player.position);
            float distance = Vector2Length(direction);

            if (distance > 0.0f && distance < 10.0f) {
                direction = Vector2Scale(Vector2Normalize(direction), 120.0f - distance);
                enemies[i].position = Vector2Add(enemies[i].position, direction);
            }
        }
    }
}

void enemiesSpawn(Enemy *enemies) {
    for (int i = enemiesCount; i < MAX_ENEMIES; i++) {
        float x, y;
        do {
            x = GetRandomValue(player.position.x - 2000, player.position.x + 2000);
        } while (x >= player.position.x-700 && x <= player.position.x+700);

        do {
            y = GetRandomValue(player.position.y - 2000, player.position.y + 2000);
        } while (y >= player.position.y-700 && y <= player.position.y+700);

        GenEnemies((Vector2){ x, y }, 1);
    }
    if(enemiesCount >= MAX_ENEMIES) {
        enemiesCount = 0;
    }
}

void UpdateProjectiles(Projectile *projectiles, int amount) {
    for (int i = 0; i < amount; i++) {
        if (projectiles[i].enabled) {
            // Actualizar la posición del proyectil
            projectiles[i].position = Vector2Add(projectiles[i].position, 
                Vector2Scale(projectiles[i].direction, projectiles[i].speed));
            
            // Deshabilitar proyectiles fuera de los límites
            if (projectiles[i].position.x < -5700 || projectiles[i].position.x > 5700 ||
                projectiles[i].position.y < -5700 || projectiles[i].position.y > 5700) {
                projectiles[i].enabled = false;
            }
        }
    }
}

void enemyTrigger(Enemy *enemies, Vector2 position) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if(enemies[i].enabled){
            if(CheckCollisionCircles(position, 10.0f, enemies[i].position, enemies[i].radius)){
                enemies[i].health -= abilityDamage*abilityMultiplier;
                if (enemies[i].health <= 0) {
                    enemies[i].enabled = false;
                    enemiesKilled++;
                    GenOrbs(enemies[i].position, 1);
                    orbsCount += 1;
                    orbsCollected++;

                    GenProjectiles(enemies[i].position, Vector2Add(enemies[i].position, (Vector2){ cosf(30 * DEG2RAD), sinf(30 * DEG2RAD) }), 1);
                    projectilesCount++;
                    GenProjectiles(enemies[i].position, Vector2Add(enemies[i].position, (Vector2){ cosf(150 * DEG2RAD), sinf(150 * DEG2RAD) }), 1);
                    projectilesCount++;
                    GenProjectiles(enemies[i].position, Vector2Add(enemies[i].position, (Vector2){ cosf(270 * DEG2RAD), sinf(270 * DEG2RAD) }), 1);
                    projectilesCount++;
                    enemies[i].position = (Vector2){ -100000, -100000 };
                }
            }
        }
    }
}

void ally(Enemy *enemies) {
    static float closestDistance = 1000.0f;
    static int closestEnemyIndex = -1;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].enabled) {
            Vector2 direction = Vector2Subtract(enemies[i].position, player.position);
            float distance = Vector2Length(direction);

            if (distance < closestDistance) {
                closestDistance = distance;
                closestEnemyIndex = i;
            }
        }
    }
    if (closestEnemyIndex != -1) {
        Vector2 closestEnemyPosition = enemies[closestEnemyIndex].position;
        GenProjectiles(Vector2Add(player.position, (Vector2){ 15, 5 }), closestEnemyPosition, 1);
        projectilesCount++;
        closestDistance = 1000.0f; // Reset
        closestEnemyIndex = -1;
    }
}

// Menu de mejoras 

void enableUpgradeMenu() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.5f));
    DrawRectangleRounded((Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 250, 400, 500 }, 0.02f, 10, Amarillo);
    DrawTexture(uiCorner, GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 250, Amarillo);
    DrawTexturePro(uiCorner, 
        (Rectangle){ 0, 0, (float)uiCorner.width, (float)uiCorner.height }, 
        (Rectangle){ GetScreenWidth()/2 + 200, GetScreenHeight()/2 - 250, (float)uiCorner.width, (float)uiCorner.height }, 
        (Vector2){ 0, 0 }, 
        90.0f, 
        Amarillo);
    DrawTexturePro(uiCorner, 
        (Rectangle){ 0, 0, (float)uiCorner.width, (float)uiCorner.height }, 
        (Rectangle){ GetScreenWidth()/2 + 200, GetScreenHeight()/2 + 250, (float)uiCorner.width, (float)uiCorner.height }, 
        (Vector2){ 0, 0 }, 
        180.0f, 
        Amarillo);
    DrawTexturePro(uiCorner, 
        (Rectangle){ 0, 0, (float)uiCorner.width, (float)uiCorner.height }, 
        (Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 + 250, (float)uiCorner.width, (float)uiCorner.height }, 
        (Vector2){ 0, 0 }, 
        270.0f, 
        Amarillo);
    DrawText("UPGRADE MENU", GetScreenWidth()/2 - MeasureText("UPGRADE MENU", 20)/2, GetScreenHeight()/2 - 200 + 20, 20, AzulOscuro);
    // Define los rextangulos
    Rectangle ability1 = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 - 150, 360, 80 };
    Rectangle ability2 = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 - 50, 360, 80 };
    Rectangle ability3 = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 + 50, 360, 80 };

    // Define los botones
    Rectangle cancelButton = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 + 160, 160, 40 };
    Rectangle acceptButton = { GetScreenWidth()/2 + 20, GetScreenHeight()/2 + 160, 160, 40 };

    // Elegir Habilidades
    if(!selectedIndex){
    do {
        index[0] = GetRandomValue(0, 17);
        index[1] = GetRandomValue(0, 17);
        index[2] = GetRandomValue(0, 17);
    }while(index[0] == index[1] || index[0] == index[2] || index[1] == index[2]);
    selectedIndex = true;
    }

    // Draw
    DrawRectangleRounded(ability1, 0.1f, 10, WHITE);
    DrawRectangleRounded(ability2, 0.1f, 10, WHITE);
    DrawRectangleRounded(ability3, 0.1f, 10, WHITE);

    switch (selectedAbility)
    {
    case 0:
        DrawRectangleLines(ability1.x, ability1.y, ability1.width, ability1.height, AzulOscuro);
        break;
    case 1:
        DrawRectangleLines(ability2.x, ability2.y, ability2.width, ability2.height, AzulOscuro);
        break;
    case 2:
        DrawRectangleLines(ability3.x, ability3.y, ability3.width, ability3.height, AzulOscuro);
        break;
    }

    DrawRectangle(ability1.x + 10, ability1.y + 10, 60, 60, RojoOscuro);
    DrawRectangle(ability2.x + 10, ability2.y + 10, 60, 60, VerdeOscuro);
    DrawRectangle(ability3.x + 10, ability3.y + 10, 60, 60, AzulOscuro);

    // Draw titles, subtitles
    DrawText(TextFormat("%s", abilities[index[0]].name), ability1.x + 80, ability1.y + 10, 20, AzulOscuro);
    DrawText(TextFormat("%s", abilities[index[0]].description), ability1.x + 80, ability1.y + 40, 16, AzulOscuro);

    DrawText(TextFormat("%s", abilities[index[1]].name), ability2.x + 80, ability2.y + 10, 20, AzulOscuro);
    DrawText(TextFormat("%s", abilities[index[1]].description), ability2.x + 80, ability2.y + 40, 16, AzulOscuro);

    DrawText(TextFormat("%s", abilities[index[2]].name), ability3.x + 80, ability3.y + 10, 20, AzulOscuro);
    DrawText(TextFormat("%s", abilities[index[2]].description), ability3.x + 80, ability3.y + 40, 16, AzulOscuro);

    // Draw buttons
    DrawRectangleRounded(cancelButton, 0.1f, 10, Naranja);
    DrawRectangleRounded(acceptButton, 0.1f, 10, VerdeOscuro);

    DrawText("Cancelar", cancelButton.x + 20, cancelButton.y + 10, 20, WHITE);
    DrawText("Aceptar", acceptButton.x + 20, acceptButton.y + 10, 20, WHITE);

    // Handle input
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, ability1)) {
            selectedAbility = 0;
        } else if (CheckCollisionPointRec(mouse, ability2)) {
            selectedAbility = 1;
        } else if (CheckCollisionPointRec(mouse, ability3)) {
            selectedAbility = 2;
        } else if (CheckCollisionPointRec(mouse, cancelButton)) {
            upgradeMenu = false; // Cancel
            selectedIndex = false;
            selectedAbility = 0;
        } else if (CheckCollisionPointRec(mouse, acceptButton)) {
            // Accept selected ability
            upgradeMenu = false;
            selectedIndex = false;
            selectedAbility = 0;
            acquiredAbilities[0] = abilities[selectedAbility];
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        upgradeMenu = false;
    }
}
void disableUpgradeMenu() {
    upgradeMenu = false;
}

void UpdateDrawAnimations(Animation *animations, int amount, Texture2D textures[]) {
    for (int i = 0; i < amount; i++) {
        if (animations[i].active) {
            animations[i].elapsedTime += GetFrameTime();
            
            if (animations[i].elapsedTime >= 0.03f) {
                animations[i].currentFrame++;
                animations[i].elapsedTime = 0.0f;
                
                if (animations[i].currentFrame >= animations[i].frames) {
                    animations[i].active = false;
                    animations[i].currentFrame = 0;
                }
            }
            
            if (animations[i].active) {
                DrawTexture(textures[animations[i].currentFrame], 
                           animations[i].position.x - textures[animations[i].currentFrame].width / 2, 
                           animations[i].position.y - textures[animations[i].currentFrame].height / 2, 
                           animations[i].tint);
            }
        }
    }
}

void startEnemyAnimation(Vector2 position, Color tint) {
    if(enemyAnimationsCount >= MAX_EN_ANI) {
        enemyAnimationsCount = 0;
    }
    for (int i = 0; i < MAX_EN_ANI; i++) {
        if (!enemyAnimations[i].active) {
            enemyAnimations[i].position = position;
            enemyAnimations[i].currentFrame = 0;
            enemyAnimations[i].elapsedTime = 0.0f;
            enemyAnimations[i].active = true;
            enemyAnimations[i].tint = tint;
        }
    }
}
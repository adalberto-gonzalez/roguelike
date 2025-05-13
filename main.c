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
#define MAX_PROJECTILES 32
#define PROJECTILE_RADIUS 5.0f
#define PROJECTILE_SPEED 4.0f
#define MAX_ANIMATIONS 32

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
    Texture2D textures[32];
    Vector2 position;
    int currentFrame;
    int frames;
    float elapsedTime;
    bool active;
    Color tint;
    float size;
} Animation;

//----------------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------------
Camera2D camera = { 0 };
Player player = { 0 };
Vector2 squarePosition = { 0 };
Vector2 mousePosition = (Vector2){ 0 };
Orb orbs[MAX_ORBS] = { 0 };
int orbsCount = 0;
Enemy enemies[MAX_ENEMIES] = { 0 };
int enemiesCount = 0;
Projectile projectiles[MAX_PROJECTILES] = { 0 };
int projectilesCount = 0;
float timer = 0.0f;
float totalGameTime = 0.0f;
float timeSinceLastClick = 0.0f;
float SpawnTimer = 0.0f;
float furiaTimer = 0.0f;
float regenTimer = 0.0f;
float stormTimer = 0.0f;
bool debug = false;

// Animaciones
Animation enemyAnimations[MAX_ANIMATIONS] = { 0 };
int enemyAnimationsCount = 0;
Animation explotionAnim[MAX_ANIMATIONS] = { 0 };
int explotionAnimCount = 0;
Animation lotusAnimation;
Animation demAnim[MAX_ENEMIES] = {0};
int demAnimCount = 0;

// Estadisticas
int enemiesKilled = 0;
int orbsCollected = 0;
int projectilesFired = 0;
int projectilesHit = 0;

// Habilidades
bool selectedIndex = false;
int abilityDamage = 20;
float abilityMultiplier = 1.0f;
float radiusMultiplier = 1.0f;
float currentSpeed = 1.0f;
int currentDamage = 10;
bool furiaActive = false;
int explotionDamage = 30;
float explotionRadius = 12.0f; //Multiplicado por player.radius
float corazonFracturadoMultiplier = 20.0f;
Ability abilities[18] = { 0 };
Ability acquiredAbilities[18] = { 0 };
bool resurrected = false;
float shootVelocity = 1.0f;
bool hasResurrect = false; // ✔️
bool hasDisparoMejorado = false;
bool hasMovimientoAgil = false;
bool hasRegeneracion = false; // ✔️
bool hasBifurcacion = false; // ✔️
bool hasAliado = false; // ✔️
bool hasTormentaDeBalas = false; // ✔️
bool hasFuria = false; // ✔️
bool hasExplosion = false; // ✔️
bool hasImanDeOrbes = false; // ✔️
bool hasDisparoRapido = false; // ✔️
bool hasAlmasErrantes = false; // ✔️
bool hasSierraGiratoria = false; // ✔️
bool hasCorazonFracturado = false; // ✔️

//UI
bool upgradeMenu = false;
bool deathScreen = false;
bool menuActive = false;
int selectedAbility = 0;
int index[3] = { 0 };

// Textures
Texture2D healthBar;
Texture2D noiseTexture;
Texture2D crosshair;
Texture2D uiCorner;
Texture2D bullet;
Texture2D saw;
Texture2D xpSection;
Texture2D xpBar;
Texture2D skullSmoke[6];
Texture2D explotion[6];
Texture2D lotus[12];
Texture2D dem[8];

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
void PlayerTakeDamage(int damage, Enemy *enemies);
void EnemyTakeDamage(Enemy *enemy, int damage);
void PushEnemiesAway(Enemy *enemies);
void enemiesSpawn(Enemy *enemies);
void UpdateProjectiles(Projectile *projectiles, int amount);
void enemyTrigger(Enemy *enemies, Vector2 position);
void ally(Enemy *enemies);
void enableUpgradeMenu();
void disableUpgradeMenu();
void UpdateDrawAnimations(Animation *animations, int amount, Texture2D textures[]);
void startEnemyAnimation(Vector2 position, Color tint);
void startAnimation(Animation *animations, Vector2 position, Color tint, int count, float size);
void UpdateAnimation(Animation *animation);
void singleAnimation(Animation *animation, Texture2D textures[], Vector2 position, Color tint, int count, float size);
void setAbilityStatus(int abilityIndex, bool status);
bool getAbilityStatus(int abilityIndex);
void DrawDebugInfo();

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
    currentSpeed = player.speed;
    currentDamage = player.damage;

    // Habilidades
    abilities[0].icon = LoadTexture("textures/ability1.png");
    abilities[0].name = "Eco de la Muerte";
    abilities[0].description = "Resucita al morir";

    abilities[1].icon = LoadTexture("textures/ability2.png");
    abilities[1].name = "Disparo Mejorado";
    abilities[1].description = "Incrementa el daño un 20%%";

    abilities[2].icon = LoadTexture("textures/ability3.png");
    abilities[2].name = "Movimiento Agil";
    abilities[2].description = "Incrementa la velocidad un 20%%";
    
    abilities[3].icon = LoadTexture("textures/ability4.png");
    abilities[3].name = "Regeneración";
    abilities[3].description = "Cada 60s emites un latido restaurador que cura 1 de vida.";

    abilities[4].icon = LoadTexture("textures/ability5.png");
    abilities[4].name = "Bifurcación Arcana";
    abilities[4].description = "Despliegas un disparo espejo.";

    abilities[5].icon = LoadTexture("textures/ability6.png");
    abilities[5].name = "Heraldos de Acero";
    abilities[5].description = "Convocas un aliado mecánico que abre fuego cada 0.5 s.";

    abilities[6].icon = LoadTexture("textures/ability7.png");
    abilities[6].name = "Tormenta de Balas";
    abilities[6].description = "Cada 3s desatas una ráfaga en seis direcciones.";

    abilities[7].icon = LoadTexture("textures/ability8.png");
    abilities[7].name = "Furia Incontenible";
    abilities[7].description = "Al recibir daño, te transformas: +50% daño y +25% velocidad por 15s.";

    abilities[8].icon = LoadTexture("textures/ability9.png");
    abilities[8].name = "Venganza Explosiva";
    abilities[8].description = "Cuando te hieren, desatas una explosión que inflige 30 de daño a los cercanos.";

    abilities[9].icon = LoadTexture("textures/ability10.png");
    abilities[9].name = "Iman de Orbes";
    abilities[9].description = "Tu campo de recolección de orbes se expande un 25%%";

    abilities[10].icon = LoadTexture("textures/ability11.png");
    abilities[10].name = "Disparo Rápido";
    abilities[10].description = "Incrementa la velocidad de disparo un 25%%";

    abilities[11].icon = LoadTexture("textures/ability12.png");
    abilities[11].name = "Almas Errantes";
    abilities[11].description = "Los enemigos muertos disparan 3 \nproyectiles al morir";
    
    abilities[12].icon = LoadTexture("textures/ability13.png");
    abilities[12].name = "Molinete de Hierro";
    abilities[12].description = "Una sierra giratoria rodea tu figura, destrozando a los enemigos cercanos.";

    abilities[13].icon = LoadTexture("textures/ability14.png");
    abilities[13].name = "Corazón Fracturado";
    abilities[13].description = "Cuando tu salud es baja, tus disparos explotan al impacto dañando todo a tu alrededor.";


    InitWindow(screenWidth, screenHeight, "Roguelike");
    //ToggleFullscreen();
    InitAudioDevice();

    // Carga de texturas
    noiseTexture = LoadTexture("textures/noise_overlay.png");
    crosshair = LoadTexture("textures/crosshair.png");
    uiCorner = LoadTexture("textures/ui_corner.png");
    bullet = LoadTexture("textures/bullet.png");
    saw = LoadTexture("textures/saw.png");
    healthBar = LoadTexture("textures/h_bar.png");
    xpSection = LoadTexture("textures/XpSection.png");
    xpBar = LoadTexture("textures/XpBar.png");

    for (int i=0; i <= 6; i++){
        skullSmoke[i] = LoadTexture(TextFormat("textures/Skull_Smoke/%d.png", i+1));
        explotion[i] = LoadTexture(TextFormat("textures/Explotion/%d.png", i+1));
    }
    for (int i=0; i <= 12; i++){
        lotus[i] = LoadTexture(TextFormat("textures/lotus/%d.png", i+1));
    }
    for (int i=0; i <= 8; i++){
        dem[i] = LoadTexture(TextFormat("textures/dem/%d.png", i+1));
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
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        enemyAnimations[i].position = (Vector2){ -100000, -100000 };
        enemyAnimations[i].currentFrame = 0;
        enemyAnimations[i].frames = 6;
        enemyAnimations[i].elapsedTime = 0.0f;
        enemyAnimations[i].active = false;
        enemyAnimations[i].tint = WHITE;
        enemyAnimations[i].size = 1.0f;
    }
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        explotionAnim[i].position = (Vector2){ -100000, -100000 };
        explotionAnim[i].currentFrame = 0;
        explotionAnim[i].frames = 6;
        explotionAnim[i].elapsedTime = 0.0f;
        explotionAnim[i].active = false;
        explotionAnim[i].tint = WHITE;
        explotionAnim[i].size = 1.0f;
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        demAnim[i].position = (Vector2){ -100000, -100000 };
        demAnim[i].currentFrame = 0;
        demAnim[i].frames = 6;
        demAnim[i].elapsedTime = 0.0f;
        demAnim[i].active = false;
        demAnim[i].tint = WHITE;
        demAnim[i].size = 1.0f;
    }
    lotusAnimation.position = (Vector2){ -100000, -100000 };
    lotusAnimation.currentFrame = 0;
    lotusAnimation.frames = 12;
    lotusAnimation.elapsedTime = 0.0f;
    lotusAnimation.active = false;
    lotusAnimation.tint = WHITE;
    lotusAnimation.size = 1.0f;

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
    UnloadTexture(crosshair);
    UnloadTexture(uiCorner);
    UnloadTexture(healthBar);
    UnloadTexture(saw);

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

    if(player.health <= 0 || (!hasResurrect && resurrected)) {
        deathScreen = true;
        menuActive = true;
    }
    if(player.experience >= player.level * 10) {
        player.level += 1;
        player.experience = 0;
        upgradeMenu = true;
        menuActive = true;
    }

    // Timers
    timeSinceLastClick += GetFrameTime();
    totalGameTime += GetFrameTime();
    timer += GetFrameTime();
    SpawnTimer += GetFrameTime() * (2.5f * log(0.8f * totalGameTime/60.0f + 1.0f) + 1.0f);
    
    if (timer >= 0.5f && hasAliado) {
        timer = 0.0f;
        ally(enemies);
    }

    // Spawner
    if (SpawnTimer >= 1.0f && !menuActive) {
        int enemies_to_spawn = (int)SpawnTimer;
        SpawnTimer -= enemies_to_spawn;
        
        for (int i = 0; i < enemies_to_spawn; ++i) {
            enemiesSpawn(enemies);
            enemiesCount += 1;
        }
    }

    // Furia Upgrade
    if(furiaActive && furiaTimer <= 15.0f) {
        player.speed = currentSpeed * 1.25f;
        player.damage = currentDamage * 2;
        furiaTimer += GetFrameTime();
    }else{
        furiaActive = false;
            player.speed = currentSpeed;
            player.damage = currentDamage;
            furiaTimer = 0.0f;
    }

    // Iman Upgrade
    if (hasImanDeOrbes) {
        radiusMultiplier += radiusMultiplier*0.25f;
        hasImanDeOrbes = false;
    }
    for (int i = 0; i < MAX_ORBS; i++) {
        orbs[i].radius = ORB_RADIUS * radiusMultiplier;
    }

    // Regen Upgrade
    if (hasRegeneracion){
        regenTimer += GetFrameTime();
        if(regenTimer >= 60.0f){
            regenTimer = 0;
            if (player.health < player.maxHealth){ 
                player.health++;
                singleAnimation(&lotusAnimation, lotus, player.position, GREEN, 1, 1.6f);
            }
        }
    }

    // Rafaga
    if (hasTormentaDeBalas){
        stormTimer += GetFrameTime();
        if(stormTimer >= 3.0f){
            stormTimer = 0;
            GenProjectiles(player.position, Vector2Add(player.position, (Vector2){ cosf(360 * DEG2RAD), sinf(0 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(player.position, Vector2Add(player.position, (Vector2){ cosf(60 * DEG2RAD), sinf(60 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(player.position, Vector2Add(player.position, (Vector2){ cosf(120 * DEG2RAD), sinf(120 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(player.position, Vector2Add(player.position, (Vector2){ cosf(180 * DEG2RAD), sinf(180 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(player.position, Vector2Add(player.position, (Vector2){ cosf(240 * DEG2RAD), sinf(240 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(player.position, Vector2Add(player.position, (Vector2){ cosf(300 * DEG2RAD), sinf(300 * DEG2RAD) }), 1);
            projectilesCount++;
        }
    }

    if(hasDisparoRapido){
        shootVelocity += shootVelocity*0.25f;
        hasDisparoRapido = false;
    }

    if(IsKeyPressed(KEY_GRAVE)){
        debug = !debug;
    }

    if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !menuActive && debug) {
        if (enemiesCount < MAX_ENEMIES) {
            GenOrbs(GetScreenToWorld2D(mousePosition, camera), 1);
            orbsCount++;
        }
    }
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && timeSinceLastClick >= 0.3f/shootVelocity && !menuActive) {
        timeSinceLastClick = 0.0f;
        if (projectilesCount < MAX_PROJECTILES) {
            GenProjectiles(player.position, GetScreenToWorld2D(mousePosition, camera), 1);
            projectilesCount++;
            if(hasBifurcacion){
                float angle = atan2f(
                    GetScreenToWorld2D(mousePosition, camera).y - player.position.y,
                    GetScreenToWorld2D(mousePosition, camera).x - player.position.x
                );
                float angle_offset = angle + 5.0f * DEG2RAD;
                Vector2 bifurcatedTarget = {
                    player.position.x + cosf(angle_offset) * 100.0f,
                    player.position.y + sinf(angle_offset) * 100.0f
                };
                GenProjectiles(player.position, bifurcatedTarget, 1);
                projectilesCount++;
            }
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
        if(debug) DrawRing((Vector2){ player.position.x, player.position.y }, player.radius - 2, player.radius, 0, 360, 32, VerdeOscuro);
        DrawOrbs(orbs, MAX_ORBS);
        DrawEnemies(enemies, MAX_ENEMIES);
        DrawProjectiles(projectiles, MAX_PROJECTILES);

        if(!menuActive) {
            UpdateProjectiles(projectiles, MAX_PROJECTILES);
            UpdateDrawAnimations(enemyAnimations, MAX_ANIMATIONS, skullSmoke);
            UpdateDrawAnimations(explotionAnim, MAX_ANIMATIONS, explotion);
            UpdateDrawAnimations(demAnim, MAX_ENEMIES, dem);
            UpdateAnimation(&lotusAnimation);
            // Collision logic
            OrbCollision(orbs);
            EnemyCollision(enemies);
            ProjectileCollision(projectiles, enemies);
            if(IsKeyDown(KEY_W)) direction.y -= player.speed * player.acceleration;
            if(IsKeyDown(KEY_S)) direction.y += player.speed * player.acceleration;
            if(IsKeyDown(KEY_A)) direction.x -= player.speed * player.acceleration;
            if(IsKeyDown(KEY_D)) direction.x += player.speed * player.acceleration;
            Vector2Normalize(direction);
            player.position.x = Clamp(player.position.x, -2500.0f, 2500.0f);
            player.position.y = Clamp(player.position.y, -2500.0f, 2500.0f);
            player.position = Vector2Add(player.position, direction);
        }

        // limit
        DrawRectangleLines(-2500, -2500, 5000, 5000, RojoOscuro);
        
        if(!menuActive && hasSierraGiratoria) {
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

    for(int i=0; i<player.maxHealth; i++){
        if(player.health > i){
            int r = 255 - (i * 25);
            if (r < 0) r = 0;
            DrawTextureEx(healthBar, (Vector2){17 + i*30, 10}, 0.0f, 3.0f, (Color){ r, 0, 0, 255 });
        }else{
            int r = 110 - (i * 10);
            int g = 120 - (i * 10);
            int b = 120 - (i * 10);
            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            DrawTextureEx(healthBar, (Vector2){17 + i*30, 10}, 0.0f, 3.0f, (Color){ r, g, b, 255 });
        }
    }

    DrawTextureEx(xpBar, (Vector2){10, 40}, 0.0f, 3.0f, WHITE);
    for(int i=0; i<10; i++){
        if(player.experience/player.level > i+1){
            DrawTextureEx(xpSection, (Vector2){13 + i*12, 43}, 0.0f, 3.0f, WHITE);
        }
    }
    
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
    
    DrawTexture(crosshair, mousePosition.x - crosshair.width/2, mousePosition.y - crosshair.height/2, RojoOscuro);
    if(debug) DrawDebugInfo();

    // Death screen
    if(deathScreen) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.5f));
        DrawRectangleRounded((Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 250, 400, 500 }, 0.1f, 10, Amarillo);
        DrawText("YOU DIED", GetScreenWidth()/2 - MeasureText("YOU DIED", 20)/2, GetScreenHeight()/2 - 200 + 20, 20, AzulOscuro);
        DrawText("Press R to respawn", GetScreenWidth()/2 - MeasureText("Press R to respawn", 20)/2, GetScreenHeight()/2 - 200 + 60, 20, AzulOscuro);
        DrawText(TextFormat("Enemigos eliminados: %d", enemiesKilled), GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 200 + 120, 20, AzulOscuro);
        DrawText(TextFormat("Orbes recogidos: %d", orbsCollected), GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 200 + 150, 20, AzulOscuro);
        DrawText(TextFormat("Proyectiles disparados: %d", projectilesFired), GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 200 + 180, 20, AzulOscuro);
        DrawText(TextFormat("Proyectiles acertados: %d", projectilesHit), GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 200 + 210, 20, AzulOscuro);
        if (IsKeyPressed(KEY_R)) {
            deathScreen = false;
            player.position = (Vector2){ 0, 0 };
            player.health = 5;
            player.maxHealth = 5;
            player.level = 1;
            player.experience = 0;
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
            menuActive = false;
        }
    }

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
        enemies[i].health = 30;
        enemies[i].maxHealth = 30;
        enemies[i].speed = 1.35f;
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
        projectiles[i].direction = Vector2Normalize(Vector2Subtract(direction, projectiles[i].position));
    }
}

void DrawOrbs(Orb *orbs, int amount) {
    for (int i = 0; i < amount; i++) {
        if(orbs[i].enabled){
            DrawCircle(orbs[i].position.x, orbs[i].position.y, 5, orbs[i].color);
            if(debug) DrawRing((Vector2){ orbs[i].position.x, orbs[i].position.y }, (orbs[i].radius*radiusMultiplier)-2, (orbs[i].radius*radiusMultiplier), 0, 360, 32, VerdeOscuro);
        }
    }
}
void DrawEnemies(Enemy *enemies, int amount) {
    for (int i = 0; i < amount; i++) {
        if(enemies[i].enabled){
            startAnimation(demAnim, enemies[i].position, WHITE, demAnimCount, 2.5f);
            demAnimCount++;
            if(debug) DrawRing((Vector2){ enemies[i].position.x, enemies[i].position.y }, enemies[i].radius-2, enemies[i].radius, 0, 360, 32, VerdeOscuro);
            if(debug) DrawRectangle(enemies[i].position.x - 10.0f, enemies[i].position.y - 20.0f, (enemies[i].health/enemies[i].maxHealth) * 20, 5, RojoOscuro);
        }
    }
}

void DrawProjectiles(Projectile *projectiles, int amount) {
    for (int i = 0; i < amount; i++) {
        if(projectiles[i].enabled){
            DrawTexture(bullet, projectiles[i].position.x - bullet.width/2, projectiles[i].position.y - bullet.height/2, Amarillo);
            }
            if(debug) DrawRing((Vector2){ projectiles[i].position.x, projectiles[i].position.y }, (projectiles[i].radius*corazonFracturadoMultiplier)-2, projectiles[i].radius*corazonFracturadoMultiplier, 0, 360, 32, VerdeOscuro);
    }
}

void OrbCollision(Orb *orbs) {
    for (int i = 0; i < MAX_ORBS; i++) {
        if(orbs[i].enabled){
            if(CheckCollisionCircles(player.position, player.radius, orbs[i].position, orbs[i].radius*radiusMultiplier)){

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
                    PlayerTakeDamage(1, enemies);
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
                        // Corazon fracturado
                        if(hasCorazonFracturado && player.health <= 1) {
                            startAnimation(explotionAnim, projectiles[i].position, BLUE, explotionAnimCount, 2.0f);
                            explotionAnimCount++;
                            for (int k = 0; k < MAX_ENEMIES; k++) {
                                if(enemies[k].enabled){
                                    if(CheckCollisionCircles(projectiles[i].position, projectiles[i].radius*corazonFracturadoMultiplier, enemies[k].position, enemies[k].radius)){
                                        EnemyTakeDamage(&enemies[k], projectiles[i].damage);
                                    }
                                }
                            }
                        }
                        projectiles[i].enabled = false;
                        projectiles[i].position = (Vector2){ -100000, -100000 };
                        EnemyTakeDamage(&enemies[j], projectiles[i].damage);
                    }
                }
            }
        }
    }
}

void EnemyTakeDamage(Enemy *enemy, int damage){
    enemy->health -= damage;
    if (enemy->health <= 0) {
        enemy->enabled = false;
        enemiesKilled++;
        GenOrbs(enemy->position, 1);
        orbsCount += 1;
        orbsCollected++;
        startEnemyAnimation(enemy->position, Amarillo);
        enemyAnimationsCount++;
        if(hasAlmasErrantes) {
            GenProjectiles(enemy->position, Vector2Add(enemy->position, (Vector2){ cosf(30 * DEG2RAD), sinf(30 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(enemy->position, Vector2Add(enemy->position, (Vector2){ cosf(150 * DEG2RAD), sinf(150 * DEG2RAD) }), 1);
            projectilesCount++;
            GenProjectiles(enemy->position, Vector2Add(enemy->position, (Vector2){ cosf(270 * DEG2RAD), sinf(270 * DEG2RAD) }), 1);
            projectilesCount++;
        }
        enemy->position = (Vector2){ -100000, -100000 };
    }
}

void PlayerTakeDamage(int damage, Enemy *enemies) {
    PushEnemiesAway(enemies);
    player.health -= damage;
    for(int i=0; i<=MAX_ENEMIES; i++){
        if(enemies[i].enabled && hasExplosion && CheckCollisionCircles(player.position, player.radius*explotionRadius, enemies[i].position, enemies[i].radius)){
            startAnimation(explotionAnim, player.position, BLUE, explotionAnimCount, 2.5f);
            EnemyTakeDamage(&enemies[i], explotionDamage);
        }
    }
    if (player.health <= 0) {
        player.health = 0;
        // Resucitar al jugador
        if(hasResurrect && !resurrected) {
            player.health++;
            singleAnimation(&lotusAnimation, lotus, player.position, GREEN, 1, 1.6f);
            resurrected = true;
            for(int i=0; i < MAX_ENEMIES; i++) {
                enemies[i].enabled = false;
                enemies[i].position = (Vector2){ -100000, -100000 };
            }
            for(int i=0; i < MAX_PROJECTILES; i++) {
                projectiles[i].enabled = false;
                projectiles[i].position = (Vector2){ -100000, -100000 };
            }
            enemiesCount = 0;
            projectilesCount = 0;
            upgradeMenu = false;
            menuActive = false;
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
                EnemyTakeDamage(&enemies[i], abilityDamage*abilityMultiplier);
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

void setAbilityStatus(int abilityIndex, bool status) {
    switch (abilityIndex) {
        case 0: hasResurrect = status; break;
        case 1: hasDisparoMejorado = status; break;
        case 2: hasMovimientoAgil = status; break;
        case 3: hasRegeneracion = status; break;
        case 4: hasBifurcacion = status; break;
        case 5: hasAliado = status; break;
        case 6: hasTormentaDeBalas = status; break;
        case 7: hasFuria = status; break;
        case 8: hasExplosion = status; break;
        case 9: hasImanDeOrbes = status; break;
        case 10: hasDisparoRapido = status; break;
        case 11: hasAlmasErrantes = status; break;
        case 12: hasSierraGiratoria = status; break;
        case 13: hasCorazonFracturado = status; break;
        default: break;  
    }
}

bool getAbilityStatus(int abilityIndex) {
    switch (abilityIndex) {
        case 0: return hasResurrect;
        case 1: return hasDisparoMejorado;
        case 2: return hasMovimientoAgil;
        case 3: return hasRegeneracion;
        case 4: return hasBifurcacion;
        case 5: return hasAliado;
        case 6: return hasTormentaDeBalas;
        case 7: return hasFuria;
        case 8: return hasExplosion;
        case 9: return hasImanDeOrbes;
        case 10: return hasDisparoRapido;
        case 11: return hasAlmasErrantes;
        case 12: return hasSierraGiratoria;
        case 13: return hasCorazonFracturado;
        default: return false;  
    }
}

void disableUpgradeMenu() {
    upgradeMenu = false;
    menuActive = false;
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
                DrawTextureEx(textures[animations[i].currentFrame],
                    (Vector2){animations[i].position.x - (textures[animations[i].currentFrame].width*animations[i].size) / 2, 
                    animations[i].position.y - (textures[animations[i].currentFrame].height*animations[i].size) / 2}, 
                    0.0f,
                    animations[i].size,
                    animations[i].tint);
            }
        }
    }
}

void startEnemyAnimation(Vector2 position, Color tint) {
    if(enemyAnimationsCount >= MAX_ANIMATIONS) {
        enemyAnimationsCount = 0;
    }
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!enemyAnimations[i].active) {
            enemyAnimations[i].position = position;
            enemyAnimations[i].currentFrame = 0;
            enemyAnimations[i].elapsedTime = 0.0f;
            enemyAnimations[i].active = true;
            enemyAnimations[i].tint = tint;
            enemyAnimations[i].size = 1.0f;
        }
    }
}

void startAnimation(Animation *animations, Vector2 position, Color tint, int count, float size) {
    if(count >= MAX_ANIMATIONS) {
        count = 0;
    }
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!animations[i].active) {
            animations[i].position = position;
            animations[i].currentFrame = 0;
            animations[i].elapsedTime = 0.0f;
            animations[i].active = true;
            animations[i].tint = tint;
            animations[i].size = size;
        }
    }
}

void UpdateAnimation(Animation *animation){
    if (animation->active) {
        animation->elapsedTime += GetFrameTime();
        
        if (animation->elapsedTime >= 0.03f) {
            animation->currentFrame++;
            animation->elapsedTime = 0.0f;
            
            if (animation->currentFrame >= animation->frames) {
                animation->active = false;
                animation->currentFrame = 0;
            }
        }

        DrawTextureEx(animation->textures[animation->currentFrame],
            (Vector2){animation->position.x - (animation->textures[animation->currentFrame].width*animation->size) / 2, 
            animation->position.y - (animation->textures[animation->currentFrame].height*animation->size) / 2}, 
            0.0f,
            animation->size,
            animation->tint);
    }
}

void singleAnimation(Animation *animation, Texture2D textures[], Vector2 position, Color tint, int count, float size) {
    if(!animation->active){
        for(int i=0; i<32; i++){
            animation->textures[i] = textures[i];
        }
        animation->position = position;
        animation->currentFrame = 0;
        animation->elapsedTime = 0.0f;
        animation->active = true;
        animation->tint = tint;
        animation->size = size;
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

    Rectangle ability1 = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 - 150, 360, 80 };
    Rectangle ability2 = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 - 50, 360, 80 };
    Rectangle ability3 = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 + 50, 360, 80 };

    Rectangle cancelButton = { GetScreenWidth()/2 - 180, GetScreenHeight()/2 + 160, 160, 40 };
    Rectangle acceptButton = { GetScreenWidth()/2 + 20, GetScreenHeight()/2 + 160, 160, 40 };

    if(!selectedIndex){
        int availableAbilities[18];
        int availableCount = 0;

        for (int i = 0; i < 18; i++) {
            if (!getAbilityStatus(i)) {
                availableAbilities[availableCount++] = i;
            }
        }

        if (availableCount < 3) {
            selectedIndex = false; 
            return;
        }

        do {
            index[0] = availableAbilities[GetRandomValue(0, availableCount - 1)];
            index[1] = availableAbilities[GetRandomValue(0, availableCount - 1)];
            index[2] = availableAbilities[GetRandomValue(0, availableCount - 1)];
        } while (index[0] == index[1] || index[0] == index[2] || index[1] == index[2]);

        selectedIndex = true;
    }

    DrawRectangleRounded(ability1, 0.1f, 10, WHITE);
    DrawRectangleRounded(ability2, 0.1f, 10, WHITE);
    DrawRectangleRounded(ability3, 0.1f, 10, WHITE);

    switch (selectedAbility) {
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

    DrawText(TextFormat("%s", abilities[index[0]].name), ability1.x + 80, ability1.y + 10, 20, AzulOscuro);
    DrawText(TextFormat("%s", abilities[index[0]].description), ability1.x + 80, ability1.y + 40, 16, AzulOscuro);

    DrawText(TextFormat("%s", abilities[index[1]].name), ability2.x + 80, ability2.y + 10, 20, AzulOscuro);
    DrawText(TextFormat("%s", abilities[index[1]].description), ability2.x + 80, ability2.y + 40, 16, AzulOscuro);

    DrawText(TextFormat("%s", abilities[index[2]].name), ability3.x + 80, ability3.y + 10, 20, AzulOscuro);
    DrawText(TextFormat("%s", abilities[index[2]].description), ability3.x + 80, ability3.y + 40, 16, AzulOscuro);

    DrawRectangleRounded(cancelButton, 0.1f, 10, Naranja);
    DrawRectangleRounded(acceptButton, 0.1f, 10, VerdeOscuro);

    DrawText("Cancelar", cancelButton.x + 20, cancelButton.y + 10, 20, WHITE);
    DrawText("Aceptar", acceptButton.x + 20, acceptButton.y + 10, 20, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, ability1)) {
            selectedAbility = 0;
        } else if (CheckCollisionPointRec(mouse, ability2)) {
            selectedAbility = 1;
        } else if (CheckCollisionPointRec(mouse, ability3)) {
            selectedAbility = 2;
        } else if (CheckCollisionPointRec(mouse, cancelButton)) {
            upgradeMenu = false; // Cancelar
            menuActive = true;
            selectedIndex = false;
            selectedAbility = 0;
        } else if (CheckCollisionPointRec(mouse, acceptButton)) {
            upgradeMenu = false;
            menuActive = true;
            acquiredAbilities[selectedAbility] = abilities[index[selectedAbility]];
            setAbilityStatus(index[selectedAbility], true); 
            selectedIndex = false;
            selectedAbility = 0;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        upgradeMenu = false;
        menuActive = false;
    }
}

void DrawDebugInfo(){
    DrawFPS(10, 10);
    DrawText(TextFormat("Exp:%d ", player.experience), 10, 30, 20, Amarillo);
    DrawText(TextFormat("Level:%d ", player.level), 10, 60, 20, Amarillo);
    DrawText(TextFormat("Health:%d ", player.health), 10, 90, 20, Amarillo);
    DrawText(TextFormat("x:%.0f, y:%.0f ", player.position.x, player.position.y), 10, 120, 20, Amarillo);
    DrawText(TextFormat("Projectiles:%d ", projectilesCount), 10, 150, 20, Amarillo);
    DrawText(TextFormat("Enemies:%d ", enemiesCount), 10, 180, 20, Amarillo);
    DrawText(TextFormat("Orbs:%d ", orbsCount), 10, 210, 20, Amarillo);
    DrawText(TextFormat("%2.0f", totalGameTime), 10, 240, 20, Amarillo);
    int yOffset = 280;
    DrawText("HABILIDADES:", 10, yOffset, 20, Amarillo);
    yOffset += 30;
    for (int i = 0; i < 14; i++) {
        Color abilityColor = getAbilityStatus(i) ? VerdeOscuro : RojoOscuro;
        DrawRectangle(10, yOffset + i * 28, 18, 18, abilityColor);
        DrawText(abilities[i].name, 35, yOffset + i * 28, 18, abilityColor);
    }
}
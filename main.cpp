#include "include/raylib.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

using std::vector;

const int FPS = 360;
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;
const Color BACKGROUND_COLOR = (Color){30, 30, 30, 255};
const float PLAYER_RADIUS = 30.0;
const float PLAYER_VELOCITY = 200.0;
const int PLAYER_HEALTH = 5;
const Color PLAYER_COLOR = (Color){255, 0, 0, 255};
const float ENEMY_VELOCITY = 20.0;
const Color ENEMY_COLOR = (Color){255, 255, 255, 255};
const float ENEMY_RADIUS = 20.0;
const int ENEMY_SPAWN_OFFSET = 100;
const int ENEMY_SPAWN_RATE = 5;
const float BULLET_RADIUS = 10.0;
const float BULLET_VELOCITY = 200.0;
const Color BULLET_COLOR = (Color) {255, 0, 0, 255};

typedef struct Circle {
    Vector2 origin;
    Vector2 direction;
    float radius;
    Rectangle tail;
    float velocity;
    int health;
    Color color;
} Circle;

Vector2 substract(Vector2 a, Vector2 b){
    return (Vector2){
	.x = b.x - a.x,
	.y = b.y - a.y
    };
}

Vector2 normalize(Vector2 vector) {
    float length = sqrtf(vector.x * vector.x + vector.y * vector.y);

    if (length != 0.0f) {
	vector.x /= length;
	vector.y /= length;
    } else {
	vector.x = 0.0f;
	vector.y = 0.0f;
    }

    return vector;
}

Vector2 inverse(Vector2 vector) {
    return (Vector2) {
	.x = -vector.x,
	.y = -vector.y
    };
}

float getMagnitude(Vector2 a, Vector2 b) {
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


float getAngle(Vector2 a, Vector2 b) {
    float dotProduct = a.x * b.x + a.y * b.y;
    float crossProduct = a.x * b.y - a.y * b.x;

    float angle = atan2f(crossProduct, dotProduct);
    angle = -angle;

    if (angle < 0)
	angle += 2 * PI;

    return angle * 180 / PI;
}

Circle createPlayer(float x, float y) {
    return (Circle){.origin = (Vector2){.x = x, .y = y},
	.radius = PLAYER_RADIUS,
	.velocity = PLAYER_VELOCITY,
	.health = PLAYER_HEALTH,
	.color = PLAYER_COLOR
    };
}

Circle createEnemy(float x, float y) {
    return (Circle){.origin = (Vector2){.x = x, .y = y},
	.radius = ENEMY_RADIUS,
	.velocity = ENEMY_VELOCITY,
	.color = ENEMY_COLOR
    };
}


Circle createBullet(Circle player, Vector2 direction) {
    return (Circle){.origin = {player.origin.x + direction.x * player.radius, player.origin.y + direction.y * player.radius },
	.direction = direction,
	.radius = BULLET_RADIUS,
	.velocity = BULLET_VELOCITY,
	.color = BULLET_COLOR
    };
}

void movePlayer(Circle *player, float dt) {
    player->direction = {0,0};

    if (IsKeyDown(KEY_W)) {
	player->direction.y = -1;
	player->origin.y -= player->velocity * dt;

	if (player->origin.y - player->radius <= 0) {
	    player->origin.y = player->radius;
	}
    }

    if (IsKeyDown(KEY_A)) {
	player->direction.x = -1;
	player->origin.x -= player->velocity * dt;

	if (player->origin.x - player->radius <= 0) {
	    player->origin.x = player->radius;
	}
    }

    if (IsKeyDown(KEY_S)) {
	player->direction.y = 1;
	player->origin.y += player->velocity * dt;

	if (player->origin.y + player->radius >= GetScreenHeight()) {
	    player->origin.y = (float)GetScreenHeight() - player->radius;
	}
    }

    if (IsKeyDown(KEY_D)) {
	player->direction.x = 1;
	player->origin.x += player->velocity * dt;

	if (player->origin.x + player->radius >= GetScreenWidth()) {
	    player->origin.x = (float)GetScreenWidth() - player->radius;
	}
    }
}

void updateEnemyPosition(Circle *enemy, Circle *player, float dt) {
    enemy->direction = normalize(substract(player->origin, enemy->origin));
    enemy->origin.x -= enemy->direction.x * enemy->velocity * dt;
    enemy->origin.y -= enemy->direction.y * enemy->velocity * dt;
}

void addTail(Circle *circle) {
    if (circle->direction.x == 0 && circle->direction.y == 0) return;
    circle->tail = (Rectangle){
	.x = circle->origin.x,
	.y = circle->origin.y,
	.width = circle->radius,
	.height = circle->radius
    };
    DrawRectanglePro(circle->tail, {0, 0}, getAngle(circle->direction, {circle->tail.x + circle->tail.width, circle->tail.y + circle->tail.height}), circle->color);
}

Vector2 generateRandomCoord(Circle player) {
    int randomX = ENEMY_SPAWN_OFFSET + rand() % ENEMY_SPAWN_OFFSET;
    int randomY = ENEMY_SPAWN_OFFSET + rand() % ENEMY_SPAWN_OFFSET;
    int randomCoord = rand() % 4;

    if (randomCoord == 0) {
	randomX = player.origin.x - randomX;
	randomY = player.origin.y - randomY;
    } else if (randomCoord == 1) {
	randomX = player.origin.x - randomX;
	randomY = player.origin.y + randomY;
    } else if (randomCoord == 2) {
	randomX = player.origin.x + randomX;
	randomY = player.origin.y - randomY;
    } else {
	randomX = player.origin.x + randomX;
	randomY = player.origin.y + randomY;
    }

    return {(float)randomX, (float)randomY};
}

void updatePlayer(Circle *player) {
    player->direction = inverse(player->direction);
    addTail(player);
    DrawRectangleLines(player->origin.x, player->origin.y, player->direction.x, player->direction.y, player->color);
    DrawCircleV(player->origin, player->radius, player->color);
    DrawText(std::to_string(player->health).c_str(), player->origin.x - 5, player->origin.y - 5, 20, WHITE);
}

void updateBullets(vector<Circle> &bulletList, float dt) {
    for (int i = 0; i < bulletList.size(); i++) {
	bulletList[i].origin.x += bulletList[i].direction.x * bulletList[i].velocity * dt;
	bulletList[i].origin.y += bulletList[i].direction.y * bulletList[i].velocity * dt;

	if (
	    bulletList[i].origin.x > GetScreenWidth() ||
	    bulletList[i].origin.x <= 0 ||
	    bulletList[i].origin.y > GetScreenHeight() ||
	    bulletList[i].origin.y <= 0 ){
	    bulletList.erase(bulletList.begin() + i);
	    continue;
	}
	DrawCircleV(bulletList[i].origin, bulletList[i].radius, bulletList[i].color);
    }
}

void spawnBullets(vector<Circle> &bulletList, Circle player) {
	if (IsMouseButtonPressed(0)){
	    Vector2 direction = normalize(substract(player.origin, GetMousePosition()));
	    bulletList.push_back(createBullet(player, direction));
	}
}

void resetGame(Circle *player, vector<Circle> *enemyList) {
    player->health = PLAYER_HEALTH;
    player->origin = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    player->color = PLAYER_COLOR;
    enemyList->clear();
}


int main() {
    srand(time(0));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Shboot");

    float dt;
    int frame = 0;
    int score = 0;
    int highScore = 0;
    float enemySpawnRate = ENEMY_SPAWN_RATE;
    float enemySpawnCooldown = enemySpawnRate;
    vector<Circle> enemyList;
    vector<Circle> bulletList;
    Circle player = createPlayer(400.0, 300.0);

    // NOTE: game loop
    SetTargetFPS(FPS);
    while (!WindowShouldClose()) {

	dt = GetFrameTime();
	frame++;
	enemySpawnCooldown -= dt;

	//NOTE: Random Enemy Generation and Spawn with a Rate
	if (enemySpawnCooldown <= 0){
	    Vector2 coord = generateRandomCoord(player);
	    enemyList.push_back(createEnemy(coord.x, coord.y));
	    if (enemySpawnRate > 0.5) enemySpawnRate -= 0.5;
	    enemySpawnCooldown = enemySpawnRate;
	}

	BeginDrawing();
	DrawFPS(10, 10);
	ClearBackground(BACKGROUND_COLOR);

	//NOTE: Player Logic
	movePlayer(&player, dt);
	updatePlayer(&player);


	//NOTE: Bullet Logic
	spawnBullets(bulletList, player);
	updateBullets(bulletList, dt);


	//NOTE: Enemy Logic
	for (int i = 0; i < enemyList.size(); i++) {
	    updateEnemyPosition(&enemyList[i], &player, dt);
	    addTail(&enemyList[i]);
	    DrawCircleV(enemyList[i].origin, enemyList[i].radius, enemyList[i].color);

	    //NOTE: Collision with Player Logic
	    if (CheckCollisionCircles(player.origin, player.radius, enemyList[i].origin, enemyList[i].radius)) {
		enemyList.erase(enemyList.begin() + i);
		player.health--;
		if (player.health <= 0) {
		    resetGame(&player, &enemyList);
		    score = 0;
		    enemySpawnRate = 5;
		    break;
		}
	    }

	    //NOTE: Collision with Bullet Logic
	    for (int bi = 0; bi < bulletList.size(); bi++) {
		if (CheckCollisionCircles(enemyList[i].origin, enemyList[i].radius, bulletList[bi].origin, bulletList[bi].radius)) {
		    bulletList.erase(bulletList.begin() + bi);
		    enemyList.erase(enemyList.begin() + i);
		    score++;
		    if (score % 10 == 0) player.health++;
		    if (score > highScore) highScore = score;
		}
	    }

	}
	DrawText(("SCORE: " + std::to_string(score)).c_str(), GetScreenWidth() / 2 - 15, 20.0, 20, WHITE);
	DrawText(("HIGH SCORE: " + std::to_string(highScore)).c_str(), GetScreenWidth() - 200, 20.0, 20, WHITE);
	EndDrawing();
    }

    CloseWindow();
    return 0;
}

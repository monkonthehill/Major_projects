#include "raylib.h"
#include <stdbool.h>

#define WIDTH 1000
#define HEIGHT 1000 
#define MAX_WAVES 20

typedef struct {
    float x;
    float y;
    float radius;
    bool active;
} Ripple;

float Radius = 30.0f;
float circleX = 500.0f;
float circleY = 500.0f;
float speed = 60.0f;
float growth_rate = 3.0f; 

Ripple waves[MAX_WAVES];
int spawn_timer = 0;
int spawn_delay = 20;

void CreateCircle() { 
    DrawCircle((int)circleX, (int)circleY, Radius, WHITE); 
}

void CreateWaves() {
    spawn_timer++;
    if (spawn_timer >= spawn_delay) {
        for (int i = 0; i < MAX_WAVES; i++) {
            if (!waves[i].active) {
                waves[i].x = circleX;
                waves[i].y = circleY;
                waves[i].radius = Radius;
                waves[i].active = true;
                spawn_timer = 0;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_WAVES; i++) {
        if (waves[i].active) {
            waves[i].radius += growth_rate;
            DrawCircleLines((int)waves[i].x, (int)waves[i].y, waves[i].radius, WHITE);
            if (waves[i].radius > 600.0f) {
                waves[i].active = false;
            }
        }
    }
}

int main(void) {
    for (int i = 0; i < MAX_WAVES; i++) {
        waves[i].x = 0.0f;
        waves[i].y = 0.0f;
        waves[i].radius = 0.0f;
        waves[i].active = false;
    }

    InitWindow(WIDTH, HEIGHT, "hello nitish");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) circleX += speed;
        if (IsKeyDown(KEY_LEFT))  circleX -= speed;
        if (IsKeyDown(KEY_DOWN))  circleY += speed;
        if (IsKeyDown(KEY_UP))    circleY -= speed;

        BeginDrawing();
            ClearBackground(BLACK);
            CreateCircle();
            CreateWaves();
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}

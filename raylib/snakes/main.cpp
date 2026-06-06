#include <stdlib.h>
#include <time.h>
#include "raylib.h"

#define HEIGHT 800
#define WIDHT 800
#define MAX_TAIL_LENGTH 100

Vector2 snake_segments[MAX_TAIL_LENGTH];
int tail_length = 3; 
float segment_size = 40.0f; 
Vector2 movement_dir = { 40.0f, 0.0f }; 
Vector2 food_pos = { 0.0f, 0.0f };
float food_radius = 15.0f;

int score = 0;
bool is_food_active = false;
bool game_over = false;

float move_timer = 0.0f;
float move_delay = 0.12f; 

void generate_new_food_position() {
    int grid_w = WIDHT / (int)segment_size;
    int grid_h = HEIGHT / (int)segment_size;
    
    food_pos.x = (float)((rand() % (grid_w - 2)) + 1) * segment_size + (segment_size / 2.0f);
    food_pos.y = (float)((rand() % (grid_h - 2)) + 1) * segment_size + (segment_size / 2.0f);
}

void reset_game() {
    tail_length = 3;
    score = 0;
    game_over = false;
    movement_dir = (Vector2){ segment_size, 0.0f };
    
    for (int i = 0; i < tail_length; i++) {
        snake_segments[i] = (Vector2){ 400.0f - (i * segment_size), 400.0f };
    }
    is_food_active = false;
}

int main(void) {
    srand((unsigned int)time(NULL));
    InitWindow(WIDHT, HEIGHT, "Classic Snake Project");
    SetTargetFPS(60);

    reset_game();

    while (!WindowShouldClose()) {
        
        if (!game_over) {
            if (IsKeyPressed(KEY_RIGHT) && movement_dir.x == 0) movement_dir = (Vector2){ segment_size, 0.0f };
            if (IsKeyPressed(KEY_LEFT)  && movement_dir.x == 0) movement_dir = (Vector2){ -segment_size, 0.0f };
            if (IsKeyPressed(KEY_DOWN)  && movement_dir.y == 0) movement_dir = (Vector2){ 0.0f, segment_size };
            if (IsKeyPressed(KEY_UP)    && movement_dir.y == 0) movement_dir = (Vector2){ 0.0f, -segment_size };
        } else {
            if (IsKeyPressed(KEY_ENTER)) reset_game(); // Restart game if dead
        }

        move_timer += GetFrameTime();
        if (move_timer >= move_delay && !game_over) {
            move_timer = 0.0f; // Reset game timer tick

            for (int i = tail_length - 1; i > 0; i--) {
                snake_segments[i] = snake_segments[i - 1];
            }

            snake_segments[0].x += movement_dir.x;
            snake_segments[0].y += movement_dir.y;

            if (snake_segments[0].x >= WIDHT)  snake_segments[0].x = 0;
            if (snake_segments[0].y >= HEIGHT) snake_segments[0].y = 0;
            if (snake_segments[0].x < 0)       snake_segments[0].x = WIDHT - segment_size;
            if (snake_segments[0].y < 0)       snake_segments[0].y = HEIGHT - segment_size;

            for (int i = 1; i < tail_length; i++) {
                if (snake_segments[0].x == snake_segments[i].x && snake_segments[0].y == snake_segments[i].y) {
                    game_over = true;
                }
            }
        }

        if (!is_food_active && !game_over) {
            generate_new_food_position();
            is_food_active = true;
        }

        if (is_food_active && !game_over) {
            Rectangle head_rect = { snake_segments[0].x, snake_segments[0].y, segment_size, segment_size };
            if (CheckCollisionCircleRec(food_pos, food_radius, head_rect)) {
                is_food_active = false;
                score += 10;
                
                if (tail_length < MAX_TAIL_LENGTH) {
                    tail_length++;
                }
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            if (!game_over) {
                if (is_food_active) {
                    DrawCircleV(food_pos, food_radius, GREEN);
                }

                DrawRectangleV(snake_segments[0], (Vector2){ segment_size, segment_size }, MAROON);
                for (int i = 1; i < tail_length; i++) {
                    DrawRectangle((int)snake_segments[i].x, (int)snake_segments[i].y, (int)segment_size - 2, (int)segment_size - 2, RED);
                }

                DrawText(TextFormat("SCORE: %04d", score), 20, 20, 24, LIGHTGRAY);
            } else {
                DrawText("GAME OVER", WIDHT / 2 - 140, HEIGHT / 2 - 40, 50, RED);
                DrawText(TextFormat("FINAL SCORE: %d", score), WIDHT / 2 - 100, HEIGHT / 2 + 20, 24, WHITE);
                DrawText("PRESS ENTER TO RESTART", WIDHT / 2 - 160, HEIGHT / 2 + 60, 20, DARKGRAY);
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include "raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BALL_SIZE 10
#define BRICK_WIDTH 76
#define BRICK_HEIGHT 28
#define BRICK_GAP 4
#define BRICKS_PER_ROW 10
#define BRICK_ROWS 5
#define MAX_LIVES 3

typedef struct {
    float x, y;
    float speedX, speedY;
    int width, height;
} Paddle;

typedef struct {
    float x, y;
    float speedX, speedY;
    int radius;
} Ball;

typedef struct {
    float x, y;
    int width, height;
    bool active;
} Brick;

void ResetLevel(Ball* ball, Paddle* paddle, Brick* bricks, int level);
void ResetBall(Ball* ball, Paddle* paddle);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout Game");
    SetTargetFPS(60);

    Paddle paddle = { 
        .x = SCREEN_WIDTH/2 - PADDLE_WIDTH/2, 
        .y = SCREEN_HEIGHT - 40, 
        .speedX = 500.0f,
        .speedY = 0,
        .width = PADDLE_WIDTH, 
        .height = PADDLE_HEIGHT 
    };

    Ball ball = { 
        .x = SCREEN_WIDTH/2, 
        .y = SCREEN_HEIGHT/2, 
        .speedX = 300.0f, 
        .speedY = 300.0f, 
        .radius = BALL_SIZE 
    };

    Brick bricks[BRICKS_PER_ROW * BRICK_ROWS];
    int score = 0;
    int level = 1;
    int lives = MAX_LIVES;
    bool gameOver = false;
    bool gameWon = false;

    // Initial brick setup
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICKS_PER_ROW; j++) {
            bricks[i * BRICKS_PER_ROW + j] = (Brick){
                .x = j * (BRICK_WIDTH + BRICK_GAP),
                .y = i * (BRICK_HEIGHT + BRICK_GAP) + 50,
                .width = BRICK_WIDTH,
                .height = BRICK_HEIGHT,
                .active = true
            };
        }
    }

    Color rainbowColors[BRICK_ROWS] = { RED, ORANGE, YELLOW, GREEN, BLUE };

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (!gameOver && !gameWon) {
            // Paddle movement
            if (IsKeyDown(KEY_LEFT) && paddle.x > 0)
                paddle.x -= paddle.speedX * deltaTime;
            if (IsKeyDown(KEY_RIGHT) && paddle.x < SCREEN_WIDTH - paddle.width)
                paddle.x += paddle.speedX * deltaTime;

            // Ball movement
            ball.x += ball.speedX * deltaTime;
            ball.y += ball.speedY * deltaTime;

            // Ball wall collision
            if (ball.x + ball.radius > SCREEN_WIDTH || ball.x - ball.radius < 0)
                ball.speedX = -ball.speedX;
            if (ball.y - ball.radius < 0)
                ball.speedY = -ball.speedY;
            if (ball.y + ball.radius > SCREEN_HEIGHT) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                } else {
                    ResetBall(&ball, &paddle);
                }
            }

            // Ball paddle collision (only from top)
            Rectangle paddleRect = { paddle.x, paddle.y, paddle.width, paddle.height };
            if (CheckCollisionCircleRec((Vector2){ball.x, ball.y}, ball.radius, paddleRect)) {
                if (ball.speedY > 0 && ball.y + ball.radius <= paddle.y + paddle.height/2) {
                    ball.speedY = -ball.speedY;
                    float hitPos = (ball.x - paddle.x) / paddle.width;
                    ball.speedX = (hitPos - 0.5f) * 600.0f;
                }
            }

            // Ball brick collision (only one brick per frame)
            int activeBricks = 0;
            bool brickHit = false;
            for (int i = 0; i < BRICKS_PER_ROW * BRICK_ROWS && !brickHit; i++) {
                if (bricks[i].active) {
                    activeBricks++;
                    Rectangle brickRect = { bricks[i].x, bricks[i].y, bricks[i].width, bricks[i].height };
                    if (CheckCollisionCircleRec((Vector2){ball.x, ball.y}, ball.radius, brickRect)) {
                        bricks[i].active = false;
                        ball.speedY = -ball.speedY;
                        score += 10;
                        brickHit = true; // Stop checking after first hit
                    }
                }
            }
            if (activeBricks == 0) {
                gameWon = true;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangle(paddle.x, paddle.y, paddle.width, paddle.height, BLUE);
        DrawCircle(ball.x, ball.y, ball.radius, WHITE); // Changed ball to white

        // Draw bricks with rainbow colors
        for (int i = 0; i < BRICK_ROWS; i++) {
            for (int j = 0; j < BRICKS_PER_ROW; j++) {
                int index = i * BRICKS_PER_ROW + j;
                if (bricks[index].active) {
                    DrawRectangle(bricks[index].x, bricks[index].y, 
                                bricks[index].width, bricks[index].height, 
                                rainbowColors[i]);
                    DrawRectangleLines(bricks[index].x, bricks[index].y, 
                                     bricks[index].width, bricks[index].height, 
                                     Fade(rainbowColors[i], 0.5f));
                }
            }
        }

        // Draw UI
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
        DrawText(TextFormat("Level: %d", level), SCREEN_WIDTH - 100, 10, 20, WHITE);
        DrawText(TextFormat("Lives: %d", lives), SCREEN_WIDTH/2 - 20, 10, 20, WHITE);

        if (gameOver) {
            DrawText("GAME OVER", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 - 10, 40, DARKGRAY);
            DrawText("Press R to Restart", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 40, 20, DARKGRAY);
        }
        if (gameWon) {
            DrawText("LEVEL COMPLETE!", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 10, 40, GOLD);
            DrawText("Press N for Next Level", SCREEN_WIDTH/2 - 90, SCREEN_HEIGHT/2 + 40, 20, GOLD);
        }

        EndDrawing();

        // Handle restart and next level
        if (gameOver && IsKeyPressed(KEY_R)) {
            lives = MAX_LIVES;
            ResetLevel(&ball, &paddle, bricks, level);
            score = 0;
            level = 1;
            gameOver = false;
        }
        if (gameWon && IsKeyPressed(KEY_N)) {
            level++;
            ResetLevel(&ball, &paddle, bricks, level);
            gameWon = false;
        }
    }

    CloseWindow();
    return 0;
}

void ResetLevel(Ball* ball, Paddle* paddle, Brick* bricks, int level) {
    ResetBall(ball, paddle);
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICKS_PER_ROW; j++) {
            bricks[i * BRICKS_PER_ROW + j].active = true;
        }
    }
    ball->speedX = 300.0f + (level - 1) * 50.0f;
    ball->speedY = 300.0f + (level - 1) * 50.0f;
}

void ResetBall(Ball* ball, Paddle* paddle) {
    ball->x = SCREEN_WIDTH/2;
    ball->y = SCREEN_HEIGHT/2 - 100; // Start higher up (moved up by 100 pixels)
    ball->speedX = 300.0f;
    ball->speedY = -300.0f;
}
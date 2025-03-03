#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1000
#define GROUND_HEIGHT 160 

// Player properties
#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 28
#define PLAYER_SCALE 4
#define PLAYER_SPEED 5
#define JUMP_STRENGTH 15
#define GRAVITY 1.0f 

// Animation properties
#define IDLE_FRAME_COUNT 4
#define WALK_FRAME_COUNT 4
int currentFrame = 0;
int animationSpeed = 6;
int frameCounter = 0;

// Textures
SDL_Texture* backgroundTexture = NULL;
SDL_Texture* playerTexture = NULL;
SDL_Texture* groundTexture = NULL;

// Player struct
typedef struct {
    SDL_Rect rect;
    int velX, velY;
    bool onGround;
    bool facingLeft;
} Player;

void movePlayer(Player* player, int dx, int dy) {
    player->rect.x += dx;
    player->rect.y += dy;
}

void applyGravity(Player* player) {
    if (!player->onGround) {
        player->velY += GRAVITY;
    }
}

void jump(Player* player) {
    if (player->onGround) {
        player->velY = -JUMP_STRENGTH;
        player->onGround = false;
    }
}

void checkCollision(Player* player, int groundY) {
    if (player->rect.y + player->rect.h >= groundY) {
        player->rect.y = groundY - player->rect.h;
        player->velY = 0;
        player->onGround = true;
    } else {
        player->onGround = false;
    }
}

void checkBoundary(Player* player) {
    if (player->rect.x < 0) {
        player->rect.x = 0;
    }
    if (player->rect.x + player->rect.w > SCREEN_WIDTH) {
        player->rect.x = SCREEN_WIDTH - player->rect.w;
    }
}

void handleAnimation(Player* player, int speed) {
    if (player->velX != 0) {
        frameCounter++;
        if (frameCounter >= speed) {
            frameCounter = 0;
            currentFrame = (currentFrame + 1) % WALK_FRAME_COUNT;
        }
    } else {
        currentFrame = 0;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Platformer Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    backgroundTexture = IMG_LoadTexture(renderer, "background_layer_1.png");
    playerTexture = IMG_LoadTexture(renderer, "image.png");
    groundTexture = IMG_LoadTexture(renderer, "dirty.png");

    if (!backgroundTexture || !playerTexture || !groundTexture) {
        printf("Failed to load one or more textures! IMG_Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player player = {{SCREEN_WIDTH / 2 - (PLAYER_WIDTH * PLAYER_SCALE) / 2, SCREEN_HEIGHT - GROUND_HEIGHT - (PLAYER_HEIGHT * PLAYER_SCALE), PLAYER_WIDTH * PLAYER_SCALE, PLAYER_HEIGHT * PLAYER_SCALE}, 0, 0, false, false};

    int running = 1;
    SDL_Event event;

    int groundTileWidth = 250;
    int groundTileHeight = 160;
    int overlapAmount = 40;
    int startX = -groundTileWidth;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_A]) {
            player.velX = -PLAYER_SPEED;
            player.facingLeft = true;
        } else if (keystates[SDL_SCANCODE_D]) {
            player.velX = PLAYER_SPEED;
            player.facingLeft = false;
        } else {
            player.velX = 0;
        }
        if (keystates[SDL_SCANCODE_W] && player.onGround) {
            jump(&player);
        }

        applyGravity(&player);
        movePlayer(&player, player.velX, player.velY);
        checkCollision(&player, SCREEN_HEIGHT - GROUND_HEIGHT);
        checkBoundary(&player);
        handleAnimation(&player, animationSpeed);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        for (int x = startX; x < SCREEN_WIDTH; x += groundTileWidth - overlapAmount) {
            SDL_Rect groundTileRect = {x, SCREEN_HEIGHT - GROUND_HEIGHT, groundTileWidth, groundTileHeight};
            SDL_RenderCopy(renderer, groundTexture, NULL, &groundTileRect);
        }

        SDL_RendererFlip flip = player.facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_Rect playerSourceRect = {currentFrame * PLAYER_WIDTH, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
        SDL_RenderCopyEx(renderer, playerTexture, &playerSourceRect, &player.rect, 0, NULL, flip);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(groundTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}

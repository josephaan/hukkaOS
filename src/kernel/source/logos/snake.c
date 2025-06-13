#include <logos/snake.h>
#include <hardware/ps2/devices/keyboard.h>
#include <display/framebuffer.h>
#include <gui/gui.h>
#include <hardware/apic/apic.h>


#define WIDTH 80
#define HEIGHT 60
#define PIXEL_SIZE 10

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

Point snake[WIDTH * HEIGHT];
int snake_length;
Point food;
Direction dir;



void spawn_food() {
    // Simple random number generation (you might want to implement a better RNG)
    static uint32_t seed = 12345;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    food.x = seed % WIDTH;
    food.y = (seed / WIDTH) % HEIGHT;
}

void init_game() {
    snake_length = 2;
    snake[0] = (Point){WIDTH / 2, HEIGHT / 2};
    dir = RIGHT;
    spawn_food();
}

void draw_game() {
    // Clear screen (black)
    
    for (int y = 1; y < ((HEIGHT * PIXEL_SIZE)-1); y++) {
        for (int x = 1; x < ((WIDTH * PIXEL_SIZE)-1); x++) {
            put_pixel(x, y, 0xffffae);
        }
    }
    

    //create_window("Snake", WIDTH * PIXEL_SIZE, HEIGHT * PIXEL_SIZE);

    // Draw snake (green)
    for (int i = 0; i < snake_length; i++) {
        for (int y = 0; y < PIXEL_SIZE; y++) {
            for (int x = 0; x < PIXEL_SIZE; x++) {
                put_pixel(snake[i].x * PIXEL_SIZE + x, snake[i].y * PIXEL_SIZE + y, 0x00FF00);
            }
        }
    }

    // Draw food (red)
    for (int y = 0; y < PIXEL_SIZE; y++) {
        for (int x = 0; x < PIXEL_SIZE; x++) {
            put_pixel(food.x * PIXEL_SIZE + x, food.y * PIXEL_SIZE + y, 0xFF0000);
        }
    }
}

bool update_game() {
    Point new_head = snake[0];

    switch (dir) {
        case UP:    new_head.y--; break;
        case DOWN:  new_head.y++; break;
        case LEFT:  new_head.x--; break;
        case RIGHT: new_head.x++; break;
    }

    // Check for collisions
    if (new_head.x < 0 || new_head.x >= WIDTH || new_head.y < 0 || new_head.y >= HEIGHT) {
        return false;  // Game over
    }

    for (int i = 0; i < snake_length; i++) {
        if (snake[i].x == new_head.x && snake[i].y == new_head.y) {
            return false;  // Game over
        }
    }

    // Move snake
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i-1];
    }
    snake[0] = new_head;

    // Check if food was eaten
    if (new_head.x == food.x && new_head.y == food.y) {
        snake_length++;
        spawn_food();
    }

    return true;
}

void run_snake_game() {
    init_game();
    
    while (1) {
        draw_game();

        char input = get_keyboard_input();
        switch (input) {
            case 'w': if (dir != DOWN)  dir = UP;    break;
            case 's': if (dir != UP)    dir = DOWN;  break;
            case 'a': if (dir != RIGHT) dir = LEFT;  break;
            case 'd': if (dir != LEFT)  dir = RIGHT; break;
            case 'W': if (dir != DOWN)  dir = UP;    break;
            case 'S': if (dir != UP)    dir = DOWN;  break;
            case 'A': if (dir != RIGHT) dir = LEFT;  break;
            case 'D': if (dir != LEFT)  dir = RIGHT; break;
            case 'q': return;  // Quit game
        }

        if (!update_game()) {
            // Game over
            break;
        }

        sleep(100);  // Control game speed
    }

    
    // Display game over message
    // (You could draw this on the framebuffer)
}
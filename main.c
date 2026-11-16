#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define W       32
#define H       16
#define MAX_INX (H * W)
#define BODY_C  's'
#define HEAD_C  'Q'
#define APPLE_C '*'

#define PASS_WALL 1

#define cls()            printf("\033[H\033[J\033[3J")
#define hide_cursor()    printf("\033[?25l")
#define show_cursor()    printf("\033[?25h")
#define set_cursor(x, y) printf("\033[%d;%dH", x, y)

typedef struct Point {
    char c;
    struct Point* n;
} Point;

typedef struct Snake {
    int len;
    int vx;
    int vy;
    int inx;
    Point* head;
    Point* tail;
} Snake;

void drop_apple(Point* table) {
    while (1) {
        int a_inx = rand() % MAX_INX;

        if (table[a_inx].c == ' ') {
            table[a_inx].c = APPLE_C;
            break;
        }
    }
}

void init_game(Point* table, Snake* snake) {
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            table[(i * W) + j].c = ' ';
            table[(i * W) + j].n = NULL;
        }
    }

    for (int j = 0; j < W; j += (W - 1)) {
        for (int i = 0; i < H; ++i) {
            table[(i * W) + j].c = '|';
        }
    }

    for (int i = 0; i < H; i += (H - 1)) {
        for (int j = 0; j < W; ++j) {
            table[(i * W) + j].c = '-';
        }
    }

    int x = 1;
    int y = 1;
    int inx = (y * W) + x;

    table[inx].c = BODY_C;
    table[inx].n = &table[inx + 1];
    snake->tail = &table[inx];

    for (int i = 1; i < snake->len; ++i) {
        table[inx + i].c = BODY_C;
        table[inx + i].n = &table[inx + i + 1];
    }

    table[inx + snake->len].c = HEAD_C;
    table[inx + snake->len].n = NULL;
    snake->head = &table[inx + snake->len];
    snake->inx = inx + snake->len;

    srand(time(NULL));
    drop_apple(table);
}

void draw_table(const Point* table, const Snake* snake) {
    cls();
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            printf("%c", table[(i * W) + j].c);
        }
        printf("\r\n");
    }
    printf("w: up, a: left, s: down, d: right\n");
    printf("score: %d\n", snake->len - 3);
}

int kbhit(void);
void get_dir(int* vx, int* vy) {
    char c = ' ';
    if (kbhit()) {
        c = getchar();
    }

    switch (c) {
        case 'a':
            *vx = -1;
            *vy = 0;
            break;
        case 's':
            *vy = 1;
            *vx = 0;
            break;
        case 'w':
            *vy = -1;
            *vx = 0;
            break;
        case 'd':
            *vx = 1;
            *vy = 0;
            break;
        default:; break;
    }
}

void update_table(Point* table, Snake* snake) {
    int vx = 0;
    int vy = 0;
    get_dir(&vx, &vy);

    if (!(snake->vx == -vx || snake->vy == -vy)) {
        snake->vx = vx;
        snake->vy = vy;
    }
    int inx = snake->inx + snake->vx + (snake->vy * W);

    if (table[inx].c != APPLE_C) {
        Point* temp = snake->tail->n;
        snake->tail->n = NULL;
        snake->tail->c = ' ';
        snake->tail = temp;
    } else {
        snake->len += 1;
        drop_apple(table);
    }

#if PASS_WALL == 1
    if (table[inx].c == BODY_C) {
        printf("Illegal index, you lost!\n");
        printf("Index %d\n", inx);
        exit(1);
    }

    if (table[inx].c == '|') {
        inx = (inx % W) > (W / 2) ? inx - (W - 2) : inx + (W - 2);
    } else if (table[inx].c == '-') {
        inx = (inx / W) > (H / 2) ? inx - ((H - 2) * W) : inx + ((H - 2) * W);
    }
#else
    if (table[inx].c != ' ' && table[inx].c != APPLE_C) {
        printf("Illegal index, you lost!\n");
        exit(1);
    }
#endif

    table[inx].n = NULL;
    table[inx].c = HEAD_C;
    snake->head->n = &table[inx];
    snake->head->c = BODY_C;
    snake->head = &table[inx];
    snake->inx = inx;
}

int main(int argc, char** argv) {
    Point table[W][H];
    Snake snake = {.len = 3, .vx = 1, .vy = 0};

    init_game(&table[0][0], &snake);

    while (1) {
        update_table(&table[0][0], &snake);
        draw_table(&table[0][0], &snake);

        usleep(150000);
    }
}

int kbhit(void) {
    struct termios oldt;
    struct termios newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
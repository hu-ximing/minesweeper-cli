#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>

#define GAME_CONTINUE 0
#define GAME_OVER 1
#define GAME_WON 2

typedef struct Cell
{
    int is_mine;
    int is_revealed;
    int is_flagged;
    int num_mines;
} Cell;

typedef struct Board
{
    int width;
    int height;
    int num_mines;
    int num_revealed;
    Cell *cells;
} Board;

void init_board(Board *board, int width, int height, int num_mines)
{
    board->width = width;
    board->height = height;
    board->num_mines = num_mines;
    board->num_revealed = 0;
    board->cells = malloc(sizeof(Cell) * width * height);
    for (int i = 0; i < width * height; i++)
    {
        board->cells[i].is_mine = 0;
        board->cells[i].is_revealed = 0;
        board->cells[i].is_flagged = 0;
        board->cells[i].num_mines = 0;
    }
}

void clear_board(Board *board)
{
    board->num_revealed = 0;
    for (int i = 0; i < board->width * board->height; i++)
    {
        board->cells[i].is_mine = 0;
        board->cells[i].is_revealed = 0;
        board->cells[i].is_flagged = 0;
        board->cells[i].num_mines = 0;
    }
}

void free_board(Board *board)
{
    free(board->cells);
}

void print_board(Board *board, int cursor_x, int cursor_y)
{
    for (int y = 0; y < board->height; y++)
    {
        for (int x = 0; x < board->width; x++)
        {
            Cell *cell = &board->cells[y * board->width + x];
            move(y, x * 3 + 1);
            if (cursor_x == x && cursor_y == y)
            {
                attron(A_REVERSE);
            }
            else
            {
                attroff(A_REVERSE);
            }
            if (cell->is_revealed)
            {
                if (cell->is_mine)
                {
                    printw("💣");
                }
                else if (cell->num_mines == 0)
                {
                    printw(" ");
                }
                else
                {
                    printw("%d", cell->num_mines);
                }
            }
            else
            {
                if (cell->is_flagged)
                {
                    printw("🚩");
                }
                else
                {
                    printw("-");
                }
            }
        }
    }
}

void shuffle(int *arr, int n)
{
    if (n > 1)
    {
        for (int i = 0; i < n - 1; i++)
        {
            int j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = arr[j];
            arr[j] = arr[i];
            arr[i] = t;
        }
    }
}

void place_mines(Board *board)
{
    int *mine_indices = malloc(sizeof(int) * board->width * board->height);
    for (int i = 0; i < board->width * board->height; i++)
    {
        mine_indices[i] = i;
    }
    shuffle(mine_indices, board->width * board->height);
    for (int i = 0; i < board->num_mines; i++)
    {
        board->cells[mine_indices[i]].is_mine = 1;
    }
}

void count_mines(Board *board)
{
    for (int y = 0; y < board->height; y++)
    {
        for (int x = 0; x < board->width; x++)
        {
            Cell *cell = &board->cells[y * board->width + x];
            if (cell->is_mine)
            {
                for (int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        if (x + i >= 0 && x + i < board->width && y + j >= 0 && y + j < board->height)
                        {
                            Cell *adjacent_cell = &board->cells[(y + j) * board->width + (x + i)];
                            adjacent_cell->num_mines++;
                        }
                    }
                }
            }
        }
    }
}

void reveal_cell(Board *board, int x, int y)
{
    Cell *cell = &board->cells[y * board->width + x];
    if (cell->is_revealed || cell->is_flagged)
    {
        return;
    }
    cell->is_revealed = 1;
    board->num_revealed++;
    if (cell->num_mines > 0)
    {
        return;
    }
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (x + i >= 0 && x + i < board->width && y + j >= 0 && y + j < board->height)
            {
                reveal_cell(board, x + i, y + j);
            }
        }
    }
}

void reveal_all(Board *board)
{
    for (int i = 0; i < board->width * board->height; i++)
    {
        board->cells[i].is_revealed = 1;
    }
}

void flag_cell(Board *board, int x, int y)
{
    Cell *cell = &board->cells[y * board->width + x];
    if (cell->is_revealed)
    {
        return;
    }
    cell->is_flagged = !cell->is_flagged;
}

int select_cell(Board *board, int x, int y)
{
    Cell *cell = &board->cells[y * board->width + x];
    if (cell->is_flagged)
    {
        return GAME_CONTINUE;
    }
    // The first step must be safe and will unlock at least 1 adjacent cell.
    while (board->num_revealed == 0 && cell->num_mines > 0)
    {
        clear_board(board);
        place_mines(board);
        count_mines(board);
    }
    if (cell->is_mine)
    {
        reveal_all(board);
        return GAME_OVER;
    }
    reveal_cell(board, x, y);
    if (board->num_revealed == board->width * board->height - board->num_mines)
    {
        return GAME_WON;
    }
    return GAME_CONTINUE;
}

// TODO: Add a timer.
// TODO: Add a high score table.
// TODO: Add a menu.
// TODO: Add a border around the board.
// TODO: Add letters on the top, and numbers on the left side.
int main(int argc, char **argv)
{
    int width = 9;
    int height = 9;
    int num_mines = 10;
    int level = 1;
    int option;
    while ((option = getopt(argc, argv, ":w:h:m:l:")) != -1)
    {
        switch (option)
        {
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
            break;
        case 'm':
            num_mines = atoi(optarg);
            break;
        case 'l':
            level = atoi(optarg);
            switch (level)
            {
            case 0:
                width = 9;
                height = 9;
                num_mines = 5;
                break;
            case 1:
                width = 9;
                height = 9;
                num_mines = 10;
                break;
            case 2:
                width = 16;
                height = 16;
                num_mines = 40;
                break;
            case 3:
                width = 30;
                height = 16;
                num_mines = 99;
                break;
            }
            break;
        case ':':
            printf("Option -%c requires an operand", optopt);
            exit(EXIT_FAILURE);
        case '?':
            printf("Usage: %s [-w width] [-h height] [-m num_mines]\n", argv[0]);
            printf("       %s [-l level]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; optind < argc; optind++, i++)
    {
        switch (i)
        {
        case 0:
            width = atoi(argv[optind]);
            break;
        case 1:
            height = atoi(argv[optind]);
            break;
        case 2:
            num_mines = atoi(argv[optind]);
            break;
        default:
            break;
        }
    }

    srand((unsigned int)time(NULL));
    setlocale(LC_ALL, "");

    Board *board = malloc(sizeof(Board));
    init_board(board, width, height, num_mines);
    place_mines(board);
    count_mines(board);
    int x = width / 2;
    int y = height / 2;
    int ch;
    int state = GAME_CONTINUE;
    int quit = 0;

    initscr();            // Start curses mode
    cbreak();             // Line buffering disabled
    keypad(stdscr, TRUE); // We get F1, F2 etc..
    noecho();             // Don't echo() while we do getch
    curs_set(0);          // Do not display cursor
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    print_board(board, x, y);
    refresh();
    while (1)
    {
        ch = getch();
        switch (ch)
        {
        case ' ':
        case 'f':
            flag_cell(board, x, y);
            break;
        case '\n':
            state = select_cell(board, x, y);
            break;
        case 'q':
            quit = 1;
            break;
        case KEY_LEFT:
        case 'a':
            if (x > 0)
                x--;
            break;
        case KEY_RIGHT:
        case 'd':
            if (x < width - 1)
                x++;
            break;
        case KEY_UP:
        case 'w':
            if (y > 0)
                y--;
            break;
        case KEY_DOWN:
        case 's':
            if (y < height - 1)
                y++;
            break;
        default:
            break;
        }
        if (quit)
            break;
        clear();
        print_board(board, x, y);
        if (state == GAME_OVER)
        {
            mvprintw(height + 1, 0, "Game over.");
            refresh();
            getch();
            break;
        }
        if (state == GAME_WON)
        {
            mvprintw(height + 1, 0, "You win! Congratulations.");
            refresh();
            getch();
            break;
        }
        refresh();
    }
    free_board(board);
    free(board);
    endwin(); // End curses mode
    return 0;
}

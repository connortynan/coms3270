#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 20

typedef struct
{
    char letter;
    uint8_t overlap_count;
} board_cell;

typedef enum
{
    DIAGONAL_UP,
    RIGHT,
    DIAGONAL_DOWN,
    DOWN,
    DIRECTION_COUNT
} word_direction;

typedef struct
{
    uint16_t x;
    uint16_t y;
    word_direction dir;
} word_location;

uint16_t board_size;
#define board_index(x, y) (((x) * board_size) + (y))
board_cell *board;

bool valid_start(uint16_t x, uint16_t y, word_direction d, int word_len);
board_cell *letter_location(uint16_t start_x, uint16_t start_y, word_direction d, int letter_of_word);
bool word_fits(uint16_t start_x, uint16_t start_y, word_direction d, char *word);

void insert_word(uint16_t start_x, uint16_t start_y, word_direction d, char *word);
void remove_word(uint16_t start_x, uint16_t start_y, word_direction d, int word_len);

// Writes to the given buffer in random order (must be malloced!) and returns the working size of the array
size_t generate_locations(word_location *locations, int word_len);

void shuffle_array(word_location *arr, size_t size);

int solve_words(int word_count, char *word_list[]);
void reset_board();
void fill_board();
void print_board();
void print_word_list(int word_count, char *word_list[]);

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (argc < 2)
    {
        fprintf(stderr,
                "ERROR: invalid arguments!\n"
                " Usage: %s <size> [word1, word2, ...]\n"
                " size MUST be a positive integer <%d\n",
                argv[0], MAX_SIZE);
        return 1;
    }

    // Get the size from the first argument
    board_size = atoi(argv[1]);

    if (board_size < 0 || board_size > MAX_SIZE)
    {
        fprintf(stderr,
                "ERROR: invalid arguments!\n"
                " Usage: %s <size> [word1, word2, ...]\n"
                " size MUST be a positive integer <%d (entered: %s)\n",
                argv[0], MAX_SIZE, argv[1]);
        return 1;
    }

    board = (board_cell *)malloc(board_size * board_size * sizeof(board_cell));
    if (!board)
    {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }

    reset_board();

    int num_words = argc - 2;

    if (num_words < 1)
    {
        fprintf(stderr,
                "ERROR: invalid arguments!\n"
                " Usage: %s <size> [word1, word2, ...]\n"
                " Enter at least one word to create the puzzle\n",
                argv[0]);
        return 1;
    }

    if (solve_words(num_words, &argv[2]))
    {
        fprintf(stderr, "Failed to solve word search generation\n");
        free(board);
        return 1;
    }

    fill_board();
    print_board();
    print_word_list(num_words, &argv[2]);

    free(board);
    return 0;
}

int solve_words_rec(int word_num, int word_count, char *word_list[])
{
    if (word_num == word_count)
    {
        return 0; // Success
    }

    char *word = word_list[word_num];
    int word_len = strlen(word);

    word_location *possible_locations = (word_location *)malloc(board_size * board_size * DIRECTION_COUNT * sizeof(word_location));

    if (!possible_locations)
    {
        fprintf(stderr, "Memory allocation failed in solve_words_rec\n");
        return 0; // Success
    }

    size_t num_locations = generate_locations(possible_locations, word_len);

    size_t i;
    for (i = 0; i < num_locations; i++)
    {
        word_location loc = possible_locations[i];
        if (word_fits(loc.x, loc.y, loc.dir, word))
        {
            insert_word(loc.x, loc.y, loc.dir, word);
            if (!solve_words_rec(word_num + 1, word_count, word_list))
            {
                free(possible_locations);
                return 0; // Found a solution, success
            }
            remove_word(loc.x, loc.y, loc.dir, word_len);
        }
    }
    free(possible_locations);
    return 1; // No valid placement for this word, backtrack
}

int solve_words(int word_count, char *word_list[])
{
    return solve_words_rec(0, word_count, word_list);
}

size_t generate_locations(word_location *locations, int word_len)
{
    int x, y;
    word_direction d;
    size_t count = 0;
    word_location loc;
    for (x = 0; x < board_size; x++)
    {
        loc.x = x;
        for (y = 0; y < board_size; y++)
        {
            loc.y = y;
            for (d = 0; d < DIRECTION_COUNT; d++)
            {
                loc.dir = d;
                if (valid_start(x, y, d, word_len))
                {
                    locations[count++] = loc;
                }
            }
        }
    }
    shuffle_array(locations, count);
    return count;
}

void shuffle_array(word_location *arr, size_t count)
{
    if (count < 2)
    {
        return;
    }

    word_location t;
    size_t i, j;
    for (i = 0; i < count; i++)
    {
        j = i + (rand() % (count - i));
        t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}

void reset_board()
{
    int x, y;
    for (y = 0; y < board_size; y++)
    {
        for (x = 0; x < board_size; x++)
        {
            board_cell *cell = &board[board_index(x, y)];
            cell->letter = 0;
            cell->overlap_count = 0;
        }
    }
}

void fill_board()
{
    int x, y;
    for (y = 0; y < board_size; y++)
    {
        for (x = 0; x < board_size; x++)
        {
            board_cell *cell = &board[board_index(x, y)];
            cell->letter = cell->overlap_count ? cell->letter : ((rand() % 26) + 'a');
        }
    }
}

void print_board()
{
    int x, y;
    for (y = 0; y < board_size; y++)
    {
        for (x = 0; x < board_size; x++)
        {
            printf("%c ", board[board_index(x, y)].letter ? board[board_index(x, y)].letter : '-');
        }
        printf("\n");
    }
}

void print_word_list(int word_count, char *word_list[])
{
    int i;
    printf("+----------------------+\n"
           "|      WORD LIST:      |\n");

    for (i = 0; i < word_count; i++)
    {
        char *word = word_list[i];
        printf("| %-20s |\n", word);
    }
    printf("+----------------------+\n");
}

bool valid_start(uint16_t start_x, uint16_t start_y, word_direction d, int word_len)
{
    switch (d)
    {
    case DIAGONAL_UP:
        return (start_x <= board_size - word_len) && (start_y > word_len - 1);
    case RIGHT:
        return (start_x <= board_size - word_len);
    case DIAGONAL_DOWN:
        return (start_x <= board_size - word_len) && (start_y <= board_size - word_len);
    case DOWN:
        return (start_y <= board_size - word_len);
    default:
        fprintf(stderr, "ERROR: undefined direction in valid_start(%d, %d, __%d__, %d)\n", start_x, start_y, d, word_len);
        return 0;
    }
}

board_cell *letter_location(uint16_t start_x, uint16_t start_y, word_direction d, int letter_of_word)
{
    switch (d)
    {
    case DIAGONAL_UP:
        return &board[board_index(start_x + letter_of_word, start_y - letter_of_word)];
    case RIGHT:
        return &board[board_index(start_x + letter_of_word, start_y)];
    case DIAGONAL_DOWN:
        return &board[board_index(start_x + letter_of_word, start_y + letter_of_word)];
    case DOWN:
        return &board[board_index(start_x, start_y + letter_of_word)];
    default:
        fprintf(stderr, "ERROR: undefined direction in coord_of_letter(%d, %d, __%d__, %d)\n", start_x, start_y, d, letter_of_word);
        return NULL;
    }
}

bool word_fits(uint16_t start_x, uint16_t start_y, word_direction d, char *word)
{
    int i;
    char letter;
    int word_len = strlen(word);
    for (i = 0; i < word_len; i++)
    {
        letter = word[i];
        board_cell *cell = letter_location(start_x, start_y, d, i);
        if (!cell)
        {
            fprintf(stderr, " ERROR: letter_overlaps OOB\n");
            return false;
        }

        if (cell->overlap_count > 0 && cell->letter != letter)
        {
            return false;
        }
    }
    return true;
}

void insert_word(uint16_t start_x, uint16_t start_y, word_direction d, char *word)
{
    int i;
    int word_len = strlen(word);
    for (i = 0; i < word_len; i++)
    {
        board_cell *cell = letter_location(start_x, start_y, d, i);
        cell->letter = word[i];
        cell->overlap_count++;
    }
}

void remove_word(uint16_t start_x, uint16_t start_y, word_direction d, int word_len)
{
    int i;
    for (i = 0; i < word_len; i++)
    {
        board_cell *cell = letter_location(start_x, start_y, d, i);
        cell->overlap_count--;
        if (cell->overlap_count <= 0)
        {
            cell->letter = 0;
        }
    }
}
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 20

size_t itr_count = 0;

typedef struct
{
    char letter;
    uint8_t overlap_count;
} board_cell;

uint8_t board_size;
#define board_index(x, y) (((x) * board_size) + (y))
board_cell *board;

bool valid_start(int x, int y, int d, int word_len);
board_cell *letter_location(int start_x, int start_y, int d, int letter_of_word);
bool word_fits(int start_x, int start_y, int d, char *word);

void insert_word(int start_x, int start_y, int d, char *word);
void remove_word(int start_x, int start_y, int d, int word_len);

bool solve_words(int word_count, char *word_list[]);
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
        return -1;
    }

    // Get the size from the first argument (after the program)
    board_size = atoi(argv[1]);

    if (board_size < 0 || board_size > MAX_SIZE)
    {
        fprintf(stderr,
                "ERROR: invalid arguments!\n"
                " Usage: %s <size> [word1, word2, ...]\n"
                " size MUST be a positive integer <%d (entered: %s)\n",
                argv[0], MAX_SIZE, argv[1]);
        return -1;
    }

    board = (board_cell *)malloc(board_size * board_size * sizeof(board_cell));
    if (!board)
    {
        fprintf(stderr, "Memory allocation error\n");
        free(board);
        return -1;
    }

    if (argc >= 3)
    {
        if (!solve_words(argc - 2, &argv[2]))
        {
            fprintf(stderr, "Failed to solve word search generation\n");
            free(board);
            return -1;
        }
    }

    fill_board();
    print_board();
    print_word_list(argc - 2, &argv[2]);

    free(board);
    return 0;
}

bool solve_words_rec(int stack_idx, int word_count, char *word_list[])
{
    if (stack_idx == word_count)
    {
        return true;
    }

    char *word = word_list[stack_idx];
    int word_len = strlen(word);

    int x = rand() % board_size;
    int y = rand() % board_size;
    int d = rand() % 4;

    int dx, dy, dd;

    for (dx = 0; dx < board_size; dx++, x = (x + 1) % board_size)
    {
        for (dy = 0; dy < board_size; dy++, y = (y + 1) % board_size)
        {
            for (dd = 0; dd < 4; dd++, d = (d + 1) % 4)
            {
                itr_count++;
                if (valid_start(x, y, d, word_len) && word_fits(x, y, d, word))
                {
                    insert_word(x, y, d, word);
                    if (solve_words_rec(stack_idx + 1, word_count, word_list))
                    {
                        return true; // Found a solution
                    }
                    remove_word(x, y, d, word_len); // Backtrack
                }
            }
        }
    }

    return false; // No valid placement for this word
}

bool solve_words(int word_count, char *word_list[])
{
    return solve_words_rec(0, word_count, word_list);
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
            printf("%c", board[board_index(x, y)].letter ? board[board_index(x, y)].letter : '-');
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

bool valid_start(int x, int y, int d, int word_len)
{
    switch (d)
    {
    case 0: // Diagonal up
        return (x <= board_size - word_len) && (y > word_len - 1);
    case 1: // Rightward
        return (x <= board_size - word_len);
    case 2: // Diagonal down
        return (x <= board_size - word_len) && (y <= board_size - word_len);
    case 3: // Downward
        return (y <= board_size - word_len);
    default:
        fprintf(stderr, "ERROR: undefined direction in valid_start(%d, %d, __%d__, %d)\n", x, y, d, word_len);
        return 0;
    }
}

board_cell *letter_location(int start_x, int start_y, int d, int letter_of_word)
{
    switch (d)
    {
    case 0: // Diagonal up
        return &board[board_index(start_x + letter_of_word, start_y - letter_of_word)];
    case 1: // Rightward
        return &board[board_index(start_x + letter_of_word, start_y)];
    case 2: // Diagonal down
        return &board[board_index(start_x + letter_of_word, start_y + letter_of_word)];
    case 3: // Downward
        return &board[board_index(start_x, start_y + letter_of_word)];
    default:
        fprintf(stderr, "ERROR: undefined direction in coord_of_letter(%d, %d, __%d__, %d)\n", start_x, start_y, d, letter_of_word);
        return NULL;
    }
}

bool word_fits(int start_x, int start_y, int d, char *word)
{
    int i;
    char letter;

    for (i = 0; i < strlen(word); i++)
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

void insert_word(int start_x, int start_y, int d, char *word)
{
    int i;
    for (i = 0; i < strlen(word); i++)
    {
        board_cell *cell = letter_location(start_x, start_y, d, i);
        cell->letter = word[i];
        cell->overlap_count++;
    }
}

void remove_word(int start_x, int start_y, int d, int word_len)
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
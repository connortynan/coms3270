#include <stdio.h>

#include "generator.h"
#include "util/bool_array.h"
#include "util/pathing.h"
#include "util/img_proc.h"
#include "util/noise.h"

#define GENERATOR_ROOM_BUCKET_SIZE 256

/**
 * @brief Attempts to place rooms within a dungeon using specified parameters.
 *
 * This function generates rooms and attempts to fit them into the dungeon space
 * according to the given constraints. If the minimum required number of rooms
 * is successfully placed, it returns the number of rooms placed. If room placement
 * fails, the function returns 0.
 *
 * @param dungeon Pointer to the dungeon data structure where rooms are placed.
 * @param params  Pointer to generator parameters specifying room size constraints,
 *                dungeon size, and the number of rooms to place.
 *
 * @return 0 if rooms successfully placed, or 1 if generation fails.
 */
int _generator_place_rooms(dungeon_data *dungeon, const generator_parameters *params);

/**
 * @brief Create pathways from each adjecent pair in the dungeon
 *
 * This function uses `util/pathing.h` to get paths between the centers of rooms,
 * and overwrites rocks that are part of the path. The path is weighted based on
 * the rock hardness
 *
 * @param dungeon Pointer to the current dungeon where the path will be written to
 *
 * @return Returns 0 on success, or an error code if the operation fails.
 */
int _generator_connect_rooms(dungeon_data *dungeon);

/**
 * @brief Fills all cells within the defined room in the dungeon with a specified value.
 *
 * This function takes the given room data and writes the provided value to every cell within the defined room bounds
 * in the dungeon. The room must fit within the dungeon's bounds, as this function does not perform boundary checks.
 * Ensure that the room fits by validating with `generator_room_fits()` before calling this function to avoid undefined behavior.
 *
 * @param dungeon Pointer to the current dungeon where the room will be written.
 * @param room_data Pointer to the room data that defines the room's position and dimensions.
 * @param value Pointer to a `dungeon_cell` struct containing the type and hardness for the room's cells.
 *              Typically, this is `ROOM` with hardness `0`.
 *
 * @return Returns 0 on success, or an error code if the operation fails.
 */
int generator_apply_room(dungeon_data *dungeon, const dungeon_room_data *room_data, const dungeon_cell *value);

/**
 * @brief Reverts the changes applied by `generator_apply_room()`.
 *
 * This function removes a previously applied room from the dungeon, restoring the dungeon's cells to their
 * original state.
 *
 * @param dungeon Pointer to the current dungeon where the room was written.
 * @param room_data Pointer to the room data that defines the bounds of the room to be undone.
 *
 * @return Returns 0 on success, or an error code if the operation fails.
 */
int generator_undo_room(dungeon_data *dungeon, const dungeon_room_data *room_data);

/**
 * @brief Checks if a given room fits within the bounds of the dungeon.
 *
 * This function verifies that the specified room defined by `room_data` fits completely within the dungeon's
 * dimensions without overlapping or exceeding boundaries.
 *
 * @param dungeon Pointer to the current dungeon where the room is being checked.
 * @param room_data Pointer to the room data defining the room's position and dimensions.
 *
 * @return Returns 1 if the room fits within the dungeon; otherwise, returns 0.
 */
int generator_room_fits(dungeon_data *dungeon, const dungeon_room_data *room_data);

int generator_set_rock_hardness(dungeon_data *dungeon, const generator_parameters *params);

int _generator_place_rooms(dungeon_data *dungeon, const generator_parameters *params)
{
    int i;

    dungeon_room_data bucket[GENERATOR_ROOM_BUCKET_SIZE] = {0};
    dungeon_room_data room;

    // Fill bucket with random rooms
    for (i = 0; i < GENERATOR_ROOM_BUCKET_SIZE; i++)
    {
        room.width = params->min_room_width + (rand() % (params->max_room_width - params->min_room_width + 1));
        room.height = params->min_room_height + (rand() % (params->max_room_height - params->min_room_height + 1));

        room.center_x = (room.width / 2 + 1) + (rand() % (DUNGEON_WIDTH - room.width - 1));
        room.center_y = (room.height / 2 + 1) + (rand() % (DUNGEON_HEIGHT - room.height - 1));

        bucket[i] = room;
    }

    uint16_t max_rooms = params->min_num_rooms + (rand() % (params->max_num_rooms - params->min_num_rooms + 1));

    dungeon->num_rooms = 0;
    size_t *placed_room_idxs = (size_t *)malloc(params->max_num_rooms * sizeof(*placed_room_idxs));

    dungeon_cell room_cell = {ROOM, 0};

    size_t bucket_idx = 0;
    while (dungeon->num_rooms < max_rooms)
    {
        dungeon_room_data *room_attempt = &bucket[bucket_idx];
        if (generator_room_fits(dungeon, room_attempt))
        {
            generator_apply_room(dungeon, room_attempt, &room_cell);
            dungeon->rooms[dungeon->num_rooms] = *room_attempt;
            placed_room_idxs[dungeon->num_rooms++] = bucket_idx;
        }
        bucket_idx++;
        if (bucket_idx >= GENERATOR_ROOM_BUCKET_SIZE)
        {
            if (dungeon->num_rooms >= params->min_num_rooms)
            {
                free(placed_room_idxs);
                return 0;
            }

            generator_undo_room(dungeon, &dungeon->rooms[--dungeon->num_rooms]);

            if (dungeon->num_rooms <= 0)
            {
                // The bucket was impossible, we checked all permutations :(
                free(placed_room_idxs);
                return 1;
            }
            bucket_idx = placed_room_idxs[dungeon->num_rooms] + 1;
        }
    }
    free(placed_room_idxs);
    return 0;
}

int _generator_connect_rooms(dungeon_data *dungeon)
{
    vector *path;
    size_t i, j;
    uint16_t x, y;
    uint16_t ax, ay, bx, by;

    uint8_t *weights = (uint8_t *)malloc(DUNGEON_WIDTH * DUNGEON_HEIGHT * sizeof(*weights));
    i = 0;
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            weights[i++] = dungeon->cells[x][y].hardness;
        }
    }

    for (i = 0; i < dungeon->num_rooms; i++)
    {
        uint16_t next_idx = (i + 1) % dungeon->num_rooms;
        ax = dungeon->rooms[i].center_x;
        ay = dungeon->rooms[i].center_y;
        bx = dungeon->rooms[next_idx].center_x;
        by = dungeon->rooms[next_idx].center_y;

        path = pathing_solve(weights, DUNGEON_WIDTH, DUNGEON_HEIGHT, ax, ay, bx, by);
        if (!path)
            continue;

        for (j = 0; j < path->size; j++)
        {
            vec2_u16 *loc = (vec2_u16 *)vector_at(path, j);
            dungeon_cell *cell = &dungeon->cells[loc->x][loc->y];
            if (cell->type == ROCK)
            {
                cell->type = CORRIDOR;
                cell->hardness = 0;
            }
        }
        free(path);
    }
    free(weights);
    return 0;
}

int generator_generate_dungeon(dungeon_data *dungeon, const generator_parameters *params)
{
    if (!params)
    {
        fprintf(stderr, "Error: generator parameters cannot be NULL\n");
        return 1;
    }

    dungeon->north = dungeon->east = dungeon->south = dungeon->west = dungeon->up = dungeon->down = NULL;

    uint16_t x, y;
    static const dungeon_cell rock = {ROCK, 255};
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            dungeon->cells[x][y] = rock;
        }
    }

    dungeon->rooms = (dungeon_room_data *)malloc((params->max_num_rooms) * sizeof(dungeon_room_data));
    if (!dungeon->rooms)
        return 1;

    while (_generator_place_rooms(dungeon, params))
        ;

    dungeon->rooms = (dungeon_room_data *)realloc(dungeon->rooms, dungeon->num_rooms * sizeof(dungeon_room_data *));
    if (!dungeon->rooms)
        return 1;

    generator_set_rock_hardness(dungeon, params);

    _generator_connect_rooms(dungeon);

    return 0;
}

int generator_apply_room(dungeon_data *dungeon, const dungeon_room_data *room_data, const dungeon_cell *value)
{
    const uint16_t x = room_data->center_x - room_data->width / 2;
    const uint16_t y = room_data->center_y - room_data->height / 2;

    uint16_t dx, dy;

    for (dx = 0; dx < room_data->width; dx++)
    {
        for (dy = 0; dy < room_data->height; dy++)
        {
            dungeon->cells[x + dx][y + dy] = *value;
        }
    }

    return 0;
}

int generator_undo_room(dungeon_data *dungeon, const dungeon_room_data *room_data)
{
    const uint16_t x = room_data->center_x - room_data->width / 2;
    const uint16_t y = room_data->center_y - room_data->height / 2;

    uint16_t dx, dy;

    dungeon_cell default_value = {
        .type = ROCK,
        .hardness = 0,
    };

    for (dx = 0; dx < room_data->width; dx++)
    {
        for (dy = 0; dy < room_data->height; dy++)
        {
            dungeon->cells[x + dx][y + dy] = default_value;
        }
    }

    return 0;
}

int generator_room_fits(dungeon_data *dungeon, const dungeon_room_data *room_data)
{
    int i;

    const int new_min_x = room_data->center_x - room_data->width / 2;
    const int new_max_x = room_data->center_x + (room_data->width - 1) / 2;
    const int new_min_y = room_data->center_y - room_data->height / 2;
    const int new_max_y = room_data->center_y + (room_data->height - 1) / 2;

    if (new_min_x < 1 || new_max_x >= DUNGEON_WIDTH - 1 ||
        new_min_y < 1 || new_max_y >= DUNGEON_HEIGHT - 1)
    {
        return 0;
    }

    // Check against all existing rooms for overlap or adjacency
    for (i = 0; i < dungeon->num_rooms; i++)
    {
        dungeon_room_data *existing_room = &dungeon->rooms[i];

        const int existing_min_x = existing_room->center_x - existing_room->width / 2;
        const int existing_max_x = existing_room->center_x + (existing_room->width - 1) / 2;
        const int existing_min_y = existing_room->center_y - existing_room->height / 2;
        const int existing_max_y = existing_room->center_y + (existing_room->height - 1) / 2;

        if (new_max_x + 1 >= existing_min_x &&
            new_min_x - 1 <= existing_max_x &&
            new_max_y + 1 >= existing_min_y &&
            new_min_y - 1 <= existing_max_y)
        {
            return 0;
        }
    }

    return 1;
}

int _generator_get_nearest_room(dungeon_data *dungeon, uint16_t x, uint16_t y)
{
    int closest_room_idx = -1;
    int64_t min_dist_sq = INT64_MAX;
    int i, dx, dy, dist_sq;
    for (i = 0; i < dungeon->num_rooms; i++)
    {
        dx = dungeon->rooms[i].center_x - x;
        dy = dungeon->rooms[i].center_y - y;
        dist_sq = dx * dx + dy * dy;
        if (dist_sq < min_dist_sq)
        {
            min_dist_sq = dist_sq;
            closest_room_idx = i;
        }
    }
    return closest_room_idx;
}

int generator_set_rock_hardness(dungeon_data *dungeon, const generator_parameters *params)
{
    size_t i;
    size_t n = dungeon->num_rooms;
    int *room_hardnesses = (int *)malloc(n * sizeof(*room_hardnesses));
    if (!room_hardnesses)
        return 1;

    for (i = 0; i < n; i++)
    {
        room_hardnesses[i] = params->min_rock_hardness + (rand() % (params->max_rock_hardness - params->min_rock_hardness + 1));
    }

    uint8_t *hardness = (uint8_t *)malloc(DUNGEON_WIDTH * DUNGEON_HEIGHT * sizeof(*hardness));
    if (!hardness)
    {
        free(room_hardnesses);
        return 1;
    }
    int x, y;

    i = 0;
    // Set rock hardness based on rooms
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            hardness[i++] = room_hardnesses[_generator_get_nearest_room(dungeon, x, y)];
        }
    }
    free(room_hardnesses);

    // Blur hardness based on parameter
    for (i = 0; i < params->rock_hardness_smoothness; i++)
        gaussian_blur(hardness, DUNGEON_WIDTH, DUNGEON_HEIGHT);

    // Add perlin noise to hardness
    float x_off = rand();
    float y_off = rand();

    if (params->rock_hardness_noise_amount > 0.f)
    {
        i = 0;
        uint8_t noisy;
        for (y = 0; y < DUNGEON_HEIGHT; y++)
        {
            for (x = 0; x < DUNGEON_WIDTH; x++)
            {
                noisy = (uint8_t)(hardness[i] + layered_noise_perlin(x + x_off, 2.f * y + y_off, params->rock_hardness_noise_amount, 1000.f, 4, 0.5f, 2.f));
                hardness[i++] = VALUE_CLAMP(noisy, 0, 255);
            }
        }
    }

    // Write hardness to rocks and set non-rocks to 0 hardness
    i = 0;
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            if (dungeon->cells[x][y].type == ROCK)
            {
                if (x == 0 || x == DUNGEON_WIDTH - 1 || y == 0 || y == DUNGEON_HEIGHT - 1) // Put hardness of 255 on the border
                {
                    dungeon->cells[x][y].hardness = 255;
                }
                else
                {
#ifdef DEBUG_DEV_FLAGS
                    dungeon->cells[x][y].type = DEBUG_SHOW_HARDNESS;
#endif // DEBUG_DEV_FLAGS
                    dungeon->cells[x][y].hardness = hardness[i++];
                }
            }
            else
            {
                dungeon->cells[x][y].hardness = 0;
            }
        }
    }
    free(hardness);
    return 0;
}
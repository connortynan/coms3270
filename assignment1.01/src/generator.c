#include <stdio.h>

#include "generator.h"
#include "util/bool_array.h"

/**
 * @brief generates a `packed_bool_array` matrix of dim(width, height) which represents the shape defined by `room_data`
 *
 * @var result flattened 2D boolean array representing a (width x height) matrix where 1 means the shape fills that cell,
 *  where (0,0) of the matrix is (center_x-width/2, center_y-height/2)
 */
int generator_shape_kernel(packed_bool_array *result, generator_room_data *room_data);

/**
 * @brief Takes the given `room_data` that defines a room and fills all cells with the given `value` in the `dungeon` that are in the room
 *
 * The room is assumed to fit in the dungeon, values should be validated with `generator_room_fits` before given to this function, or else UB
 *
 * @var dungeon current dungeon that the room will be written to
 * @var room_data all information about the room that defines where it will go
 * @var value `dungeon_cell` that holds type and hardness (likely `ROOM` and `0`)
 */
int generator_apply_room(dungeon_data *dungeon, generator_room_data *room_data, dungeon_cell *value);

/**
 * @brief determines if a room will fit in the current dungeon.
 * Given the rules that the bounding boxes of all rooms must not touch at all (and must have one empty cell between them)
 *
 * @var dungeon current dungeon that the room is attempting to go into
 * @var room_data all information about the room that defines where it will go
 */
int generator_room_fits(dungeon_data *dungeon, generator_room_data *room_data);

int generator_generate_dungeon(dungeon_data *dungeon, generator_parameters *params)
{
    int x, y;

    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            dungeon->cells[x][y].hardness = 0;
            dungeon->cells[x][y].type = 0;
        }
    }

    generator_room_data rounded_rect_data = {
        .center_x = 40,
        .center_y = 10,
        .corner_radius = 4,
        .width = 75,
        .height = 20,
        .shape = SHAPE_ELLIPSE,
    };
    dungeon_cell room_cell = {ROOM, 0};

    generator_apply_room(dungeon, &rounded_rect_data, &room_cell);

    dungeon->cells[40][10].type = CORRIDOR;

    return 0;
}

int generator_apply_room(dungeon_data *dungeon, generator_room_data *room_data, dungeon_cell *value)
{
    packed_bool_array kernel;

    generator_shape_kernel(&kernel, room_data);

    uint16_t x = room_data->center_x - room_data->width / 2;
    uint16_t y = room_data->center_y - room_data->height / 2;

    uint16_t dx, dy;

    for (dx = 0; dx < room_data->width; dx++)
    {
        for (dy = 0; dy < room_data->height; dy++)
        {
            if (bool_array_get(&kernel, dx + (dy * room_data->width)))
                dungeon->cells[x + dx][y + dy] = *value;
        }
    }

    return 0;
}

void _generator_shape_rounded_rect(packed_bool_array *kernel, generator_room_data *room_data)
{
    int _radius = room_data->corner_radius;
    uint16_t dx, dy;
    uint16_t w = room_data->width;
    uint16_t h = room_data->height;

    _radius = (_radius > w) ? w : _radius; // min(w, _radius)
    _radius = (_radius > h) ? h : _radius; // min(h, _radius)

    for (dy = 0; dy <= _radius; dy++)
    {
        for (dx = 0; dx <= _radius; dx++)
        {
            if (dx * dx + dy * dy > _radius * _radius) // Outside shape
            {
                bool_array_set(kernel, (_radius - dx) + (_radius - dy) * w, 0);                 // top left
                bool_array_set(kernel, (_radius - dx) + (h - _radius + dy - 1) * w, 0);         // bottom left
                bool_array_set(kernel, (w - _radius + dx - 1) + (_radius - dy) * w, 0);         // top right
                bool_array_set(kernel, (w - _radius + dx - 1) + (h - _radius + dy - 1) * w, 0); // bottom right
            }
        }
    }
}

void _generator_shape_ellipse(packed_bool_array *kernel, generator_room_data *room_data)
{
    uint16_t dx, dy;
    uint16_t w = room_data->width;
    uint16_t h = room_data->height;
    uint16_t xr = (w + 1) / 2;
    uint16_t yr = (h + 1) / 2;

    for (dy = 0; dy <= yr; dy++)
    {
        for (dx = 0; dx <= xr; dx++)
        {
            if (4 * (dx * dx * h * h + dy * dy * w * w) > (w * w * h * h)) // Outside ellipse
            {
                bool_array_set(kernel, (xr - dx) + (yr - dy) * w, 0);         // top left
                bool_array_set(kernel, (xr - dx) + (yr + dy - 1) * w, 0);     // bottom left
                bool_array_set(kernel, (xr + dx - 1) + (yr - dy) * w, 0);     // top right
                bool_array_set(kernel, (xr + dx - 1) + (yr + dy - 1) * w, 0); // bottom right
            }
        }
    }
}

int generator_shape_kernel(packed_bool_array *result, generator_room_data *room_data)
{
    // Assume everything is part of the shape, then remove things outside

    bool_array_reserve(result, room_data->width * room_data->height);
    bool_array_fill(result, 1);

    switch (room_data->shape)
    {
    case SHAPE_RECT:
        // Rectangle is already filled
        break;
    case SHAPE_ROUNDED_RECT:
        _generator_shape_rounded_rect(result, room_data);
        break;
    case SHAPE_ELLIPSE:
        _generator_shape_ellipse(result, room_data);
        break;
    }

    return 0;
}
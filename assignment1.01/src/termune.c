#include <stdio.h>

#include "dungeon.h"
#include "generator.h"

int main(int argc, char const *argv[])
{
    dungeon_data dungeon;
    generator_generate_dungeon(&dungeon, NULL);

    dungeon_display(&dungeon, 1);

    return 0;
}

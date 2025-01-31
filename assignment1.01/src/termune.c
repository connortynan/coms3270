#include <stdio.h>

#include "dungeon.h"
#include "generator.h"

int main(int argc, char const *argv[])
{
    dungeon_data main_dungeon;

    generator_generate_dungeon(&main_dungeon, NULL);

    dungeon_display(&main_dungeon, 1);

    return 0;
}

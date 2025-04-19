#include "player.hpp"

#include "game_context.hpp"

constexpr mapsize_t PLAYER_ZINDEX = 3; // Above monsters and items

Player::Player(mapsize_t x, mapsize_t y, UiManager *ui)
    : Character(x, y, '@', {COLOR_WHITE}, 10, 100, Dice{1, 1, 1}, PLAYER_ZINDEX), ui(ui) {}

bool Player::move(int, int, GameContext &g, bool)
{
    int dx, dy;
    bool force;

    while (g.running && ui->running)
    {
        ui->wait_for_input(dx, dy, force);

        // Try the move, if invalid, show message and stay in the loop
        if (Character::move(dx, dy, g, force))
        {
            ui->display_message(" "); // clear message
            return true;
        }
        // If not successful, show message and repeat
        ui->display_message("You can't move there!");
    }
    return true;
}

std::string_view Player::name() const
{
    return "Player";
}

std::string_view Player::description() const
{
    return "You are the player character.";
}
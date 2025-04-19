#pragma once

#include "character.hpp"
#include "util/colors.hpp"

class GameContext;

class Player : public Character
{
public:
    Player(mapsize_t x, mapsize_t y, UiManager *ui = nullptr);

    bool move(int, int, GameContext &g, bool) override;

    std::string_view name() const override;
    std::string_view description() const override;

    UiManager *ui = nullptr;
};
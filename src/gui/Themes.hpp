#pragma once

#include "raylib-cpp.hpp"

struct GridTheme
{
    bool showGrid;
    bool showDots;
    Color bgColour;
    Color dotColour;
    Color gridColour;
    Color cursorColour;
    Color selectedCellColour;
};

const GridTheme kGridThemeDefault =
    {
        false,    // showGrid
        true,     // showDots
        BLACK,    // bgColour
        BLUE,     // dotColour
        GRAY,     // gridColour
        PINK,     // cursorColour
        DARKBLUE, // selectedCellColour
};

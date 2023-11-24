#pragma once

#include "raylib-cpp.hpp"
#include "gui/ScrollingGrid.hpp"

#include <algorithm>
#include <map>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace std::placeholders;
using Vec2D = raylib::Vector2;
using ObjectID = uint16_t;

void emsMainLoop();

class App
{
public:
    static App *getInstance()
    {
        static App *gAppInstance = nullptr;
        if (gAppInstance == nullptr)
        {
            gAppInstance = new App;
        }
        return gAppInstance;
    }

    App(){};
    ~App(){};

    int run()
    {
        raylib::Window window(gridViewWidthCells * gridCellSize, gridViewHeightCells * gridCellSize, "Scrolling Grid");
        gridFrame = new ScrollingGrid{gridViewPosX, gridViewPosY, mCursorPosition, {(float)gridViewWidthCells, (float)gridViewHeightCells}, {(float)gridWidth, (float)gridHeight}, gridCellSize};
#if defined(PLATFORM_WEB)
        emscripten_set_main_loop(emsMainLoop, 0, 1);
#else
        SetTargetFPS(30);

        // Main loop
        while (!window.ShouldClose()) // Detect window close button or ESC key
        {
            mainLoop();
        }
#endif
        return 0;
    }

    void mainLoop()
    {
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            if (IsKeyDown(KEY_DOWN))
            {
                mZoom -= 0.01;
                mZoom = std::fmax(mZoom, 0.5);
                gridFrame->setZoom(mZoom);
            }
            if (IsKeyDown(KEY_UP))
            {
                mZoom += 0.01;
                mZoom = std::fmin(mZoom, 2);
                gridFrame->setZoom(mZoom);
            }
        }
        else
        {
            if (IsKeyPressed(KEY_UP))
                moveCursor(0, -1);
            if (IsKeyPressed(KEY_DOWN))
                moveCursor(0, 1);
            if (IsKeyPressed(KEY_LEFT))
                moveCursor(-1, 0);
            if (IsKeyPressed(KEY_RIGHT))
                moveCursor(1, 0);
            if (IsKeyPressed(KEY_ENTER))
            {
                addObject(mCursorPosition);
            }
            if (IsKeyPressed(KEY_BACKSPACE))
            {
                removeObject(mCursorPosition);
            }
        }

        // Update dirty cells
        // These should either be empty cells or the position of an object
        for (auto &cell : mDirtyGridCells)
        {
            auto it = std::find_if(mObjectPositions.begin(), mObjectPositions.end(), [&cell](std::pair<const ObjectID, Vec2D> &pair)
                                   {  return pair.second == cell; });

            if (it != mObjectPositions.end())
            {
                gridFrame->drawObject(cell, mObjects.at(it->first), cell == mCursorPosition, false);
            }
            else
            {
                gridFrame->drawEmptyCell(cell, cell == mCursorPosition);
            }
        }

        mDirtyGridCells.clear();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        gridFrame->draw();
        EndDrawing();
    }

private:
    const int gridWidth = 32;
    const int gridHeight = 16;
    const int gridCellSize = 32;
    const int gridViewPosX = 0;
    const int gridViewPosY = 0;
    const int gridViewWidthCells = 16;
    const int gridViewHeightCells = 8;

    float mZoom = 1.0f;
    Vec2D mCursorPosition = {8, 8};
    ScrollingGrid *gridFrame;
    std::vector<Vec2D> mDirtyGridCells;

    std::map<ObjectID, ObjectDrawInfo> mObjects;
    std::map<ObjectID, Vec2D> mObjectPositions;

    void moveCursor(int x, int y)
    {
        Vec2D newPos = mCursorPosition + Vec2D{(float)x, (float)y};

        // Clamp position
        if (newPos.GetX() < 0)
        {
            newPos.SetX(0);
        }
        if (newPos.GetY() < 0)
        {
            newPos.SetY(0);
        }
        if (newPos.GetX() >= gridWidth)
        {
            newPos.SetX(gridWidth - 1);
        }
        if (newPos.GetY() >= gridHeight)
        {
            newPos.SetY(gridHeight - 1);
        }

        if (newPos != mCursorPosition)
        {
            mDirtyGridCells.push_back(newPos);
            mDirtyGridCells.push_back(mCursorPosition);
            mCursorPosition = newPos;
        }
    }

    void addObject(Vec2D position)
    {
        auto it = std::find_if(mObjectPositions.begin(), mObjectPositions.end(), [&position](std::pair<const ObjectID, Vec2D> &pair)
                               {  return pair.second == position; });

        if (it == mObjectPositions.end())
        {
            ObjectID id = getNewObjectID();
            ObjectDrawInfo obj;
            obj.size = 1;
            obj.glyph = "o";
            mObjects[id] = obj;
            mObjectPositions[id] = position;
            mDirtyGridCells.push_back(position);
        }
    }

    void removeObject(Vec2D position)
    {
        auto it = std::find_if(mObjectPositions.begin(), mObjectPositions.end(), [&position](std::pair<const ObjectID, Vec2D> &pair)
                               {  return pair.second == position; });

        if (it != mObjectPositions.end())
        {
            ObjectID id = it->first;
            mObjects.erase(id);
            mObjectPositions.erase(id);
            mDirtyGridCells.push_back(position);
        }
    }

    uint16_t getNewObjectID()
    {
        static ObjectID id = 0;
        while (mObjects.count(id))
        {
            id++;
        }
        return id;
    }
};

void emsMainLoop()
{
    App::getInstance()->mainLoop();
}
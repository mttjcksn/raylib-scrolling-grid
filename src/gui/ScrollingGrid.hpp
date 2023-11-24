#include "raylib-cpp.hpp"
#include "raymath.h"
#include "Themes.hpp"
#include "Transition.hpp"
#include <algorithm>
#include <functional>

using Vec2D = raylib::Vector2;

struct ObjectDrawInfo
{
    // Default to empty cell
    int size = 0;
    std::string glyph;
    Color objectColour = WHITE;
    Color glyphColour = WHITE;
};

class ScrollingGrid
{
public:
    ScrollingGrid(int screenPosX, int screenPosY, Vec2D cursorStartPos, Vec2D viewSizeCells, Vec2D gridSizeCells, int cellSizePixels) : mViewPosX(screenPosX),
                                                                                                                  mViewPosY(screenPosY),
                                                                                                                  mGridSize(gridSizeCells),
                                                                                                                  mCellSizePixels(cellSizePixels),
                                                                                                                  mCursorPosition(cursorStartPos)
    {
        mViewSizeCellsX = viewSizeCells.GetX();
        mViewSizeCellsY = viewSizeCells.GetY();
        mGridTexture = LoadRenderTexture(gridSizeCells.GetX() * cellSizePixels, gridSizeCells.GetY() * cellSizePixels);
        mObjectSprite = LoadTexture("resources/object-sprite.png");

        drawEmptyGrid();
    }

    void drawEmptyCell(Vec2D position, bool hovered)
    {
        // Get pixel pos from grid pos
        int pixelX = position.GetX() * mCellSizePixels;
        int pixelY = position.GetY() * mCellSizePixels;

        BeginTextureMode(mGridTexture);
        DrawRectangle(pixelX, pixelY, mCellSizePixels, mCellSizePixels, mTheme.bgColour);
        DrawPixel(pixelX, pixelY, mTheme.dotColour);
        DrawPixel(pixelX, pixelY + mCellSizePixels - 1, mTheme.dotColour);
        DrawPixel(pixelX + mCellSizePixels - 1, pixelY, mTheme.dotColour);
        DrawPixel(pixelX + mCellSizePixels - 1, pixelY + mCellSizePixels - 1, mTheme.dotColour);

        // char zoomText[5] ={0};
        // snprintf(zoomText, 5, "%d,%d", x, y);
        // DrawText(zoomText, pixelX+10, pixelY+10, 10, LIGHTGRAY);

        if (hovered)
        {
            static const int cursorSize = 10;
            static const int cursorSizeHalf = 5;
            DrawRectangle(pixelX + (mCellSizePixels >> 1) - cursorSizeHalf, pixelY + (mCellSizePixels >> 1) - 1, cursorSize, 2, mTheme.cursorColour);
            DrawRectangle(pixelX + (mCellSizePixels >> 1) - 1, pixelY + (mCellSizePixels >> 1) - cursorSizeHalf, 2, cursorSize, mTheme.cursorColour);
            setCursorPosition(position);
        }
        EndTextureMode();
    }

    void drawObject(Vec2D position, ObjectDrawInfo objectInfo, bool hovered, bool selected)
    {
        int pixelX = position.GetX() * mCellSizePixels;
        int pixelY = position.GetY() * mCellSizePixels;

        BeginTextureMode(mGridTexture);
        DrawRectangle(pixelX, pixelY, mCellSizePixels, mCellSizePixels, mTheme.bgColour);
        DrawTextureRec(mObjectSprite, {0, 0, 32, 32}, {(float)pixelX, (float)pixelY}, hovered?mTheme.cursorColour:objectInfo.objectColour);
        DrawText(objectInfo.glyph.c_str(), pixelX + 10, pixelY + 4, 20, objectInfo.glyphColour);
        EndTextureMode();

        if(hovered)
        {
            setCursorPosition(position);
        }
    }

    /**
     * @brief Set the camera zoom level
     *
     * @param zoomLevel 0.5 - 2
     */
    void setZoom(float zoomLevel)
    {
        float zoomFactor = 1.0f / fabs(zoomLevel);
        if (zoomFactor != mZoom)
        {
            mZoom = zoomFactor;
            zoomCamera();
        }
    }

    void draw()
    {
        mCameraZoom.update();
        mCameraX.update();
        mCameraY.update();

        float camW = mViewSizeCellsX * mCameraZoom.value();
        float camH = mViewSizeCellsY * mCameraZoom.value();
        float camX = mCameraX.value() - (camW / 2.0f);
        float camY = mCameraY.value() - (camH / 2.0f);

        camX = std::fmin(std::fmax(camX, 0), mGridSize.GetX() - camW);
        camY = std::fmin(std::fmax(camY, 0), mGridSize.GetY() - camH);

        float GLY = mGridSize.GetY() - (mCameraZoom.value() * mViewSizeCellsY) - camY;

        DrawTexturePro(mGridTexture.texture, {camX * mCellSizePixels, GLY * mCellSizePixels, camW * mCellSizePixels, -camH * mCellSizePixels}, {(float)mViewPosX, (float)mViewPosY, (float)mViewSizeCellsX * mCellSizePixels, (float)mViewSizeCellsY * mCellSizePixels}, {0, 0}, 0, WHITE);
    }

private:
    // Pos and size of grid teture screen render
    int mViewSizeCellsX = 32;
    int mViewSizeCellsY = 16;
    int mViewPosX = 0;
    int mViewPosY = 0;

    const Vec2D mGridSize;
    const int mCellSizePixels;

    // Pan and zoom
    float mZoom = 1.0f;
    Vec2D mCursorPosition = {0, 0};
    Vec2D mCameraPosition = {8, 8};
    TransitionValue mCameraX = {8, 5};
    TransitionValue mCameraY = {8, 5};
    TransitionValue mCameraZoom = {1, 5};

    // Textures
    raylib::Texture2D mObjectSprite;
    raylib::RenderTexture2D mGridTexture;

    // Theme
    GridTheme mTheme = kGridThemeDefault;

    void drawEmptyGrid()
    {
        BeginTextureMode(mGridTexture);
        ClearBackground(BLACK);
        for (int y = 0; y < mGridSize.GetX(); y++)
        {
            for (int x = 0; x < mGridSize.GetX(); x++)
            {
                Vec2D pos = {(float)x, (float)y};
                drawEmptyCell(pos, pos == mCursorPosition);
            }
        }
        EndTextureMode();
    }

    void zoomCamera()
    {
        mCameraPosition = mCursorPosition;
        mCameraX.setTarget(mCameraPosition.GetX());
        mCameraY.setTarget(mCameraPosition.GetY());
        mCameraZoom.setTarget(mZoom);
    }

    void setCursorPosition(const Vec2D position)
    {
        mCursorPosition = position;
        panCamera();
    }

    void panCamera()
    {
        float halfViewWidthCells = mViewSizeCellsX * mZoom * 0.5;
        float halfViewHeightCells = mViewSizeCellsY * mZoom * 0.5;

        // Calculate the difference between cursor and camera positions
        Vec2D offset = mCursorPosition - mCameraPosition;

        // Check if cursor is beyond the right edge
        if (offset.GetX() >= halfViewWidthCells)
        {
            mCameraPosition.SetX(mCursorPosition.GetX() - halfViewWidthCells + 1);
        }
        // Check if cursor is beyond the left edge
        else if (offset.GetX() <= -halfViewWidthCells)
        {
            mCameraPosition.SetX(mCursorPosition.GetX() + halfViewWidthCells);
        }

        // Check if cursor is beyond the bottom edge
        if (offset.GetY() >= halfViewHeightCells)
        {
            mCameraPosition.SetY(mCursorPosition.GetY() - halfViewHeightCells + 1);
        }
        // Check if cursor is beyond the top edge
        else if (offset.GetY() <= -halfViewHeightCells)
        {
            mCameraPosition.SetY(mCursorPosition.GetY() + halfViewHeightCells);
        }

        mCameraX.setTarget(mCameraPosition.GetX());
        mCameraY.setTarget(mCameraPosition.GetY());
    }
};
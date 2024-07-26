#pragma once

#include "utils.h"


enum class PlayerMove
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct GridPosition
{
    GridPosition(int8_t initialRow, int8_t initialCol)
        :row(initialRow), col(initialCol)
    {
        assert(initialRow >= 0);
        assert(initialCol >= 0);
    }

    // struct GridPositionHash
    // {
    //     size_t operator()(const GridPosition& obj)
    //     {
    //         // Basic hash to allow for use in unordered set
    //         std::string toHash = std::to_string(obj.row) + "_" + std::to_string(obj.col);
    //         return std::hash<std::string>{}(toHash);
    //     }
    // };

    int8_t row;
    int8_t col;
};

/*
 * Class to track position of player, the "Drainer", on grid
 */
class Drainer
{
public:
    Drainer(int8_t initialRow, int8_t initialCol, int8_t gridWidth, int8_t gridHeight);
    void move(PlayerMove move); // TODO: wrap around?? 
    GridPosition position();

private:
    GridPosition m_position;
    int8_t m_gridWidth;
    int8_t m_gridHeight;
};

/*
 * Class for a single square on the grid
 */
class Cell : public IDrawable
{
public:
    bool isEmpty();
    float howFull();

    /*
     *  shrinkRate is proportion of total width/height to shrink per second
     *      0.5 means cell will be half full after a second
     *      1.0 means cell will be drained after a second
     *      2.0 means a cell will be drained in half a second
     */
    void fill(float shrinkRate);
    void drain();
    void update(float dt); // Shrink based on shrinkRate
    
    virtual void draw(SDL_Renderer* renderer) override;

private:
    float m_shrinkRate{ 0.0f };
    float m_proportionFilled{ 0.0f }; 
};

class Grid : public IDrawable
{
public:
    Grid(uint8_t width, uint8_t height, Drainer& drainer);

    bool drainCell(uint8_t row, uint8_t col, float& ret);
    void fillCell(float shrinkRate); // Grid's responsibility to choose a cell that has not been filled yet
    void update(float dt);
    
    virtual void draw(SDL_Renderer* renderer) override; // Grid's responsibility to draw everything

private:
    std::vector<GridPosition> m_availableCells; // TODO: Confirm what the right data structure is for this?
        // I want fast erasing and insertion + indexing by number?? Becuase I want to select randomly easily from it
    std::vector<std::vector<Cell>> m_grid;
    Drainer& m_drainer;
};


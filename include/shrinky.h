#pragma once

#include "utils.h"

static constexpr const uint32_t WINDOW_HEIGHT = 720;
static constexpr const uint32_t WINDOW_WIDTH = 1280;

static struct GameConfigurations {
    // Grid dimensions configurations
    uint32_t gridHeight_cells = 4;
    uint32_t gridWidth_cells = 4;
    uint32_t gridHeight_px = 0.8 * WINDOW_HEIGHT;
    uint32_t gridWidth_px = gridHeight_px;
    uint32_t gridOriginX_px = (WINDOW_WIDTH / 2) - (gridWidth_px / 2);
    uint32_t gridOriginY_px = (WINDOW_HEIGHT / 2) - (gridHeight_px / 2);
    uint32_t cellWidth_px = gridWidth_px / gridWidth_cells;
    uint32_t cellHeight_px = gridHeight_px / gridHeight_cells;

    float fillInterval_ms = 1000;
    float fillIntervalDelta_ms = 5;
    float fillIntervalDeltaPeriod_ms = 1000; // Every second, fill interval goes down
    float fillIntervalMin_ms = 500;

    float drainRate = 0.33;
    float drainRateDelta = 0.01;
    float drainRateDeltaPeriod_ms = 1000;
    float drainRateMax = 1.0;
    float drainRateVariation = 0.1;
} config;

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

    bool operator==(const GridPosition& rhs)
    {
        return (row == rhs.row) && (col == rhs.col);
    }

    struct GridPositionHash
    {
        size_t operator()(const GridPosition& obj)
        {
            // Basic hash to allow for use in unordered set
            std::string toHash = std::to_string(obj.row) + "_" + std::to_string(obj.col);
            return std::hash<std::string>{}(toHash);
        }
    };

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
class Cell
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
    uint8_t update(float dt);
    
    virtual void draw(SDL_Renderer* renderer) override; // Grid's responsibility to draw everything

private:
    std::vector<GridPosition> m_availableCells;
    std::vector<std::vector<Cell>> m_grid;
    std::vector<std::vector<SDL_Rect>> m_drawGrid;
    Drainer& m_drainer;

    /* TODO: I will add this feature once the game logic is done. Strikes will still appear in a bar at the top, just not directly on the cells for right now
     * Cells are added to the strikeCells when:
     *     - A player clicks an empty cells
     *     - A player lets a cell drain completely without draining themselves
     * 
     * The missed cell will be added to the strike cells in order for draw() to know where to draw red X's
     * In order for the X to not blip in and out immediately, we will draw it several draws in a row.
     * draw() will increment the value associated with the strike cell to a certain threshold e.g. 10 
     * The cell then becomes available in m_availableCells.
     * 
     * I don't believe there will be too much worry in running out of available cells (all cells full or containing X) 
     * because I think the X will be there very shortly and compared to the slower spawn rate of full cells, we will be alright,
     * but it is something to keep in mind
     */
    // std::unordered_map<GridPosition, int, GridPosition::GridPositionHash> m_strikeCells;

};


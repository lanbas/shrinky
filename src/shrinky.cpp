#include "shrinky.h"


////////////////////////////////
// Drainer
////////////////////////////////
Drainer::Drainer(int8_t initialRow, int8_t initialCol, int8_t gridWidth, int8_t gridHeight)
    : m_position(initialRow, initialCol), m_gridWidth(gridWidth), m_gridHeight(gridHeight)
{
    assert(initialRow >= 0);
    assert(initialCol >= 0);
    assert(gridWidth > 0);
    assert(gridHeight > 0);
}

void Drainer::move(PlayerMove move)
{
    switch (move)
    {
        case PlayerMove::UP:
            m_position.row = std::max(0, m_position.row - 1); // Confirm that this does not roll over to max
        case PlayerMove::DOWN:
            m_position.row = std::min((int)m_gridHeight, m_position.row + 1);
        case PlayerMove::LEFT:
            m_position.col = std::max(0, m_position.col - 1);
        case PlayerMove::RIGHT:
            m_position.col = std::min((int)m_gridWidth, m_position.col + 1);
    }
}

GridPosition Drainer::position()
{
    return m_position;
}


////////////////////////////////
// Cell : IDrawable
////////////////////////////////
bool Cell::isEmpty()
{
    float eps = 0.0001;
    return m_proportionFilled < eps;
}

float Cell::howFull()
{
    return m_proportionFilled;
}

void Cell::fill(float shrinkRate)
{
    m_shrinkRate = shrinkRate;
    m_proportionFilled = 1.0f;
}

void Cell::drain()
{
    m_shrinkRate = 0.0f;
    m_proportionFilled = 0.0f;
}

void Cell::update(float dt)
{
    m_proportionFilled = std::max(0.0f, m_proportionFilled - (dt * m_shrinkRate));
}

////////////////////////////////
// Grid : IDrawable
////////////////////////////////
Grid::Grid(uint8_t width, uint8_t height, Drainer& drainer)
    : m_drainer(drainer)
{
    m_grid = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            m_availableCells.push_back(GridPosition(i, j));
        }
    }
}

bool Grid::drainCell(uint8_t row, uint8_t col, float& ret)
{
    // Return whether or not the cell was empty and how full it was if it was not empty
    if (m_grid[row][col].isEmpty())
    {
        ret = 0.0f;
        return false;
    }
   
    // else
    ret = m_grid[row][col].howFull();
    m_grid[row][col].drain();
    // add to availableCells
    return true;
}

void Grid::fillCell(float shrinkRate)
{
    // Randomly select cell for filling
    // remove from availableCells
}

void Grid::update(float dt)
{
    // call update on all cells
    for (size_t i = 0; i < m_grid.size(); i++)
    {
        for (size_t j = 0; j < m_grid[i].size(); j++)
        {
            m_grid[i][j].update(dt);
        }
    }
}

void Grid::draw(SDL_Renderer* renderer)
{
    // Draw all cells based on their fullness
    // Current drainer position cell should be outlined in color
}
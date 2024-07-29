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
    m_drawGrid = std::vector<std::vector<SDL_Rect>>(height, std::vector<SDL_Rect>(width));

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Set available cells
            m_availableCells.push_back(GridPosition(i, j));

            // Intialize known pixel positions of cells
            m_drawGrid[i][j] = SDL_Rect();
            m_drawGrid[i][j].h = config.cellHeight_px;
            m_drawGrid[i][j].w = config.cellWidth_px;
            m_drawGrid[i][j].x = config.gridOriginX_px + (j * config.cellWidth_px);
            m_drawGrid[i][j].y = config.gridOriginY_px + (i * config.cellHeight_px);
        }
    }
}

bool Grid::drainCell(uint8_t row, uint8_t col, float& ret)
{
    // Return whether or not the cell was empty and how full it was if it was not empty
    if (m_grid[row][col].isEmpty())
    {
        ret = 0.0f;
        // TODO: Add to m_strikeCells
        return false;
    }
   
    // else
    ret = m_grid[row][col].howFull(); // For scoring
    m_grid[row][col].drain();
    m_availableCells.push_back(GridPosition(row, col));
    return true;
}

void Grid::fillCell(float shrinkRate)
{
    // Randomly select cell for filling
    size_t numAvailable = m_availableCells.size();
    int idx = rand() % (numAvailable - 1);
    GridPosition chosen = m_availableCells[idx];

    // Fill cell in grid
    m_grid[chosen.row][chosen.col].fill(shrinkRate);

    // remove from availableCells
    m_availableCells[idx] = m_availableCells.back();
    m_availableCells.back() = chosen;
    m_availableCells.pop_back();
}

uint8_t Grid::update(float dt)
{
    uint8_t missedCells = 0;

    // call update on all cells
    for (size_t i = 0; i < m_grid.size(); i++)
    {
        for (size_t j = 0; j < m_grid[i].size(); j++)
        {
            bool wasFull = !m_grid[i][j].isEmpty();
            m_grid[i][j].update(dt);

            // If cell was not empty and now is, we have to add it to available cells
            if (wasFull && m_grid[i][j].isEmpty())
            {
                missedCells++; // TODO: Add to m_strikeCells
                m_availableCells.push_back(GridPosition(i, j));
            }
        }
    }

    return missedCells;
}

void Grid::draw(SDL_Renderer* renderer)
{
    // Draw all cells based on their fullness
    // Current drainer position cell should be outlined in color

    // For each cell
    uint8_t i = 0;
    for (uint32_t y = 0; y < config.gridHeight_px; y += config.cellHeight_px)
    {
        uint8_t j = 0;
        for (uint32_t x = 0; x < config.gridWidth_px; x += config.cellWidth_px)
        {
            // Set color if the drainer is hovering over this cell
            if (GridPosition(i, j) == m_drainer.position())
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 200); // Blue
            else
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White

            // Draw border of cell
            SDL_RenderDrawRect(renderer, &m_drawGrid[i][j]);

            // Draw filled inside of cell if not empty
            if (!m_grid[i][j].isEmpty())
            {
                SDL_Rect fillRect;
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White

                // Draw filled rectangle based on how much of cell is full 
                fillRect.w = config.cellWidth_px * m_grid[i][j].howFull();
                fillRect.h = 
            }



            j++;
        }
        i++;
    }

}
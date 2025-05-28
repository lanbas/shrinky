#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "utils.h"
#include "shrinky.h"

using namespace std::chrono_literals;

void drawGameOver(TTF_Font* font, SDL_Renderer* renderer)
{
	SDL_Rect rect{};
	SDL_Surface* surface = TTF_RenderText_Solid(font, "GAME OVER", {0xFF, 0xFF, 0xFF, 0xFF});
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int width, height;
	SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
	rect.x = (WINDOW_WIDTH / 2) - width / 2; 
	rect.y = 10; 
	rect.w = width;
	rect.h = height;

	SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

void drawStrikes(uint8_t numStrikes, TTF_Font* font, SDL_Renderer* renderer, uint32_t topLeftX, uint32_t topLeftY)
{
	std::string strikeStr = "";
	for (int i = 0; i < numStrikes; i++)
	{
		strikeStr += "X";
	}
	
	SDL_Rect rect{};
	SDL_Surface* surface = TTF_RenderText_Solid(font, strikeStr.c_str(), {0xFF, 0xFF, 0xFF, 0xFF});
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int width, height;
	SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
	rect.x = topLeftX; 
	rect.y = topLeftY; 
	rect.w = width;
	rect.h = height;

	SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

int main(int argc, char** argv)
{
    // Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

	SDL_Window* window = SDL_CreateWindow("Shrinky", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0); 

    // Initialize the font
	TTF_Font* gameFont = TTF_OpenFont("fonts/DejaVuSansMono.ttf", 32);
	Score score(gameFont, {20, 20});

	// Initialize sound
	// Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	// Mix_Chunk* wallHitSound = Mix_LoadWAV("audio/pongWallHit.wav");
	// Mix_Chunk* paddleHitSound = Mix_LoadWAV("audio/pongPaddleHit.wav");

    // Create Drainer and Grid
	Drainer drainer(0, 0, config.gridWidth_cells, config.gridHeight_cells);
    Grid grid(config.gridWidth_cells, config.gridHeight_cells, drainer);
	Score totalScore(gameFont, {10, 10});

	auto globalStartTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	auto totalTimeElapsed_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - globalStartTime).count();
	float lastFillTime_ms = 0.0f;
	float lastFillPeriodUpdate_ms = 0.0f;
	float lastDrainRatePeriodUpdate_ms = 0.0f;
	uint8_t strikes = 0;
	bool gameOver = false;

    // Game logic
    {
        bool running = true;
		float dt = 0.0f;

        // Continue looping and processing events until user exits
		while (running)
		{
			auto startTime = std::chrono::high_resolution_clock::now();
			SDL_Event event;
			float score = 0.0f;

			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				
				// Check which buttons are down/up
				else if (event.type == SDL_KEYDOWN)
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							running = false;
							break;
						case SDLK_SPACE:
						{
							GridPosition drainerPosition = drainer.position(); 
							bool wasFull = grid.drainCell(drainerPosition.row, drainerPosition.col, score);
							totalScore.addScore((int)score);
							if (!wasFull)
							{
								strikes++;
							}
							break;
						}
						case SDLK_DOWN:
							drainer.move(PlayerMove::DOWN);
							break;
						case SDLK_UP:
							drainer.move(PlayerMove::UP);
							break;
						case SDLK_LEFT:
							drainer.move(PlayerMove::LEFT);
							break;
						case SDLK_RIGHT:
							drainer.move(PlayerMove::RIGHT);
							break;
					}
				}
			}

			// Update fill frequency and rate
			if (totalTimeElapsed_ms - lastFillPeriodUpdate_ms > config.fillIntervalDeltaPeriod_ms)
			{
				lastFillPeriodUpdate_ms = totalTimeElapsed_ms;
				config.fillInterval_ms = std::max(config.fillIntervalMin_ms, config.fillInterval_ms - config.fillIntervalDelta_ms);
			}

			if (totalTimeElapsed_ms - lastDrainRatePeriodUpdate_ms > config.drainRateDeltaPeriod_ms)
			{
				lastDrainRatePeriodUpdate_ms = totalTimeElapsed_ms;
				config.drainRate = std::min(config.drainRateMax, config.drainRate + config.drainRateDelta);
			}

			// If enough time has passed, fill a cell on the grid
			if (totalTimeElapsed_ms - lastFillTime_ms > config.fillInterval_ms)
			{
				lastFillTime_ms = totalTimeElapsed_ms;
				grid.fillCell(config.drainRate);
			}

            // Clear the window to black
			SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
			SDL_RenderClear(renderer);

			// Update and draw game
			strikes += grid.update(dt);

			// Draw grid()
			grid.draw(renderer);
			totalScore.draw(renderer);
			drawStrikes(std::min(3, (int)strikes), gameFont, renderer, 0.9 * WINDOW_WIDTH, 10);

			if (strikes >= 3)
			{
				running = false;
				gameOver = true;
				drawGameOver(gameFont, renderer);
			}

			// Present the backbuffer
			SDL_RenderPresent(renderer);

            // Calculate frame time
			auto stopTime = std::chrono::high_resolution_clock::now();
			dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
			totalTimeElapsed_ms += dt;
        }

		if (gameOver)
			std::this_thread::sleep_for(5s);
    }

    return 0;
}
// To spawn:
    // Add object to shared obsctacle list at whatever the 

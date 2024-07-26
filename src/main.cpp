#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "utils.h"
#include "jumper.h"

using namespace std::chrono_literals;

bool running = false;
std::mutex obstacleLock;
std::condition_variable cv;

void drawGameOver(TTF_Font* font, SDL_Renderer* renderer)
{
	SDL_Rect rect{};
	SDL_Surface* surface = TTF_RenderText_Solid(font, "GAME OVER", {0xFF, 0xFF, 0xFF, 0xFF});
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int width, height;
	SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
	rect.x = kWindowWidth / 2; 
	rect.y = kWindowHeight / 2; 
	rect.w = width;
	rect.h = height;

	SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

void printObstacle(Obstacle& o)
{
	std::cout << "============\n";
	std::cout << "Position: (" << o.position().x << ", " <<  o.position().y << ")\n";
	std::cout << "============\n";
}

void printObstacleList(std::deque<Obstacle>& obstacleList)
{
	for (auto& o : obstacleList)
	{
		printObstacle(o);
	}
}

float randomFloatInRange(float min, float max)
{
	return min + static_cast<float>(rand()) / ( static_cast<float>(RAND_MAX / (max - min)) );
}

void spawnObstacles(std::deque<Obstacle>& obstacleList, std::chrono::_V2::system_clock::time_point startTime)
{
	float obstacleVelocityX = kInitialObstacleVelocity;
	float obstacleWidthFactorMin = kInitialObstacleWidthFactorMin;
	float obstacleWidthFactorMax = kInitialObstacleWidthFactorMax;
	uint32_t obstacleSpawnInterval_ms = kInitialObstacleSpawnInterval_ms;

	while (running)
	{
		// Determine total time elapsed
		auto now = std::chrono::high_resolution_clock::now();
		auto totalTimeElapsed_s = std::chrono::duration<float, std::chrono::seconds::period>(now - startTime).count();

		// Velocity and object width distribution factor increase until steady state is reached
		if (totalTimeElapsed_s < kSteadyStateTimeElapsed_s)
		{
			obstacleVelocityX += kObstacleVelocityIncrement;
			obstacleWidthFactorMin += kWidthFactorIncrement;
			obstacleWidthFactorMax += kWidthFactorIncrement;
			obstacleSpawnInterval_ms += kObstacleSpawnIntervalIncrement_ms;
		}

		// Determine obstacle width
		float obstacleWidthFactor = randomFloatInRange(obstacleWidthFactorMin, obstacleWidthFactorMax);
		float obstacleWidth = obstacleWidthFactor * kObstacleUnitWidth_px;
		
		// Initialize as jump obstacle, kDuckObstacleProportion chance to change to duck obstacle
		float obstaclePositionY;
		float obstacleHeight;
		if (randomFloatInRange(0.0f, 1.0f) < kDuckObstacleProportion) // Duck obstacle
		{
			obstaclePositionY = 0;
			obstacleHeight = randomFloatInRange(kJumperHomeY, kJumperHomeY + (kJumperHeight * (1 - (1.0 / kDuckHeightDivisor))) - 2);
		}
		else // Jump obstacle
		{
			// Obstacles can be up to 1.5 * the height of the jumper
			obstaclePositionY = randomFloatInRange(kJumperHomeY - (kJumperHeight / 2), kJumperHomeY);
			obstacleHeight = kFloorHeight - obstaclePositionY;
		}

		Obstacle obstacle(obstaclePositionY, {obstacleVelocityX, 0}, obstacleHeight, obstacleWidth);

		std::unique_lock<std::mutex> g(obstacleLock);
		obstacleList.push_back(obstacle);
		// std::cout << "Adding obstacle\n";
		float spawnDeviation_ms = randomFloatInRange(-1 * kObstacleSpawnIntervalDeviation_ms, kObstacleSpawnIntervalDeviation_ms);
		cv.wait_for(g, std::chrono::milliseconds(obstacleSpawnInterval_ms + (int)spawnDeviation_ms));

	}
}

int main(int argc, char** argv)
{
    // Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

	SDL_Window* window = SDL_CreateWindow("Jumper", 0, 0, kWindowWidth, kWindowHeight, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0); 

    // Initialize the font
	TTF_Font* gameFont = TTF_OpenFont("fonts/DejaVuSansMono.ttf", 32);
	Score score(gameFont, {20, 20});

	// Initialize sound
	// Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	// Mix_Chunk* wallHitSound = Mix_LoadWAV("audio/pongWallHit.wav");
	// Mix_Chunk* paddleHitSound = Mix_LoadWAV("audio/pongPaddleHit.wav");

    // Create Jumper (starts at configured defaults)
    Jumper jumper;
	auto globalStartTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	auto totalTimeElapsed_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - globalStartTime).count();

    // Game logic
    {
        running = true;
		
		float dt = 0.0f;
		std::deque<Obstacle> obstacleList;
		std::thread t(&spawnObstacles, std::ref(obstacleList), globalStartTime);

        // Continue looping and processing events until user exits
		while (running)
		{
			auto startTime = std::chrono::high_resolution_clock::now();
			SDL_Event event;
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
							if (jumper.state() == JumperState::RUNNING)
                            {
                                jumper.jump();
                            }
							break;
						case SDLK_s:
							if (jumper.state() == JumperState::RUNNING)
                            {
                                jumper.duck();
                            }
							break;
						case SDLK_DOWN:
							if (jumper.state() == JumperState::RUNNING)
                            {
                                jumper.duck();
                            }
							break;
					}
				}
				else if (event.type == SDL_KEYUP)
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_s:
                            if (jumper.state() == JumperState::DUCKING)
							    jumper.reset();
							break;
						case SDLK_DOWN:
							if (jumper.state() == JumperState::DUCKING)
							    jumper.reset();
							break;
                    }
                }
			}

            // Clear the window to black
			SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
			SDL_RenderClear(renderer);

            // Set the draw color to be white
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

			// Update objects and score
			score.setScore(totalTimeElapsed_ms / 100);

			jumper.updateMotion(dt);

			{
				// Remove front if necessary, then update + draw remaining
				std::unique_lock<std::mutex> g(obstacleLock);
				if (!obstacleList.empty() && obstacleList.front().right() <= 0)
				{
					// Timing is such that only one obstacle (the front one) will need to be deleted on any given run of this loop
					obstacleList.pop_front();
				}

				for (auto& obstacle : obstacleList)
				{
					// Nice to only iterate once, but idk if there is a benefit to drawing all at once instead of sneaking in an updateMotion
					obstacle.updateMotion(dt);
					obstacle.draw(renderer);
				}
			}

			jumper.draw(renderer);
			score.draw(renderer);

			// If jumper is colliding with object, game is over
			if (jumper.isColliding({obstacleList.begin(), obstacleList.end()}))
			{
				running = false;
			}

			// Present the backbuffer
			SDL_RenderPresent(renderer);

            // Calculate frame time
			auto stopTime = std::chrono::high_resolution_clock::now();
			dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
			totalTimeElapsed_ms += dt;
        }

		cv.notify_one();
		t.join();

		drawGameOver(gameFont, renderer);
		SDL_RenderPresent(renderer);
		std::this_thread::sleep_for(10s);

    }

    return 0;
}
// To spawn:
    // Add object to shared obsctacle list at whatever the 

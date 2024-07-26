#pragma once

#include "base.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

// struct Collision

class IDrawable
{
public:
    virtual void draw(SDL_Renderer* renderer) = 0;
};

class Vector2D
{
public:
    Vector2D()
		: x(0.0f), y(0.0f)
	{}

	Vector2D(float x, float y)
		: x(x), y(y)
	{}

	Vector2D operator+(const Vector2D& rhs)
	{
		return Vector2D(x + rhs.x, y + rhs.y);
	}

	Vector2D& operator+=(const Vector2D& rhs)
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}

	Vector2D operator*(float rhs)
	{
		return Vector2D(x * rhs, y * rhs);
	}

    Vector2D operator/(float rhs)
	{
        assert(rhs != 0);
		return Vector2D(x / rhs, y / rhs);
	}

	float x, y;
};

class Object2D
{
public:
    Object2D(const Vector2D& initialPosition, const Vector2D& initialVelocity, const Vector2D& initialAcceleration, const uint32_t height, const uint32_t width);
    Vector2D position();
    Vector2D velocity();
    Vector2D acceleration();
    uint32_t height();
    uint32_t width();
    uint32_t left();
    uint32_t right();
    uint32_t top();
    uint32_t bottom();

    // Updates position and velocity 
    virtual void updateMotion(float dt);

    // virtual void onCollision() = 0;
    bool isColliding(std::vector<Object2D> objectList);

protected:
    Vector2D m_position;
    Vector2D m_velocity;
    Vector2D m_acceleration;
    uint32_t m_height;
    uint32_t m_width;
};

struct Score : public IDrawable
{
public:
    Score(TTF_Font* font, Vector2D position, int64_t initialScore = 0);
    void setScore(int64_t value);
    int64_t getScore();
    void addScore(int64_t value);
    void subtractScore(int64_t value);

    // TODO operator overloads

    // fetchAdd type beat? 
    virtual void draw(SDL_Renderer* renderer) override;
    
private:
    int64_t m_score;
    TTF_Font* m_font;
	SDL_Surface* m_surface{};
	SDL_Texture* m_texture{};
	SDL_Rect m_rect{};
    bool m_initialized{ false };
};

static bool areColliding(Object2D& obj1, Object2D& obj2)
{
    if (obj1.left() >= obj2.right())
		return false;

	if (obj1.right() <= obj2.left())
		return false;

	if (obj1.top() >= obj2.bottom())
		return false;

	if (obj1.bottom() <= obj2.top())
		return false;

    return true;
}

// onTick()? global function that ups global settings e.g. obstacle velocity, every N seconds


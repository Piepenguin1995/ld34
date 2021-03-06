#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <utility>

#include "constants.hpp"
#include "util.hpp"

// Player is a cube (higher polygon?) with coloured entries on
// a select number of sides. Player can be rotated by 90 degrees
// to rotate colours, must do so to collide correct colours with
// incoming bullets from the 4 cardinal directions (or more if
// using higher side counts).
class Player : public sf::Transformable, public sf::Drawable
{
private:

	float mDim;
	// Black slot means empty, any other colour is a collider
	// Vector will probably just be length 4 for LD
	// Colours are in HSV, and converted to RGB when drawn
	std::vector<std::pair<sf::Vector3f, sf::RectangleShape>> mSlots;
	sf::RectangleShape mBody;

	const float mRotateDuration;
	float mRotateInterp;

	float getAngle()
	{
		// Temporarily use a linear transition
		// TODO: Make transition cubic/quadratic
		// Want interpolating polynomial of smallest degree with
		// f(0) = 0; f'(0) = 0; f(1) = 1; f'(1) = 0
		// which solving gives
		// f(x) = x^2(x-2)^2
		float& x = mRotateInterp;
		return 90.0f*fma(mRotating, x*x*(x-2)*(x-2), mDir);
		// return 90.0f * mDir + 90.0f * x*x*(x-2)*(x-2) * mRotating;
		// return mDir * 90.0f + 90.0f * mRotateInterp * mRotating;
	}

	int mRotating;
	// 0, 1, 2, 3 are N, E, S, W, respectively
	int mDir;

public:

	unsigned long score;
	unsigned long lives;

	// Add a new randomly coloured slot
	// Slots are 1/3 size of the total rectangle,
	// so
	// |-o-|
	// o x o
	// |-o-|
	void addSlot()
	{
		// Up to a max of 4 for now
		if(mSlots.size() < 4)
		{
			// New slot is hue shifted from previous to
			// prevent repetition by rand value in (30, 90)
			float prevHue = (mSlots.size() > 0 ? mSlots.back().first.x : ld::rand(0, 360.0f));
			sf::Vector3f col(
				fmod(prevHue + ld::rand(60.0f, 90.0f), 360.0f),
				ld::saturation,
				ld::value);
			// Selecting same colour twice?
			sf::RectangleShape slot(sf::Vector2f(mDim / 3.0f, mDim / 3.0f));
			slot.setFillColor(ld::hsvToRgb(col));
			slot.setOutlineThickness(0.0f);
			// Set origin to centre of slot
			slot.setOrigin(mDim / 6.0f, mDim / 6.0f);
			switch(mSlots.size())
			{
			case 0: // North
				slot.setPosition(0.0f, -mDim/3.0f); break;
			case 1: // East
				slot.setPosition(mDim/3.0f, 0.0f); break;
			case 2: // South
				slot.setPosition(0.0f, mDim/3.0f); break;
			case 3: // West
				slot.setPosition(-mDim/3.0f, 0.0f); break;
			default:
				slot.setPosition(0.0f, 0.0f); break;
			}
			mSlots.push_back(std::make_pair(col, slot));
		}
	}

	Player(float dim) :
		mDim(dim), mBody(sf::Vector2f(dim, dim)),
		mRotateDuration(0.05),
		mRotateInterp(0.0),
		mRotating(false),
		mDir(0),
		score(0),
		lives(3)
	{
		// Create main body rectangle
		mBody.setFillColor(ld::playerCol);
		mBody.setOutlineThickness(0.0f);
		mBody.setOrigin(dim / 2.0f, dim / 2.0f);

		// addSlot();
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		target.draw(mBody, states);
		for(auto& slot : mSlots) target.draw(slot.second, states);
	}

	void update(float dt)
	{
		if(mRotating) mRotateInterp += dt / mRotateDuration;
		if(mRotateInterp > 1.0f)
		{
			mRotateInterp = 0.0f;
			mDir = mDir + mRotating;
			if(mDir >= 4) mDir -= 4;
			else if(mDir < 0) mDir += 4;
			mRotating = 0;
		}
		setRotation(getAngle());
	}

	// Rotate the player by 90 degrres with a smooth transition
	// 1 = right, -1 = left
	void rotate(int dir)
	{
		if(mRotating == 0)
		{
			mRotating = dir;
			mRotateInterp = 0.0f;
		}
	}

	// Choose a random colour
	sf::Vector3f sample() const
	{
		return mSlots[static_cast<int>(ld::rand(0, mSlots.size()))].first;
	}

	// Get the colour associated with a specific side,
	// after the current rotation. Rotation ccw by one step
	// is equivalent to a left shift by 1, and cw a right shift.
	// Dir should be in the usual Player format, i.e. 0,1,2,3
	sf::Color colOnSide(float dir) const
	{
		// First slot is north, others proceed N,E,S,W
		int slot = static_cast<int>(dir - mDir);
		if(slot < 0) slot += 4;
		if(slot >= mSlots.size())
			return sf::Color(0,0,0);
		else
			return ld::hsvToRgb(mSlots[slot].first);
	}

	// Get bounding rectangle, ignoring rotations
	sf::FloatRect bounds() const
	{
		sf::Transform trans;
		trans.translate(getPosition()-mBody.getOrigin());
		return trans.transformRect(mBody.getLocalBounds());
	}

	// Cycle hue of all colours by a small factor
	// Does so instantly, so for a smooth transition pass
	// a small df over and over
	void cycleHue(float factor)
	{
		for(auto& slot : mSlots)
		{
			// sf::Vector3f newCol(slot.first);
			// newCol.x = ;
			slot.first.x = fmod(slot.first.x + factor, 360.0f);
			slot.second.setFillColor(ld::hsvToRgb(slot.first));
		}
	}

	// Change the alpha value of a slot
	// Alpha is in [0,1)
	void setAlpha(size_t slot, float a)
	{
		if(slot >= mSlots.size()) return;
		sf::Color c = mSlots[slot].second.getFillColor();
		if(a > 1.0) a = 1.0f;
		if(a < 0.0) a = 0.0f;
		c.a = 255 * a;
		mSlots[slot].second.setFillColor(c);
	}

	size_t numSlots() const
	{
		return mSlots.size();
	}

	sf::Color getCol(size_t slot) const
	{
		if(mSlots.size() == 0) return sf::Color::Black;
		if(slot > mSlots.size()) slot = mSlots.size()-1;
		return ld::hsvToRgb(mSlots[slot].first);
	}

	int getDir() const
	{
		return mDir;
	}
};

#endif /* PLAYER_HPP */

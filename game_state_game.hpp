#ifndef GAME_STATE_GAME_HPP
#define GAME_STATE_GAME_HPP

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>

#include "game_state.hpp"
#include "player.hpp"
#include "projectile.hpp"
#include "constants.hpp"
#include "text.hpp"
#include "particles.hpp"

class GameStateGame : public GameState
{
private:

	enum class SubState {PLAY, PAUSE, TRANSITIONING, TRANSITIONED, DYING};

	Player mPlayer;
	float mNextGen;
	float mT;
	float mT2;
	float mTrailEmissionInterval;
	std::vector<Projectile> mProjectiles;
	ParticleTracker mParticles;
	SubState mSubstate;
	// SubState mPrevSubstate; // Used for resuming

	sf::SoundBuffer mHitSoundBufGood;
	sf::SoundBuffer mHitSoundBufBad;
	sf::SoundBuffer mDeadSoundBuf;
	sf::Sound mHitSound;
	sf::Sound mDeadSound;

	Text mTextScore;
	Text mTextPause;
	Text mTextLives;

	float mDuration;

	float mTransitionTimer;
	float mTransitionLength;
	int mCurrentLevel;

	// Projectile angle to player side is
	// north = (0,90)    = 0 => south = 2
	// west =  (90,180)  = 1 => east  = 1
	// south = (180,270) = 2 => north = 0
	// east =  (270,360) = 3 => west  = 3
	float dirToFacing(float dir)
	{
		const static int map[4] = { 2, 1, 0, 3 };
		return map[static_cast<int>(dir / 90.0f)];
	}

	float getLevel() const
	{
		if(mDuration < ld::levelTimers[0])
			return 0;
		else if(mDuration < ld::levelTimers[1])
			return 1;
		else if(mDuration < ld::levelTimers[2])
			return 2;
		else if(mDuration < ld::levelTimers[3])
			return 3;
		else if(mDuration < ld::levelTimers[4])
			return 4;
		else if(mDuration < ld::levelTimers[5])
			return 5;
		else
			return 6;
	}

	float getGenerationInterval(int level) const
	{
		if(mSubstate == SubState::TRANSITIONING && mCurrentLevel % 2 == 0) return 0.25f;
		static const float difficultyMap[] = {
			1.6, 1.4, 1.2, 1.0, 0.8, 0.65, 0.5
		};
		// Translates to BPM for music, have two beats per note?
		return difficultyMap[level];
	}

public:

	GameStateGame(std::shared_ptr<GameState>& state,
		std::shared_ptr<GameState>& prevState) :
		GameState(state, prevState),
		mPlayer(1.0f),
		mNextGen(2.0f),
		mT(0.0f),
		mT2(0.0f),
		mTrailEmissionInterval(0.1f),
		mSubstate(SubState::TRANSITIONING),
		mTextScore("0"),
		mTextPause("PAUSED"),
		mTextLives("^"),
		mDuration(0.0f),
		mTransitionTimer(0.0),
		mTransitionLength(2.0f),
		mCurrentLevel(-1)
	{
		mPlayer.setPosition(ld::gameDim/2.0f, ld::gameDim/2.0f);

		mTextScore.setPosition(ld::gameDim*5.0f/6.0f, ld::gameDim/15.0f);
		mTextScore.setScale(0.1f, 0.1f);

		mTextLives.setString(std::string(mPlayer.lives, '^'));
		mTextLives.setPosition(ld::gameDim*5.0f/6.0f, 0.0f);
		mTextLives.setScale(0.1f, 0.1f);

		mTextPause.setPosition(ld::gameDim/2.0f, ld::gameDim/2.0f);
		mTextPause.setOrigin(6 * 5 * 0.5f, 1 * 6 * 0.5f);
		mTextPause.setScale(0.2f, 0.2f);

		// Load sounds
		mHitSoundBufGood.loadFromFile(ld::hitSoundGoodPath);
		mHitSoundBufBad.loadFromFile(ld::hitSoundBadPath);
		mDeadSoundBuf.loadFromFile(ld::deadSoundPath);

		// Instantly triggers level -1 -> 0 which fades in a new slot
		// Projectiles are not fired until after the projectile is
		// created
		mTransitionTimer = 0;
		mT = -mTransitionLength;
		mPlayer.addSlot();
		mPlayer.setAlpha(mPlayer.numSlots()-1, 0);

		mTextLives.setColor(ld::hsvToRgb(mPlayer.sample()));
		mTextLives.setAlpha(0);
	}

	virtual void handleEvent(const sf::Event& event);
	virtual void handleInput(float dt);
	virtual void update(float dt);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		if(mSubstate != SubState::DYING)
		{
			target.draw(mPlayer, states);
		}
		// Helps if I draw the damn things...
		for(auto& projectile : mProjectiles) target.draw(projectile, states);
		target.draw(mParticles, states);

		target.draw(mTextScore, states);
		target.draw(mTextLives, states);

		if(mSubstate == SubState::PAUSE)
		{
			target.draw(mTextPause, states);
		}
	}
};

#endif /* GAME_STATE_GAME_HPP */

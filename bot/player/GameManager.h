#pragma once

class GameManager {
public:
	virtual ~GameManager() {
	}

	/**
	 * Callback for when a round starts
	 */
	virtual void startRound() = 0;

	/**
	 * Callback for when a round ends.
	 */
	virtual void endRound() = 0;
};

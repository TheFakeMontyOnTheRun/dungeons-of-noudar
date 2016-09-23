package br.odb.noudar;

import java.io.Serializable;

import br.odb.droidlib.Updatable;

/**
 * Created by monty on 23/08/16.
 */
class Splat implements Updatable, Serializable {
	private static final int TOTAL_ANIMATION_TIME = 1500;
	private static final int NUMBER_OF_FRAMES = 3;
	private long showSplatTime = 0;

	public Splat() {
		startSplatAnimation();
	}

	public synchronized void update(long ms) {
		showSplatTime -= ms;
	}

	private void startSplatAnimation() {
		showSplatTime = TOTAL_ANIMATION_TIME;
	}

	public int getSplatFrame() {
		if (showSplatTime > 0) {
			int timePerFrame = TOTAL_ANIMATION_TIME / NUMBER_OF_FRAMES;

			return  (int) ((TOTAL_ANIMATION_TIME - showSplatTime)/ timePerFrame);
		} else {
			return -1;
		}
	}

	public boolean isFinished() {
		return showSplatTime <= -TOTAL_ANIMATION_TIME / NUMBER_OF_FRAMES;
	}
}

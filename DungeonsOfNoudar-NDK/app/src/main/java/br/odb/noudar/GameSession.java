/**
 *
 */
package br.odb.noudar;

import android.content.res.Resources;
import android.os.Bundle;

import br.odb.menu.GameActivity;

/**
 * @author monty
 */
public class GameSession {

	private GameLevel currentLevel;

	public GameSession() {
    }

    public GameLevel obtainCurrentLevel(Resources res, int level, GameActivity.GameDelegate delegate, GameViewGLES2.GameRenderer renderer) {

	    if ( currentLevel == null || currentLevel.getLevelNumber() != level ) {
		    currentLevel = GameLevelLoader.loadLevel(level, res, delegate, renderer);
	    }

        return currentLevel;
    }

	private boolean hasSavedGameSession(Bundle savedInstanceState) {
		return savedInstanceState != null && savedInstanceState.getSerializable("Level") != null;
	}

	public void restoreFromLevel(Bundle savedInstanceState, GameActivity.GameDelegate gameDelegate, GameViewGLES2.GameRenderer gameRenderer) {

		if ( hasSavedGameSession( savedInstanceState ) ) {
			GameLevel level = (GameLevel) savedInstanceState.getSerializable("Level");
			level.setDelegates( gameDelegate, gameRenderer );
			currentLevel = level;
		}
	}

	public void saveLevelTo(Bundle outState) {
		outState.putSerializable("Level", currentLevel);
	}
}

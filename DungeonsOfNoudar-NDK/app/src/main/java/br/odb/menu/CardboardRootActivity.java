package br.odb.menu;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import br.odb.noudar.GameConfigurations;
import br.odb.noudar.GameLevelLoader;
import br.odb.noudar.GameSession;
import br.odb.noudar.R;

import static br.odb.menu.GameActivity.USE_VR;

public class CardboardRootActivity extends Activity {

    public static final String MAPKEY_SUCCESSFUL_LEVEL_OUTCOME = "outcome";
    public static final String MAPKEY_SUCCESSFUL_LEVEL_COMPLETION = "good";
    public static final String MAPKEY_LEVEL_TO_PLAY = "level";
	private static final int PLAY_GAME_REQUEST_CODE = 1;

	public enum GameOutcome { UNDEFINED, VICTORY, DEFEAT }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

	    playNextLevel( 0 );
    }

	private void onLevelEnded(int levelPlayed, GameOutcome outcome) {
		if (outcome == GameOutcome.VICTORY) {

			++levelPlayed;

			if (levelPlayed > GameLevelLoader.NUMBER_OF_LEVELS) {
				showGameEnding();
			} else {
				playNextLevel( levelPlayed );
			}
		} else if (outcome == GameOutcome.DEFEAT) {
			showGameOver();
		}
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == PLAY_GAME_REQUEST_CODE && data != null) {
			Bundle bundle = data.getExtras();
			GameOutcome outcome = GameOutcome.values()[bundle.getInt(MAPKEY_SUCCESSFUL_LEVEL_COMPLETION)];
			int levelPlayed = data.getIntExtra(MAPKEY_LEVEL_TO_PLAY, 0);
			onLevelEnded( levelPlayed, outcome );
		}
	}

	private void playNextLevel(int levelToPlay) {
		Intent intent = new Intent(getBaseContext(), GameActivity.class);
		intent.putExtra(MAPKEY_LEVEL_TO_PLAY, levelToPlay);
		intent.putExtra(USE_VR, Boolean.TRUE);
		startActivityForResult(intent, PLAY_GAME_REQUEST_CODE);
		overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
	}

    private void showGameOver() {
        Intent intent = new Intent(this, ShowOutcomeActivity.class);
        intent.putExtra(MAPKEY_SUCCESSFUL_LEVEL_OUTCOME, GameOutcome.DEFEAT.toString());
        this.startActivity(intent);
	    overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
    }

    private void showGameEnding() {
        Intent intent = new Intent(this, ShowOutcomeActivity.class);
        intent.putExtra(MAPKEY_SUCCESSFUL_LEVEL_OUTCOME, GameOutcome.VICTORY.toString());
        this.startActivity(intent);
	    overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
    }
}
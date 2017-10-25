package br.odb.menu;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import br.odb.SoundManager;
import br.odb.noudar.R;

import static br.odb.menu.GameActivity.USE_VR;

public class RootActivity extends Activity {

	public static final String MAPKEY_SUCCESSFUL_LEVEL_OUTCOME = "outcome";
	private static final int NUMBER_OF_LEVELS = 7;

	public enum GameOutcome { UNDEFINED, VICTORY, DEFEAT }

	private SoundManager mSoundManager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		Typeface font = Typeface.createFromAsset(getAssets(), "fonts/MedievalSharp.ttf");
		( (TextView)findViewById(R.id.tvTitle) ).setTypeface( font );

		mSoundManager = new SoundManager( getApplicationContext() );

		findViewById(R.id.btStart).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				playGame();
			}
		});
//		findViewById(R.id.btnCredits).setOnClickListener(new View.OnClickListener() {
//			@Override
//			public void onClick(View v) {
//				showCredits();
//			}
//		});
		findViewById(R.id.btnHowToPlay).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				showHowToPlay();
			}
		});

		((Button)findViewById(R.id.btStart)).setTypeface( font );
//		((Button)findViewById(R.id.btnCredits)).setTypeface( font );
		((Button)findViewById(R.id.btnHowToPlay)).setTypeface( font );

		mSoundManager.playMusic( R.raw.canto_rg );
    }

	private void playGame() {
		Intent intent = new Intent(getBaseContext(), GameActivity.class);
		intent.putExtra(USE_VR, Boolean.FALSE);
		startActivity(intent);
		overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
	}

	private void onLevelEnded(int levelPlayed, GameOutcome outcome) {
		if (outcome == GameOutcome.VICTORY) {

			++levelPlayed;

			if (levelPlayed > NUMBER_OF_LEVELS) {
				showGameEnding();
			}
		} else if (outcome == GameOutcome.DEFEAT) {
			showGameOver();
		}
	}

	private void showHowToPlay() {
		Intent intent = new Intent(this, ShowHowToPlayActivity.class);
		startActivity(intent);
		overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
	}

	private void showCredits() {
		Intent intent = new Intent(this, ShowCreditsActivity.class);
		startActivity(intent);
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

	@Override
	protected void onPause() {
		mSoundManager.stop();
		super.onPause();
	}
}
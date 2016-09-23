package br.odb.menu;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import br.odb.noudar.GameConfigurations;
import br.odb.noudar.GameLevelLoader;
import br.odb.noudar.GameSession;
import br.odb.noudar.R;

public class KnightsOfAlentejoSplashActivity extends Activity {

    public static final String MAPKEY_SUCCESSFUL_LEVEL_OUTCOME = "outcome";
    public static final String MAPKEY_SUCCESSFUL_LEVEL_COMPLETION = "good";
    public static final String MAPKEY_LEVEL_TO_PLAY = "level";
	private static final int PLAY_GAME_REQUEST_CODE = 1;

	public enum GameOutcome { UNDEFINED, VICTORY, DEFEAT }

	private SoundManager mSoundManager;

	boolean playInVR = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);
	    Typeface font = Typeface.createFromAsset(getAssets(), "fonts/MedievalSharp.ttf");
	    ( (TextView)findViewById(R.id.tvTitle) ).setTypeface( font );
//	    ( (TextView)findViewById(R.id.tvSubtitle) ).setTypeface( font );

	    mSoundManager = new SoundManager( getApplicationContext() );

        findViewById(R.id.btStart).setOnClickListener(new OnClickListener() {
	        @Override
	        public void onClick(View v) {
		        playInVR = false;
		        playNextLevel(0);
	        }
        });

	    findViewById(R.id.btnplayInvr).setOnClickListener(new OnClickListener() {
		    @Override
		    public void onClick(View v) {
			    playInVR = true;
			    playNextLevel(0);
		    }
	    });

        findViewById(R.id.btnCredits).setOnClickListener(new OnClickListener() {
	        @Override
	        public void onClick(View v) {
		        showCredits();
	        }
        });
        findViewById(R.id.btnHowToPlay).setOnClickListener(new OnClickListener() {
	        @Override
	        public void onClick(View v) {
		        showHowToPlay();
	        }
        });

	    ((Button)findViewById(R.id.btStart)).setTypeface( font );
	    ((Button)findViewById(R.id.btnCredits)).setTypeface( font );
	    ((Button)findViewById(R.id.btnHowToPlay)).setTypeface( font );

	    mSoundManager.playMusic( R.raw.canto_rg );
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

	private void playNextLevel(int levelToPlay) {

		int score = 0;
		GameSession session = GameConfigurations.getInstance().getCurrentGameSession();
		if ( session != null ) {
			score = session.getScore();
		}

		if ( levelToPlay == 0 ) {
			score = 0;
		}

		GameConfigurations.getInstance().startNewSession(score);
		Intent intent = new Intent(getBaseContext(), GameActivity.class);
		intent.putExtra(MAPKEY_LEVEL_TO_PLAY, levelToPlay);
		intent.putExtra(GameActivity.USE_VR, playInVR);
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

	@Override
	protected void onPause() {
		mSoundManager.stop();
		super.onPause();
	}
}
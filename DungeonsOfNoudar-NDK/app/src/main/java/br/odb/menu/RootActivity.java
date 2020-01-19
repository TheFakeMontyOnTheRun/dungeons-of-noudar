package br.odb.menu;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import br.odb.SoundManager;
import br.odb.noudar.R;

public class RootActivity extends Activity implements CompoundButton.OnCheckedChangeListener {

    public static final String MAPKEY_SUCCESSFUL_LEVEL_OUTCOME = "outcome";
    private SoundManager mSoundManager;
    private Switch chkSound;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        Typeface font = Typeface.createFromAsset(getAssets(), "fonts/MedievalSharp.ttf");
        ((TextView) findViewById(R.id.tvTitle)).setTypeface(font);

        mSoundManager = new SoundManager(getApplicationContext());

        findViewById(R.id.btStart).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                playGame();
            }
        });
		findViewById(R.id.btnCredits).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				showCredits();
			}
		});
        findViewById(R.id.btnHowToPlay).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showHowToPlay();
            }
        });

        this.chkSound = findViewById (R.id.swEnableSound);
        chkSound.setChecked(( (NoudarApplication) getApplication()).mayEnableSound());
        chkSound.setOnCheckedChangeListener(this);

        ((Button) findViewById(R.id.btStart)).setTypeface(font);
		((Button)findViewById(R.id.btnCredits)).setTypeface( font );
        ((Button) findViewById(R.id.btnHowToPlay)).setTypeface(font);
    }

    private void playGame() {
        Bundle bundle = new Bundle();
        bundle.putString("hasSound", chkSound.isChecked() ? "y" : "n");
        Intent intent = new Intent(getBaseContext(), GameActivity.class);
        intent.putExtras(bundle);
        startActivityForResult(intent, 1);
        overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
    }

    private void onLevelEnded(int levelPlayed) {
        if (levelPlayed == 7) {
            showGameEnding();
        } else if (levelPlayed == 8) {
            showGameOver();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        onLevelEnded(resultCode);
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

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
    }

    public enum GameOutcome {UNDEFINED, VICTORY, DEFEAT}
}
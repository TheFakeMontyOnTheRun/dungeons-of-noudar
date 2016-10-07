package br.odb.menu;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import br.odb.noudar.R;

import static br.odb.menu.GameActivity.USE_VR;

public class KnightsOfAlentejoSplashActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
	    playNextLevel(0);
    }

	private void playNextLevel(int levelToPlay) {
		Intent intent = new Intent(getBaseContext(), GameActivity.class);
		intent.putExtra(USE_VR, Boolean.FALSE);
		startActivity(intent);
		overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
		finish();
	}
}
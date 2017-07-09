package br.odb.menu;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import br.odb.noudar.R;

import static br.odb.menu.GameActivity.USE_VR;

public class CardboardRootActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

		Intent intent = new Intent(getBaseContext(), CardboardGameActivity.class);
		intent.putExtra(USE_VR, Boolean.TRUE);
		startActivity(intent);
		overridePendingTransition(R.anim.fade_out, R.anim.fade_in);
		finish();
	}
}
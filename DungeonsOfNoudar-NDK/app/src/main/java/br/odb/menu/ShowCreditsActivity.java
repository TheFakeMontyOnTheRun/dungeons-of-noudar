package br.odb.menu;

import android.app.Activity;
import android.os.Bundle;

import br.odb.noudar.R;

public class ShowCreditsActivity extends Activity {

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.credits_layout);
    }
}
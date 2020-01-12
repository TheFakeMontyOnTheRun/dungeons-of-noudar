package br.odb.menu;

import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.View;

import br.odb.ControllerHelper;
import br.odb.GL2JNILib;
import br.odb.GameViewGLES2;
import br.odb.noudar.R;

public class GameActivity extends Activity {

    private ControllerHelper mControllerHelper;
    private GameViewGLES2 view;
    private boolean mHaveController;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        mControllerHelper = new ControllerHelper(this);

        configureUiForInputDevice();

        setContentView(R.layout.game3d_layout);

        if (mHaveController ) {
            if(getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
                findViewById(R.id.rlLeftPanel).setVisibility(View.GONE);
                findViewById(R.id.rlRightPanel).setVisibility(View.GONE);
            } else {
                findViewById(R.id.rlControllers).setVisibility(View.GONE);
            }
        }

        view = findViewById(R.id.gameView1);

        view.init(this, 0, mHaveController);

        findViewById(R.id.btnUp).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.moveUp();
            }
        });

        findViewById(R.id.btnDown).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.moveDown();
            }
        });

        findViewById(R.id.btnLeft).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.rotateLeft();
            }
        });

        findViewById(R.id.btnRight).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.rotateRight();
            }
        });

        findViewById(R.id.btnAttack).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.useItem();
            }
        });

        findViewById(R.id.btnPick).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.pickItem();
            }
        });

        findViewById(R.id.btnNextItem).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.cycleNextItem();
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();

        view.onPause();
        view.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();

        view.onCreate(getAssets());
        view.onResume();

    }
    //presentation and interaction

    private void configureUiForInputDevice() {
        mHaveController = mControllerHelper.hasGamepad() || mControllerHelper.hasPhysicalKeyboard();
    }
}

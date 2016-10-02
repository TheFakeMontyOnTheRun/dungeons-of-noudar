package br.odb.menu;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.KeyEvent;
import android.widget.TextView;

import com.google.vr.sdk.base.GvrActivity;

import br.odb.noudar.GameConfigurations;
import br.odb.noudar.GameViewGLES2;
import br.odb.noudar.R;

public class GameActivity extends GvrActivity {

	public static final String USE_VR = "use-vr";

	private ControllerHelper mControllerHelper;
	private int floorNumber;
	private GameViewGLES2 view;
	private boolean mHaveController;

	public interface GameDelegate {
		void onGameOver();

		void onFatalError();

		void onKnightExited();
	}

	GameDelegate gameDelegate = new GameDelegate() {

		@Override
		public void onGameOver() {
			endLevelWithResult(KnightsOfAlentejoSplashActivity.GameOutcome.DEFEAT);
		}

		@Override
		public void onFatalError() {
			finish();
		}

		@Override
		public void onKnightExited() {
			endLevelWithResult(KnightsOfAlentejoSplashActivity.GameOutcome.VICTORY);
		}
	};

	private void endLevelWithResult(final KnightsOfAlentejoSplashActivity.GameOutcome outcome) {
		view.fadeOut();

		new Handler().postDelayed(new Runnable() {
			@Override
			public void run() {
				view.stopRunning();

				Intent intent = new Intent();
				Bundle bundle = new Bundle();
				bundle.putInt(KnightsOfAlentejoSplashActivity.MAPKEY_SUCCESSFUL_LEVEL_COMPLETION, outcome.ordinal());
				bundle.putInt(KnightsOfAlentejoSplashActivity.MAPKEY_LEVEL_TO_PLAY, floorNumber);
				intent.putExtras(bundle);

				setResult(RESULT_OK, intent);
				finish();

				overridePendingTransition(R.anim.hold, R.anim.fade_out);
			}
		}, 1000);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		mControllerHelper = new ControllerHelper( this );

		this.floorNumber = getIntent().getIntExtra(KnightsOfAlentejoSplashActivity.MAPKEY_LEVEL_TO_PLAY, 0);
		boolean playInVR = getIntent().getBooleanExtra(USE_VR, false);

		configureUiForInputDevice();
		restoreGameSession(savedInstanceState);


		view = new GameViewGLES2( this );
		setContentView( view );
		view.setStereoModeEnabled( playInVR );
		view.init(this, gameDelegate, floorNumber, mHaveController);
		setGvrView( view );
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

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		GameConfigurations.getInstance().getCurrentGameSession().saveLevelTo( outState );

		super.onSaveInstanceState(outState);
	}

	private void restoreGameSession(Bundle savedInstanceState) {
		if ( savedInstanceState != null ) {
			GameConfigurations.getInstance().getCurrentGameSession().restoreFromLevel(savedInstanceState, GameActivity.this.gameDelegate, view.getRenderingDelegate());
		}
	}

	//presentation and interaction

	private void configureUiForInputDevice() {
		mHaveController = mControllerHelper.hasGamepad() || mControllerHelper.hasPhysicalKeyboard();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		boolean handled = super.onKeyDown(keyCode, event);

		if (event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
			finish();
			return true;
		}

		return handled || view.onKeyDown(keyCode, event);
	}
}

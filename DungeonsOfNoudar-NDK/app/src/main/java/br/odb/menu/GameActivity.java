package br.odb.menu;

import android.app.Presentation;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.media.MediaRouter;
import android.os.Bundle;
import android.os.Handler;
import android.view.Display;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.sdk.base.GvrActivity;

import java.util.ArrayList;
import java.util.List;

import br.odb.droidlib.Vector2;
import br.odb.noudar.GameConfigurations;
import br.odb.noudar.GameLevel;
import br.odb.noudar.GameSession;
import br.odb.noudar.GameViewGLES2;
import br.odb.noudar.Knight;
import br.odb.noudar.R;

public class GameActivity extends GvrActivity {

	public static final String USE_VR = "use-vr";

	public interface GameDelegate {
		void onTurnEnded();

		void onGameOver();

		void onLevelFinished();

		void onGameStarted();

		void onFatalError();

		void onKnightChanged();
	}

	GameDelegate gameDelegate = new GameDelegate() {
		@Override
		public void onTurnEnded() {
			List<Knight> listOfKnightOnTheLevel = getListOfAvailableKnights();

			boolean thereAreNoAliveKnightsOnTheLevel = listOfKnightOnTheLevel.isEmpty();

			if (hasPlayerKilledAllMonsters() || (thereAreNoAliveKnightsOnTheLevel && hasAnyKnightExited())) {
				proceedToNextLevel();
				return;
			} else if (thereAreNoAliveKnightsOnTheLevel && !hasAnyKnightExited()) {
				endGameAsDefeat();
				return;
			}

			updateUI( listOfKnightOnTheLevel );
		}

		@Override
		public void onGameOver() {
			updateUI(getListOfAvailableKnights());
			Intent intent = new Intent();
			Bundle bundle = new Bundle();
			bundle.putInt(KnightsOfAlentejoSplashActivity.MAPKEY_SUCCESSFUL_LEVEL_COMPLETION, KnightsOfAlentejoSplashActivity.GameOutcome.DEFEAT.ordinal());
			bundle.putInt(KnightsOfAlentejoSplashActivity.MAPKEY_LEVEL_TO_PLAY, floorNumber);
			intent.putExtras(bundle);
			final Intent finalIntent = intent;
			view.fadeOut();
			new Handler().postDelayed(new Runnable() {
				@Override
				public void run() {
					setResult(RESULT_OK, finalIntent);
					view.stopRunning();
					finish();
					overridePendingTransition(R.anim.hold, R.anim.fade_out);
				}
			}, 1000);
		}

		@Override
		public void onLevelFinished() {
			Intent intent = new Intent();
			Bundle bundle = new Bundle();
			bundle.putInt(KnightsOfAlentejoSplashActivity.MAPKEY_SUCCESSFUL_LEVEL_COMPLETION, KnightsOfAlentejoSplashActivity.GameOutcome.VICTORY.ordinal());
			bundle.putInt(KnightsOfAlentejoSplashActivity.MAPKEY_LEVEL_TO_PLAY, floorNumber);
			intent.putExtras(bundle);
			final Intent finalIntent = intent;
			view.fadeOut();
			new Handler().postDelayed(new Runnable() {
				@Override
				public void run() {
					setResult(RESULT_OK, finalIntent);
					view.stopRunning();
					finish();
					overridePendingTransition(R.anim.hold, R.anim.fade_out);
				}
			}, 1000);
		}

		@Override
		public void onGameStarted() {
			view.getCurrentLevel().selectDefaultKnight();
			List<Knight> listOfKnightOnTheLevel = getListOfAvailableKnights();
			updateUI( listOfKnightOnTheLevel );
		}

		@Override
		public void onFatalError() {
			finish();
			overridePendingTransition(R.anim.hold, R.anim.fade_out);
		}

		@Override
		public void onKnightChanged() {
			List<Knight> listOfKnightOnTheLevel = getListOfAvailableKnights();
			updateUI( listOfKnightOnTheLevel );
		}
	};


	public enum Direction {
		N(0, -1),
		E(1, 0),
		S(0, 1),
		W(-1, 0);

		private final Vector2 offsetVector = new Vector2(0, 0);

		Direction(int x, int y) {
			offsetVector.x = x;
			offsetVector.y = y;
		}

		public Vector2 getOffsetVector() {
			return offsetVector;
		}
	}

	private final static class GamePresentation extends Presentation {

		final GameViewGLES2 canvas;

		public GamePresentation(Context context, Display display, GameViewGLES2 gameView) {
			super(context, display);

			this.canvas = gameView;
		}

		@Override
		protected void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);
			setContentView(canvas);
		}
	}

	private int floorNumber;
	private GameViewGLES2 view;
	private boolean mHaveController;

	//basic Activity structure
	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		view = new GameViewGLES2( this );

		setContentView( view );
        setGvrView( view );


		configureUiForInputDevice();

		this.floorNumber = getIntent().getIntExtra(KnightsOfAlentejoSplashActivity.MAPKEY_LEVEL_TO_PLAY, 0);
		boolean playInVR = getIntent().getBooleanExtra(USE_VR, false);

		if (hasSavedGameSession(savedInstanceState)) {
			restoreGameSession(savedInstanceState);
		} else {
			greetPlayerOnLevelProgress();
		}

		useBestRouteForGameplayPresentation();
		view.setStereoModeEnabled( playInVR );
		view.init(this, gameDelegate, floorNumber, mHaveController);

	}

	@Override
	protected void onPause() {
		super.onPause();
		view.onPause();
		synchronized (view.renderingLock) {
			view.onDestroy();
		}
	}

	@Override
	protected void onResume() {
		super.onResume();

		synchronized (view.renderingLock) {
			view.onCreate(getAssets());
		}

		gameDelegate.onGameStarted();
		view.onResume();
		enterImmersiveMode();
	}

	private boolean hasPlayerKilledAllMonsters() {
		return view.getCurrentLevel().getMonsters() == 0;
	}

	private boolean hasAnyKnightExited() {
		return view.getExitedKnights() > 0;
	}

	private void proceedToNextLevel() {
		gameDelegate.onLevelFinished();
	}

	private void endGameAsDefeat() {
		gameDelegate.onGameOver();
	}

	private boolean hasSavedGameSession(Bundle savedInstanceState) {
		return savedInstanceState != null && savedInstanceState.getSerializable("Level") != null;
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {

		outState.putSerializable("Level", view.getCurrentLevel());
		super.onSaveInstanceState(outState);
	}

	private void restoreGameSession(Bundle savedInstanceState) {
		GameLevel level = (GameLevel) savedInstanceState.getSerializable("Level");
		level.setDelegates( gameDelegate, view.getRenderingDelegate() );
		GameSession configuration = GameConfigurations.getInstance().getCurrentGameSession();
		configuration.restoreFromLevel(level);
	}

	//presentation and interaction

	private void enterImmersiveMode() {
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		getWindow().getDecorView().setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
						| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_FULLSCREEN
						| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
	}

	private void configureUiForInputDevice() {
		mHaveController = hasGamepad() || !hasTouchscreen() || hasPhysicalKeyboard();
	}

	private boolean hasGamepad() {
		return getGameControllerIds().size() > 0;
	}

	private void greetPlayerOnLevelProgress() {
		if (floorNumber > 0) {
			view.displayLevelAdvanceMessage();
		} else {
			view.displayGreetingMessage();
		}
	}

	private void useBestRouteForGameplayPresentation() {
		MediaRouter.RouteInfo mRouteInfo = findSecundaryDisplayRouter();

		if (mRouteInfo != null) {
			Display presentationDisplay = mRouteInfo.getPresentationDisplay();
			if (presentationDisplay != null) {
				useSecundaryDisplayForGameplayPresentation(presentationDisplay);
			}
		}
	}

	private MediaRouter.RouteInfo findSecundaryDisplayRouter() {
		MediaRouter mMediaRouter = (MediaRouter) getSystemService(Context.MEDIA_ROUTER_SERVICE);
		return mMediaRouter.getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_VIDEO);
	}

	private void useSecundaryDisplayForGameplayPresentation(Display presentationDisplay) {
		view.getParentViewManager().removeView(view);
		Presentation presentation = new GamePresentation(this, presentationDisplay, view);
		presentation.show();
	}

	private boolean hasTouchscreen() {
		return getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN);
	}

	private boolean hasPhysicalKeyboard() {
		return getResources().getConfiguration().keyboard != Configuration.KEYBOARD_NOKEYS;
	}

	private List<Integer> getGameControllerIds() {
		List<Integer> gameControllerDeviceIds = new ArrayList<>();

		int[] deviceIds = InputDevice.getDeviceIds();
		for (int deviceId : deviceIds) {
			InputDevice dev = InputDevice.getDevice(deviceId);
			int sources = dev.getSources();

			if (((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
					|| ((sources & InputDevice.SOURCE_JOYSTICK)
					== InputDevice.SOURCE_JOYSTICK)) {

				if (!gameControllerDeviceIds.contains(deviceId)) {
					gameControllerDeviceIds.add(deviceId);
				}
			}
		}

		return gameControllerDeviceIds;
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

	List<Knight> getListOfAvailableKnights() {
		List<Knight> listOfKnightOnTheLevel = new ArrayList<>();

		Knight selectedKnight = view.getCurrentLevel().getSelectedPlayer();

		if (selectedKnight != null && selectedKnight.isAlive()) {
			listOfKnightOnTheLevel.add(selectedKnight);
		}

		for (Knight k : view.getCurrentLevel().getKnights()) {
			if (!listOfKnightOnTheLevel.contains(k)) {
				if ( k.isAlive() ) {
					listOfKnightOnTheLevel.add(k);
				}
			}
		}

		return listOfKnightOnTheLevel;
	}

	void updateUI( List<Knight> knights ) {
	}
}

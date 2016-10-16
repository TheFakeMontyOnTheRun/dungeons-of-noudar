package br.odb.menu;

import android.os.Bundle;
import android.view.KeyEvent;

import com.google.vr.sdk.base.GvrActivity;

import br.odb.CardboardGameViewGLES2;
import br.odb.ControllerHelper;

public class CardboardGameActivity extends GvrActivity {

	public static final String USE_VR = "use-vr";

	private ControllerHelper mControllerHelper;
	private CardboardGameViewGLES2 view;
	private boolean mHaveController;

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		mControllerHelper = new ControllerHelper( this );
		boolean playInVR = getIntent().getBooleanExtra(USE_VR, false);

		configureUiForInputDevice();

		view = new CardboardGameViewGLES2( this );
		setContentView( view );
		view.setStereoModeEnabled( playInVR );
		view.init(this, 0, mHaveController);
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

/**
 *
 */
package br.odb;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;

import com.google.vr.sdk.base.Eye;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.HeadTransform;
import com.google.vr.sdk.base.Viewport;

import java.io.IOException;

import javax.microedition.khronos.egl.EGLConfig;

import static java.security.AccessController.getContext;

/**
 * @author monty
 */
public class CardboardGameViewGLES2 extends GvrView implements GvrView.StereoRenderer {

	public static final int TICK_INTERVAL = 20;
	private AssetManager assets;

	@Override
	public void onNewFrame(HeadTransform headTransform) {
		long delta = tick();
		GL2JNILib.tick(delta);

		headTransform.getEulerAngles(forwardVector, 0);
		float xz = extractAngleXZFromHeadtransform(headTransform);
		float yz = extractAngleYZFromHeadtransform(headTransform);

		if (!mHaveController && getStereoModeEnabled()) {

			int previousRotation = rotation;

			rotation = (int) (( 360- xz) / 90) % 4;

			while ( rotation < 0 ) {
				rotation += 4;
			}

			if ( rotation != previousRotation ) {
				GL2JNILib.forcePlayerDirection( rotation );
			}
		}

		if (getStereoModeEnabled()) {
			GL2JNILib.setXZAngle(xz);
			GL2JNILib.setYZAngle(yz);
		}
	}


	@Override
	public void onDrawEye(Eye eye) {

		synchronized (renderingLock) {
			GLES20.glEnable(GLES20.GL_DEPTH_TEST);

			if (getStereoModeEnabled()) {
				GL2JNILib.setEyeMatrix(eye.getEyeView());
				GL2JNILib.setPerspectiveMatrix(eye.getPerspective(0.1f, 100.0f));
			}

			GL2JNILib.step();
		}
	}

	@Override
	public void onFinishFrame(Viewport viewport) {
	}

	@Override
	public void onSurfaceChanged(int width, int height) {
		GL2JNILib.init(width, height, this.assets);
	}

	@Override
	public void onSurfaceCreated(EGLConfig eglConfig) {
	}

	@Override
	public void onRendererShutdown() {
	}


	public enum KB {
		UP, RIGHT, DOWN, LEFT, ROTATE_LEFT, ROTATE_RIGHT
	}

	OnKeyListener keyListener = new OnKeyListener() {
		@Override
		public synchronized boolean onKey(View v, int keyCode, KeyEvent event) {

			if (event.getAction() == KeyEvent.ACTION_DOWN) {

				if (keyCode == KeyEvent.KEYCODE_COMMA || keyCode == KeyEvent.KEYCODE_BUTTON_L1) {
					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.LEFT);
				}

				if (keyCode == KeyEvent.KEYCODE_PERIOD || keyCode == KeyEvent.KEYCODE_BUTTON_R1) {
					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.RIGHT);
				}

				if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {

					KB newValue = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.UP);

					if (key == newValue) {
						GL2JNILib.onLongPressingMove();
					}

					key = newValue;
				}

				if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.DOWN);
				}

				if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT) {
					key = KB.ROTATE_LEFT;
				}

				if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
					key = KB.ROTATE_RIGHT;
				}
			} else {
				GL2JNILib.onReleasedLongPressingMove();
			}

			return true;// key != null;
		}
	};

	final private Object renderingLock = new Object();
	private long timeUntilTick;
	private long t0;
	CardboardGameViewGLES2.KB key = null;
	private float[] forwardVector = new float[3];
	boolean mHaveController;
	int rotation = 0;

	private long tick() {

		long delta = (System.currentTimeMillis() - t0);

		timeUntilTick -= delta;

		if (timeUntilTick < 0) {

			if (key != null) {
				handleCommand(key);
				key = null;
			}

			timeUntilTick = TICK_INTERVAL;
			t0 = System.currentTimeMillis();
		}

		return delta;
	}

	private void init() {
		setRenderer(this);
		setFocusable(true);
		t0 = System.currentTimeMillis();
	}

	public CardboardGameViewGLES2(Context context) {
		super(context);
		init();
	}


	public CardboardGameViewGLES2(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	public void init(Context context, int level, boolean haveController) {
		mHaveController = haveController;

		if (haveController) {
			Log.d("Monty", "We have controllers!");
			setOnKeyListener(keyListener);
			requestFocus();
			requestFocusFromTouch();
		}

		if (this.getStereoModeEnabled()) {
			setOnCardboardTriggerListener(new Runnable() {
				@Override
				public void run() {
					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.UP);
				}
			});
		} else {
			setOnTouchListener(new OnSwipeTouchListener(getContext()) {

				@Override
				public void onSwipeLeft() {
					super.onSwipeLeft();

					key = KB.ROTATE_LEFT;
				}

				@Override
				public void onSwipeRight() {
					super.onSwipeRight();

					key = KB.ROTATE_RIGHT;
				}

				@Override
				public void onSwipeUp() {
					super.onSwipeUp();
					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.UP);
				}

				@Override
				public void onSwipeDown() {
					super.onSwipeDown();
					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.DOWN);
				}
			});
		}

		requestFocus();
		requestFocusFromTouch();
	}

	public void onDestroy() {
		synchronized (renderingLock) {
			GL2JNILib.onDestroy();
		}
	}

	public void onCreate(AssetManager assets) {
		synchronized (renderingLock) {
			this.assets = assets;
			GL2JNILib.onCreate(assets);
			loadTextures(assets);

			final Activity activity = ((Activity) getContext());

			GL2JNILib.loadSounds( activity.getAssets(), new String[] {
					"grasssteps.wav", //0
					"stepsstones.wav", //1
					"bgnoise.wav", //2
					"monsterdamage.wav", //3
					"monsterdead.wav", //4
					"playerdamage.wav", //5
					"playerdead.wav", //6
					"swing.wav" //7
			});
		}
	}


	private void loadMeshes(AssetManager assets) {
		GL2JNILib.setMeshes( assets, new String[]{"cube.obj"});
	}

	public void setTextures(Bitmap[] bitmaps) {

	}

	public void loadTextures(AssetManager assets) {
		try {
			Bitmap[] bitmaps;

			bitmaps = loadBitmaps(assets, new String[]{
					"grass.png",
					"stonefloor.png",
					"bricks.png",
					"arch.png",
					"bars.png",
					"begin.png",
					"exit.png",
					"bricks_blood.png",
					"bricks_candles.png",
					"foe0.png",
					"foe1.png",
					"foe2.png",
					"foe3.png",
					"foe4.png",
					"foe5.png",
					"crusader0.png",
					"crusader1.png",
					"crusader2.png",
					"shadow.png",
					"ceiling.png",
					"ceilingdoor.png",
					"ceilingbegin.png",
					"ceilingend.png",
					"splat0.png",
					"splat1.png",
					"splat2.png",
					"ceilingbars.png",
					"clouds.png",
					"stonegrassfar.png",
					"grassstonefar.png",
					"stonegrassnear.png",
					"grassstonenear.png",
					"cross.png",
			});
			setTextures(bitmaps);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private Bitmap[] loadBitmaps(AssetManager assets, String[] filenames) throws IOException {
		Bitmap[] toReturn = new Bitmap[filenames.length];

		for (int i = 0; i < filenames.length; i++) {
			toReturn[i] = BitmapFactory.decodeStream(assets.open(filenames[i]));
		}

		return toReturn;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		return keyListener.onKey(this, keyCode, event);
	}

	@Override
	public void onResume() {
		super.onResume();
		setFocusable(true);
		requestFocus();
	}

	public KB transformMovementToCameraRotation(KB direction) {
		return direction;
	}

	public void handleCommand(final KB key) {
		synchronized (renderingLock) {
			switch (key) {

				case UP:
					GL2JNILib.moveUp();
					break;

				case DOWN:
					GL2JNILib.moveDown();
					break;

				case LEFT:
					GL2JNILib.moveLeft();
					break;

				case RIGHT:
					GL2JNILib.moveRight();
					break;

				case ROTATE_LEFT:
					GL2JNILib.rotateLeft();
					break;

				case ROTATE_RIGHT:
					GL2JNILib.rotateRight();
					break;

			}

		}
	}

	private float extractAngleXYFromHeadtransform(HeadTransform headTransform) {
		return 360.0f - ((float) (forwardVector[2] * (180 / Math.PI)));
	}

	private float extractAngleYZFromHeadtransform(HeadTransform headTransform) {
		return ((float) (forwardVector[0] * (180 / Math.PI)));
	}

	private float extractAngleXZFromHeadtransform(HeadTransform headTransform) {
		return ((float) (forwardVector[1] * (180 / Math.PI)));
	}
}

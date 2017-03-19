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
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;

import java.io.IOException;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * @author monty
 */
public class GameViewGLES2 extends GLSurfaceView implements GLSurfaceView.Renderer {

	public static final int TICK_INTERVAL = 20;
	private AssetManager assets;

	@Override
	public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
	}

	@Override
	public void onSurfaceChanged(GL10 gl10, int width, int height) {
		GL2JNILib.init(width, height, this.assets);
	}

	@Override
	public void onDrawFrame(GL10 gl10) {
		long delta = tick();
		GL2JNILib.tick(TICK_INTERVAL);

		synchronized (renderingLock) {
			GLES20.glEnable(GLES20.GL_DEPTH_TEST);
			GL2JNILib.step();
		}
	}


	public enum KB {
		UP, RIGHT, DOWN, LEFT, ROTATE_LEFT, ROTATE_RIGHT, CYCLE_PREV, CYCLE_NEXT, PICK, DROP, USE
	}

	View.OnKeyListener keyListener = new OnKeyListener() {
		@Override
		public synchronized boolean onKey(View v, int keyCode, KeyEvent event) {

			if (event.getAction() == KeyEvent.ACTION_DOWN) {

				if (keyCode == KeyEvent.KEYCODE_COMMA || keyCode == KeyEvent.KEYCODE_BUTTON_L1) {
					key = transformMovementToCameraRotation(GameViewGLES2.KB.LEFT);
				}

				if (keyCode == KeyEvent.KEYCODE_PERIOD || keyCode == KeyEvent.KEYCODE_BUTTON_R1) {
					key = transformMovementToCameraRotation(GameViewGLES2.KB.RIGHT);
				}

				if (keyCode == KeyEvent.KEYCODE_MINUS || keyCode == KeyEvent.KEYCODE_BUTTON_L2) {
					key = GameViewGLES2.KB.CYCLE_PREV;
				}

				if (keyCode == KeyEvent.KEYCODE_EQUALS || keyCode == KeyEvent.KEYCODE_BUTTON_R2) {
					key = GameViewGLES2.KB.CYCLE_NEXT;
				}

				if (keyCode == KeyEvent.KEYCODE_Y || keyCode == KeyEvent.KEYCODE_BUTTON_Y) {
					key = GameViewGLES2.KB.PICK;
				}

				if (keyCode == KeyEvent.KEYCODE_B || keyCode == KeyEvent.KEYCODE_BUTTON_B) {
					key = GameViewGLES2.KB.DROP;
				}

				if (keyCode == KeyEvent.KEYCODE_A || keyCode == KeyEvent.KEYCODE_BUTTON_A) {
					key = GameViewGLES2.KB.USE;
				}



				if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {

					KB newValue = transformMovementToCameraRotation(GameViewGLES2.KB.UP);

					if (key == newValue) {
						GL2JNILib.onLongPressingMove();
					}

					key = newValue;
				}

				if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
					key = transformMovementToCameraRotation(GameViewGLES2.KB.DOWN);
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
	GameViewGLES2.KB key = null;
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

		setEGLContextClientVersion(2);

		setRenderer(this);
		setFocusable(true);
		t0 = System.currentTimeMillis();
	}

	public GameViewGLES2(Context context) {
		super(context);
		init();
	}


	public GameViewGLES2(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	boolean touching = false;

	public void init(Context context, int level, boolean haveController) {
		mHaveController = haveController;

		if (haveController) {
			setOnKeyListener(keyListener);
		}

	setOnTouchListener(new OnSwipeTouchListener(getContext()) {

		@Override
		public void onSwipeLeft() {
			super.onSwipeLeft();

			if (touching ) {
				key = KB.CYCLE_NEXT;
			} else {
				key = KB.ROTATE_RIGHT;
			}
		}

		@Override
		public void onSwipeRight() {
			super.onSwipeRight();

			if (touching ) {
				key = KB.CYCLE_PREV;
			} else {
				key = KB.ROTATE_LEFT;
			}
		}

		@Override
		public void onSwipeUp() {
			super.onSwipeUp();

			if (!touching) {
				key = transformMovementToCameraRotation(GameViewGLES2.KB.UP);
			}
		}

		@Override
		public void onDoubleTap() {
			super.onDoubleTap();

			if ( GL2JNILib.isThereAnyObjectInFrontOfYou() ) {
				key = KB.PICK;
			} else {
				key = KB.DROP;
			}
		}

		@Override
		public void onLongPress() {
			super.onLongPress();

			if (!touching) {
				key = KB.USE;
			}
		}

		@Override
		public void onSwipeDown() {
			super.onSwipeDown();

			if (!touching) {
				key = transformMovementToCameraRotation(GameViewGLES2.KB.DOWN);
			}
		}
	});

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
			loadMeshes(assets);
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
	public void onResume() {
		super.onResume();
		setFocusable(true);
		requestFocus();
	}

	public KB transformMovementToCameraRotation(KB direction) {
		return direction;
	}

	public synchronized void handleCommand(final KB key) {
		synchronized (renderingLock) {
			if (!GL2JNILib.isAnimating()) {
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


					case CYCLE_PREV:
						GL2JNILib.cyclePreviousItem();
						break;
					case CYCLE_NEXT:
						GL2JNILib.cycleNextItem();
						break;

					case DROP:
						GL2JNILib.dropItem();
						break;

					case PICK:
						GL2JNILib.pickItem();
						break;

					case USE:
						GL2JNILib.useItem();
						break;



				}
			}
		}
	}
}

//package br.odb;
//
//import android.app.Activity;
//import android.content.Context;
//import android.content.res.AssetManager;
//import android.opengl.GLES20;
//import android.util.AttributeSet;
//import android.util.Log;
//import android.view.KeyEvent;
//import android.view.View;
//
//import com.google.vr.sdk.base.Eye;
//import com.google.vr.sdk.base.GvrView;
//import com.google.vr.sdk.base.HeadTransform;
//import com.google.vr.sdk.base.Viewport;
//
//import javax.microedition.khronos.egl.EGLConfig;
//
///**
// * @author monty
// */
//public class CardboardGameViewGLES2 extends GvrView implements GvrView.StereoRenderer {
//
//	public static final int TICK_INTERVAL = 20;
//	private static final float RAD = (float) (180.0f / Math.PI);
//	private AssetManager assets;
//	private boolean mIsStereoEnabled = false;
//
//	@Override
//	public void onNewFrame(HeadTransform headTransform) {
//		long delta = tick();
//		GL2JNILib.tick(delta);
//
//		headTransform.getEulerAngles(forwardVector, 0);
//		float xz = extractAngleXZFromHeadtransform();
//		float yz = extractAngleYZFromHeadtransform();
//		int previousRotation = rotation;
//
//		rotation = (int) ((360 - xz + 45) / 90) % 4;
//
//		while (rotation < 0) {
//			rotation += 4;
//		}
//
//		if (rotation != previousRotation) {
//			GL2JNILib.forcePlayerDirection(rotation);
//		}
//
//		GL2JNILib.setHeadAngles(xz, yz);
//	}
//
//
//	@Override
//	public void onDrawEye(Eye eye) {
//
//		synchronized (renderingLock) {
//
//			if (mIsStereoEnabled) {
//				GL2JNILib.setEyeMatrix(eye.getEyeView());
//				GL2JNILib.setPerspectiveMatrix(eye.getPerspective(0.1f, 100.0f));
//			}
//			GL2JNILib.step();
//		}
//	}
//
//	@Override
//	public void onFinishFrame(Viewport viewport) {
//	}
//
//	@Override
//	public void onSurfaceChanged(int width, int height) {
//		GL2JNILib.init(width, height, this.assets);
//	}
//
//	@Override
//	public void onSurfaceCreated(EGLConfig eglConfig) {
//	}
//
//	@Override
//	public void onRendererShutdown() {
//	}
//
//
//	public enum KB {
//		UP, RIGHT, DOWN, LEFT, ROTATE_LEFT, ROTATE_RIGHT
//	}
//
//	OnKeyListener keyListener = new OnKeyListener() {
//		@Override
//		public synchronized boolean onKey(View v, int keyCode, KeyEvent event) {
//
//			if (event.getAction() == KeyEvent.ACTION_DOWN) {
//
//				if (keyCode == KeyEvent.KEYCODE_COMMA || keyCode == KeyEvent.KEYCODE_BUTTON_L1) {
//					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.LEFT);
//				}
//
//				if (keyCode == KeyEvent.KEYCODE_PERIOD || keyCode == KeyEvent.KEYCODE_BUTTON_R1) {
//					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.RIGHT);
//				}
//
//				if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {
//
//					KB newValue = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.UP);
//
//					if (key == newValue) {
//						GL2JNILib.onLongPressingMove();
//					}
//
//					key = newValue;
//				}
//
//				if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
//					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.DOWN);
//				}
//
//				if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT) {
//					key = KB.ROTATE_LEFT;
//				}
//
//				if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
//					key = KB.ROTATE_RIGHT;
//				}
//			} else {
//				GL2JNILib.onReleasedLongPressingMove();
//			}
//
//			return true;// key != null;
//		}
//	};
//
//	final private Object renderingLock = new Object();
//	private long timeUntilTick;
//	private long t0;
//	CardboardGameViewGLES2.KB key = null;
//	private float[] forwardVector = new float[3];
//	boolean mHaveController;
//	int rotation = 0;
//
//	private long tick() {
//
//		long delta = (System.currentTimeMillis() - t0);
//
//		timeUntilTick -= delta;
//
//		if (timeUntilTick < 0) {
//
//			if (key != null) {
//				handleCommand(key);
//				key = null;
//			}
//
//			timeUntilTick = TICK_INTERVAL;
//			t0 = System.currentTimeMillis();
//		}
//
//		return delta;
//	}
//
//	private void init() {
//		setRenderer(this);
//		setFocusable(true);
//		GL2JNILib.setHUDLessMode( true );
//		t0 = System.currentTimeMillis();
//		mIsStereoEnabled = getStereoModeEnabled();
//	}
//
//	public CardboardGameViewGLES2(Context context) {
//		super(context);
//		init();
//	}
//
//
//	public CardboardGameViewGLES2(Context context, AttributeSet attrs) {
//		super(context, attrs);
//		init();
//	}
//
//	public void init(Context context, boolean haveController) {
//		mHaveController = haveController;
//
//		if (haveController) {
//			Log.d("Monty", "We have controllers!");
//			setOnKeyListener(keyListener);
//			requestFocus();
//			requestFocusFromTouch();
//		}
//
//		if (this.getStereoModeEnabled()) {
//			setOnCardboardTriggerListener(new Runnable() {
//				@Override
//				public void run() {
//					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.UP);
//				}
//			});
//		} else {
//			setOnTouchListener(new OnSwipeTouchListener(getContext()) {
//
//				@Override
//				public void onSwipeLeft() {
//					super.onSwipeLeft();
//
//					key = KB.ROTATE_LEFT;
//				}
//
//				@Override
//				public void onSwipeRight() {
//					super.onSwipeRight();
//
//					key = KB.ROTATE_RIGHT;
//				}
//
//				@Override
//				public void onSwipeUp() {
//					super.onSwipeUp();
//					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.UP);
//				}
//
//				@Override
//				public void onSwipeDown() {
//					super.onSwipeDown();
//					key = transformMovementToCameraRotation(CardboardGameViewGLES2.KB.DOWN);
//				}
//			});
//		}
//
//		requestFocus();
//		requestFocusFromTouch();
//	}
//
//	public void onDestroy() {
//		synchronized (renderingLock) {
//			GL2JNILib.onDestroy();
//		}
//	}
//
//	public void onCreate(AssetManager assets) {
//		synchronized (renderingLock) {
//			this.assets = assets;
//			GL2JNILib.onCreate(assets);
//
//			final Activity activity = ((Activity) getContext());
//
//			GL2JNILib.loadSounds( activity.getAssets(), new String[] {
//					"grasssteps.wav", //0
//					"stepsstones.wav", //1
//					"bgnoise.wav", //2
//					"monsterdamage.wav", //3
//					"monsterdead.wav", //4
//					"playerdamage.wav", //5
//					"playerdead.wav", //6
//					"swing.wav" //7
//			});
//		}
//	}
//
//	@Override
//	public boolean onKeyDown(int keyCode, KeyEvent event) {
//		return keyListener.onKey(this, keyCode, event);
//	}
//
//	@Override
//	public void onResume() {
//		super.onResume();
//		setFocusable(true);
//		requestFocus();
//	}
//
//	public KB transformMovementToCameraRotation(KB direction) {
//		return direction;
//	}
//
//	public void handleCommand(final KB key) {
//		synchronized (renderingLock) {
//			switch (key) {
//
//				case UP:
//					GL2JNILib.moveUp();
//					break;
//
//				case DOWN:
//					GL2JNILib.moveDown();
//					break;
//
//				case LEFT:
//					GL2JNILib.moveLeft();
//					break;
//
//				case RIGHT:
//					GL2JNILib.moveRight();
//					break;
//
//				case ROTATE_LEFT:
//					GL2JNILib.rotateLeft();
//					break;
//
//				case ROTATE_RIGHT:
//					GL2JNILib.rotateRight();
//					break;
//
//			}
//
//		}
//	}
//
//	private float extractAngleYZFromHeadtransform() {
//		return forwardVector[0] * RAD;
//	}
//
//	private float extractAngleXZFromHeadtransform() {
//		return forwardVector[1] * RAD;
//	}
//}

package br.odb;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * @author monty
 */
public class GameViewGLES2 extends GLSurfaceView implements GLSurfaceView.Renderer {

    public static final int TICK_INTERVAL = 20;
    final private Object renderingLock = new Object();
    boolean playing = true;
    GameViewGLES2.KB key = null;
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
    boolean mHaveController;
    private AssetManager assets;
    private Context mContext;
    private long timeUntilTick;
    private long t0;
    public GameViewGLES2(Context context) {
        super(context);
        init(context);
    }
    public GameViewGLES2(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        GL2JNILib.init(width, height, this.assets);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        if (!playing) {
            return;
        }

        synchronized (renderingLock) {
            tick();
            GL2JNILib.tick(TICK_INTERVAL);

            if (mContext instanceof Activity) {
                Activity activity = ((Activity) mContext);
                int level = GL2JNILib.getLevel();
                boolean win = (level == 7);
                boolean loose = (level == 8);

                if (win || loose) {
                    activity.setResult(level);
                    activity.finish();
                    playing = false;
                }
            }
        }
    }

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

    private void init(Context context) {
        this.mContext = context;
        setEGLContextClientVersion(2);

        setRenderer(this);
        setFocusable(true);
        t0 = System.currentTimeMillis();
    }

    public void init(final Context context, int level, boolean haveController) {
        mHaveController = haveController;

        if (mHaveController) {
            setOnKeyListener(keyListener);
        }

        setOnTouchListener(new OnSwipeTouchListener(getContext()) {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                synchronized (GameViewGLES2.this) {

                    long downTime = event.getEventTime() - event.getDownTime();
                    long pressTimeout = ViewConfiguration.get(context).getLongPressTimeout();
                    if (event.getAction() == MotionEvent.ACTION_UP && key == null && (downTime < pressTimeout)) {
                        float pointerX = event.getX();
                        int width = v.getWidth();
                        if (pointerX < width / 3) {
                            key = KB.CYCLE_PREV;
                        } else if (pointerX > ((2 * width) / 3)) {
                            key = KB.CYCLE_NEXT;
                        }
                    }
                }

                return super.onTouch(v, event);
            }

            @Override
            public void onSwipeLeft() {
                super.onSwipeLeft();

                key = KB.ROTATE_RIGHT;
            }

            @Override
            public void onSwipeRight() {
                super.onSwipeRight();

                key = KB.ROTATE_LEFT;
            }

            @Override
            public void onSwipeUp() {
                super.onSwipeUp();

                key = transformMovementToCameraRotation(GameViewGLES2.KB.UP);
            }

            @Override
            public void onDoubleTap() {
                super.onDoubleTap();

                if (GL2JNILib.isThereAnyObjectInFrontOfYou()) {
                    key = KB.PICK;
                } else {
                    key = KB.DROP;
                }
            }

            @Override
            public void onLongPress() {
                super.onLongPress();

                key = KB.USE;
            }

            @Override
            public void onSwipeDown() {
                super.onSwipeDown();

                key = transformMovementToCameraRotation(GameViewGLES2.KB.DOWN);
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
        }
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

    public enum KB {
        UP, RIGHT, DOWN, LEFT, ROTATE_LEFT, ROTATE_RIGHT, CYCLE_PREV, CYCLE_NEXT, PICK, DROP, USE
    }
}

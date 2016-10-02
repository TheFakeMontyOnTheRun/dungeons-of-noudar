/**
 *
 */
package br.odb.noudar;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;

import com.google.vr.sdk.base.Eye;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.HeadTransform;
import com.google.vr.sdk.base.Viewport;

import java.io.IOException;

import javax.microedition.khronos.egl.EGLConfig;

import br.odb.GL2JNILib;
import br.odb.OnSwipeTouchListener;
import br.odb.droidlib.Renderable;
import br.odb.droidlib.Tile;
import br.odb.droidlib.Vector2;
import br.odb.menu.GameActivity;
import br.odb.noudar.characters.Actor;
import br.odb.noudar.characters.Crusader;

/**
 * @author monty
 */
public class GameViewGLES2 extends GvrView implements GvrView.StereoRenderer  {

	public static final int TICK_INTERVAL = 500;

	@Override
	public void onNewFrame(HeadTransform headTransform) {
        long delta = tick();
        GL2JNILib.tick(delta);

        if (needsUpdate) {
            needsUpdate = false;
            updateNativeSnapshot();
        }

        headTransform.getEulerAngles(forwardVector, 0);
        float xz = extractAngleXZFromHeadtransform(headTransform);
        float yz = extractAngleYZFromHeadtransform(headTransform);

	    if ( !mHaveController &&  getStereoModeEnabled() ) {
		    rotation = (int) ((360 - xz) / 90);
	    }

	    if ( getStereoModeEnabled() ) {
		    GL2JNILib.setXZAngle(xz);
		    GL2JNILib.setYZAngle(yz);
	    }
    }


	@Override
	public void onDrawEye(Eye eye) {
		if (!running) {
			return;
		}

		synchronized (renderingLock) {
            GLES20.glEnable( GLES20.GL_DEPTH_TEST );

			if ( getStereoModeEnabled() ) {
				GL2JNILib.setEyeMatrix( eye.getEyeView() );
				GL2JNILib.setPerspectiveMatrix( eye.getPerspective(0.1f, 1000.0f ) );
			}

			GL2JNILib.step();
		}
	}

	@Override
	public void onFinishFrame(Viewport viewport) {
	}

	@Override
	public void onSurfaceChanged(int width, int height) {
		GL2JNILib.init(width, height);
	}

	@Override
	public void onSurfaceCreated(EGLConfig eglConfig) {
	}

	@Override
	public void onRendererShutdown() {
	}

	public interface GameRenderer {
		void fadeOut();
		void setNeedsUpdate();
		void cameraRotateLeft();
		void cameraRotateRight();
	}

	public static final int SPLAT_NONE = -1;
	public static final int ID_NO_ACTOR = 0;

	public enum KB {
		UP, RIGHT, DOWN, LEFT, ROTATE_LEFT, ROTATE_RIGHT
	}

	public enum ETextures {
		None,
		Grass,
		Bricks,
		Arch,
		Bars,
		Begin,
		Exit,
		BricksBlood,
		BricksCandles,
		Boss0,
		Boss1,
		Boss2,
		Cuco0,
		Cuco1,
		Cuco2,
		Demon0,
		Demon1,
		Demon2,
		Lady0,
		Lady1,
		Lady2,
		Crusader,
		Shadow,
		Ceiling,
		CeilingDoor,
		CeilingBegin,
		CeilingEnd,
		Splat0,
		Splat1,
		Splat2,
		CeilingBars,
		CornerLeftFar,
		CornerLeftNear,
		Skybox,
		CubeColours,
		CubeNormals,
	};

	GameRenderer gameRenderer = new GameRenderer() {

		@Override
		public void fadeOut() {
			GL2JNILib.fadeOut();
		}

		@Override
		public void setNeedsUpdate() {
			GameViewGLES2.this.setNeedsUpdate();
		}

		@Override
		public void cameraRotateLeft() {
			GL2JNILib.rotateLeft();

			--rotation;
		}

		@Override
		public void cameraRotateRight() {
			GL2JNILib.rotateRight();

			++rotation;
		}
	};


	View.OnKeyListener keyListener = new OnKeyListener() {
		@Override
		public synchronized boolean onKey(View v, int keyCode, KeyEvent event) {

			if ( event.getAction() == KeyEvent.ACTION_DOWN ) {

				if (keyCode == KeyEvent.KEYCODE_COMMA || keyCode == KeyEvent.KEYCODE_BUTTON_L1) {
					key = transformMovementToCameraRotation(GameViewGLES2.KB.LEFT);
				}

				if (keyCode == KeyEvent.KEYCODE_PERIOD || keyCode == KeyEvent.KEYCODE_BUTTON_R1) {
					key = transformMovementToCameraRotation(GameViewGLES2.KB.RIGHT);
				}

				if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {

					KB newValue = transformMovementToCameraRotation(GameViewGLES2.KB.UP);

					if( key == newValue ){
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
	private boolean needsUpdate = true;
	private volatile boolean running = true;
	private Vector2 cameraPosition;
	private long timeUntilTick;
	private long t0;
	GameViewGLES2.KB key = null;
    private float[] forwardVector = new float[3];

	//snapshot
	private final int[] map = new int[GameLevel.MAP_SIZE * GameLevel.MAP_SIZE];
	private final int[] ids = new int[GameLevel.MAP_SIZE * GameLevel.MAP_SIZE];
	private final int[] snapshot = new int[GameLevel.MAP_SIZE * GameLevel.MAP_SIZE];
	private final int[] splats = new int[GameLevel.MAP_SIZE * GameLevel.MAP_SIZE];
	private final Vector2 v = new Vector2();

	//game logic stuff - that shouldn't really be here.
	private GameLevel currentLevel;
	private GameActivity.GameDelegate gameDelegate;
	private GameSession gameSession;
	boolean mHaveController;
	private int currentLevelNumber;
	int rotation = 0;

	private long tick() {

		long delta = (System.currentTimeMillis() - t0);

		timeUntilTick -= delta;

		if (timeUntilTick < 0) {

			centerOn(currentLevel.getSelectedPlayer() );

			if ( key != null ) {
				handleCommand(key);
				key = null;
				setNeedsUpdate();
			}

			currentLevel.updateSplats(TICK_INTERVAL - timeUntilTick);
			needsUpdate = needsUpdate || currentLevel.needsUpdate() || ( key != null );
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

    public GameViewGLES2(Context context) {
        super(context);
        init();
    }


    public GameViewGLES2(Context context, AttributeSet attrs) {
		super(context, attrs);
        init();
	}

	private void updateNativeSnapshot() {
		int position;
			GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

		for (int y = 0; y < GameLevel.MAP_SIZE; ++y) {
			for (int x = 0; x < GameLevel.MAP_SIZE; ++x) {
				v.x = x;
				v.y = y;
				position = (y * GameLevel.MAP_SIZE) + x;

				Tile tile = this.currentLevel.getTile(v);

				ETextures index = tile.getTextureIndex();
				map[position] = tile.getMapTextureIndex().ordinal();
				splats[position] = SPLAT_NONE;
				ids[ position ] = ID_NO_ACTOR;
				snapshot[position] = ETextures.None.ordinal();

				Renderable occupant = tile.getOccupant();

				if (  occupant instanceof Actor) {
					ids[ position ] = ((Actor)occupant).mId;
					snapshot[position] = index.ordinal();
				}
			}
		}

		for ( Vector2 pos : currentLevel.mSplats.keySet() ) {
			Splat splat = currentLevel.mSplats.get(pos);

			position = (int) ((pos.y * GameLevel.MAP_SIZE) + pos.x);
			int frame = splat.getSplatFrame();

			if ( frame > splats[position ] ) {
				splats[position] = frame;
			}
		}
		GL2JNILib.setFloorNumber( currentLevelNumber );
		GL2JNILib.setMapWithSplatsAndActors(map, snapshot, splats);
		GL2JNILib.setActorIdPositions( ids );
		GL2JNILib.setCameraPosition(cameraPosition.x, cameraPosition.y);
	}

	public void init(Context context, GameActivity.GameDelegate delegate, int level, boolean haveController) {
		cameraPosition = new Vector2();
		mHaveController = haveController;
		this.gameSession = GameConfigurations.getInstance()
				.getCurrentGameSession();

		this.gameDelegate = delegate;
		buildPresentation(context.getResources(), level);
		this.currentLevelNumber = level;

		if ( haveController ) {
			setOnKeyListener( keyListener );
		}

		if ( this.getStereoModeEnabled() ) {
			setOnCardboardTriggerListener(new Runnable() {
				@Override
				public void run() {
					key = transformMovementToCameraRotation(GameViewGLES2.KB.UP);
				}
			});
		} else {
			setOnTouchListener( new OnSwipeTouchListener(getContext()){

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
					key = transformMovementToCameraRotation(GameViewGLES2.KB.UP);
				}

				@Override
				public void onSwipeDown() {
					super.onSwipeDown();
					key = transformMovementToCameraRotation(GameViewGLES2.KB.DOWN);
				}
			});
		}

		requestFocus();
		requestFocusFromTouch();
	}

	private void buildPresentation(Resources res, int level) {
		currentLevel = gameSession.obtainCurrentLevel(res, level, gameDelegate, gameRenderer);
	}

	public void centerOn(Actor actor) {
		if ( actor != null ) {
			cameraPosition = actor.getPosition();
		}
	}

	private synchronized void setNeedsUpdate() {
		needsUpdate = true;
	}

	public void fadeOut() {
		gameRenderer.fadeOut();
	}

	public void onDestroy() {
		synchronized (renderingLock) {
			GL2JNILib.onDestroy();
		}
	}

	public void onCreate(AssetManager assets) {
		synchronized ( renderingLock ) {
			GL2JNILib.onCreate(assets);
			loadTextures( assets );
		}
	}

	public void setTextures(Bitmap[] bitmaps) {
		GL2JNILib.setTextures(bitmaps);
	}

	public void loadTextures( AssetManager assets ) {
		try {
			Bitmap[] bitmaps;

			boolean isDungeonSurfaceLevel = this.currentLevelNumber > 0;

			bitmaps = loadBitmaps(assets, new String[]{
					"grass.png", //none
					(isDungeonSurfaceLevel ? "stonefloor.png" : "grass.png"),
					"bricks.png",
					"arch.png",
					"bars.png",
					"begin.png",
					"exit.png",
					"bricks_blood.png",
					"bricks_candles.png",
					"boss0.png",
					"boss1.png",
					"boss2.png",
					"cuco0.png",
					"cuco1.png",
					"cuco2.png",
					"demon0.png",
					"demon1.png",
					"demon2.png",
					"lady0.png",
					"lady1.png",
					"lady2.png",
					"grass.png", //crusader
					(isDungeonSurfaceLevel ? "stoneshadow.png" : "shadow.png"),
					(isDungeonSurfaceLevel ? "stoneceiling.png" : "ceiling.png"),
					"ceilingdoor.png",
					"ceilingbegin.png",
					"ceilingend.png",
					"splat0.png",
					"splat1.png",
					"splat2.png",
					"ceilingbars.png",
					"bricks.png",
					"bricks.png",
					"clouds.png",
					"monty.png",
					"cubenormals.png",
			});
			setTextures(bitmaps);
		} catch (IOException e) {
			e.printStackTrace();
			gameDelegate.onFatalError();
		}
	}

	private Bitmap[] loadBitmaps(AssetManager assets, String[] filenames) throws IOException {
		Bitmap[] toReturn = new Bitmap[filenames.length];

		for (int i = 0; i < filenames.length; i++) {
			toReturn[i] = BitmapFactory.decodeStream(assets.open(filenames[i]));
		}

		return toReturn;
	}

	public void stopRunning() {
		this.running = false;
	}

	@Override
	public void onResume() {
		super.onResume();
		setIsPlaying( true );
		setFocusable(true);
		requestFocus();

		setNeedsUpdate();
	}

	public void setIsPlaying(boolean isPlaying) {
		this.running = isPlaying;
	}

	public KB transformMovementToCameraRotation(KB direction) {

		int index = (rotation + direction.ordinal()) % GameLevel.Direction.values().length;
		while ( index < 0 ) {
			index += GameLevel.Direction.values().length;
		}

		return KB.values()[ index ];
	}

// game logic - that shouldn't really be here.

	public synchronized void handleCommand(final KB key) {

		if (!running) {
			return;
		}

		if (currentLevel.getSelectedPlayer() == null) {
			return;
		}

		((Activity)getContext()).runOnUiThread(new Runnable() {
			@Override
			public void run() {
				synchronized (renderingLock) {
					if ( !GL2JNILib.isAnimating() ) {
						currentLevel.handleCommand(key);
					}
				}
			}
		});
	}

	public GameLevel.Direction transformMovementToCameraRotation(GameLevel.Direction direction) {
		int indexDirection = direction.ordinal();
		KB directionKey = KB.values()[ indexDirection ];
		KB transformedKey =  transformMovementToCameraRotation(  directionKey );

		return GameLevel.Direction.values()[ transformedKey.ordinal() ];
	}

	public GameRenderer getRenderingDelegate() {
		return gameRenderer;
	}

    private float extractAngleXYFromHeadtransform(HeadTransform headTransform) {
        return 360.0f - ((float) (forwardVector[2] * (180 / Math.PI)));
    }

    private float extractAngleYZFromHeadtransform(HeadTransform headTransform) {
        return  ((float) (forwardVector[0] * (180 / Math.PI)));
    }

    private float extractAngleXZFromHeadtransform(HeadTransform headTransform) {
        return ((float) (forwardVector[1] * (180 / Math.PI)));
    }
}

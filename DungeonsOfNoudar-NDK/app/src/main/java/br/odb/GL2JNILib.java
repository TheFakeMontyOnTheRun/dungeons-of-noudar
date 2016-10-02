package br.odb;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

public class GL2JNILib {

	private static int currentId = 0;

	public static int getNextId() {
		currentId++;

		return currentId;
	}

	static {
		System.loadLibrary("NdkGlue");
	}

	public static native void init(int width, int height);

	public static native void step();

	public static native void onDestroy();

	public static native void onCreate(AssetManager assetManager);

	public static native void setTextures(Bitmap[] bitmaps);

	public static native void setMapWithSplatsAndActors( int[] map, int[] actors, int[] splats);

	public static native void setActorIdPositions( int[] ids);

	public static native void setCameraPosition(float x, float y);

	public static native void fadeOut();

	public static native void rotateLeft();

	public static native void rotateRight();

	public static native boolean isAnimating();

	public static native void setFloorNumber( long floor );

	public static native void tick( long delta );

	public static native void onReleasedLongPressingMove();

	public static native void onLongPressingMove();

	public static native void setEyeMatrix(float[] eyeView);

	public static native void setPerspectiveMatrix(float[] perspectiveMatrix) ;

	public static native void setXZAngle(float xz);

	public static native void setYZAngle(float yz);
}

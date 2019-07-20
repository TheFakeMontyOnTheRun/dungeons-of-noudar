package br.odb;

import android.content.res.AssetManager;

public class GL2JNILib {

    static {
        System.loadLibrary("NdkGlue");
    }

    public static native void init(int width, int height, AssetManager assets);

    public static native void onDestroy();

    public static native void onCreate(AssetManager assetManager);

    public static native void rotateLeft();

    public static native void rotateRight();

    public static native boolean isAnimating();

    public static native void tick(long delta);

    public static native void onReleasedLongPressingMove();

    public static native void onLongPressingMove();

    public static native void moveUp();

    public static native void moveDown();

    public static native void moveLeft();

    public static native void moveRight();

    public static native void cyclePreviousItem();

    public static native void cycleNextItem();

    public static native void dropItem();

    public static native void pickItem();

    public static native void useItem();

    public static native boolean isThereAnyObjectInFrontOfYou();

    public static native int getLevel();
}

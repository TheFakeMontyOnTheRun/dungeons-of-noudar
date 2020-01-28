package pt.b13h.noudar;

import android.content.res.AssetManager;

public class NoudarJNI {
    static {
        System.loadLibrary("native-lib");
    }

    public static native void initAssets(AssetManager assetManager);
    public static native void getPixelsFromNative(byte[] javaSide);
    public static native void sendCommand(char cmd);
    public static native int getSoundToPlay();

}

package pt.b13h.noudar

import android.content.res.AssetManager

object NoudarJNI {
    init {
        System.loadLibrary("native-lib")
    }

    external fun initAssets(assetManager: AssetManager?)
    external fun getPixelsFromNative(javaSide: ByteArray?)
    external fun sendCommand(cmd: Char)
    external fun getSoundToPlay(): Int
}

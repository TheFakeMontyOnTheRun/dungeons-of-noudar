package pt.b13h.noudar

import android.app.Application
import android.content.Context
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.media.AudioManager
import android.view.InputDevice
import java.util.*

class NoudarApplication : Application() {

    fun mayEnableSound(): Boolean {
        val am = getSystemService(Context.AUDIO_SERVICE) as AudioManager
        return when (am.ringerMode) {
            AudioManager.RINGER_MODE_NORMAL -> true
            AudioManager.RINGER_MODE_SILENT, AudioManager.RINGER_MODE_VIBRATE -> false
            else -> false
        }
    }

    private fun hasTouchscreen(): Boolean {
        return packageManager.hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN)
    }

    private fun hasPhysicalKeyboard(): Boolean {
        return resources.configuration.keyboard != Configuration.KEYBOARD_NOKEYS
    }


    private fun hasGamepad(): Boolean {
        for (deviceId in InputDevice.getDeviceIds()) {
            val dev = InputDevice.getDevice(deviceId)
            val sources = dev!!.sources
            if ((sources and InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD ||
                ((sources and InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)
            ) {
                return true
            }
        }
        return false
    }

    fun hasPhysicalController() : Boolean {
        return hasPhysicalKeyboard() || hasGamepad() || !hasTouchscreen()
    }
}
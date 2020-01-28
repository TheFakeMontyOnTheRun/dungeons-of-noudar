package pt.b13h.noudar

import android.content.Context
import android.content.res.Configuration
import android.graphics.Bitmap
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.SoundPool
import android.os.Build
import android.os.Bundle
import android.view.KeyEvent
import android.view.View
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity(), View.OnClickListener {

    private var soundPool: SoundPool? = null
    private var sounds = IntArray(7)
    private var pixels = ByteArray(320 * 240 * 4)
    val bitmap: Bitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888)
    private var running = false


    private fun initAudio() {
        soundPool = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            SoundPool.Builder().setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .build()
            ).build()
        } else {
            SoundPool(7, AudioManager.STREAM_MUSIC, 0)
        }


        sounds[0] = soundPool!!.load(this, R.raw.i114t1o8f, 1)
        sounds[1] = soundPool!!.load(this, R.raw.t200i101o3afo1a, 1)
        sounds[2] = soundPool!!.load(this, R.raw.t200i101o8ao4ao2ao1a, 1)
        sounds[3] = soundPool!!.load(this, R.raw.t200i52o4defg, 1)
        sounds[4] = soundPool!!.load(this, R.raw.t200i53o3fo1f, 1)
        sounds[5] = soundPool!!.load(this, R.raw.t200i98a, 1)
        sounds[6] = soundPool!!.load(this, R.raw.t200i9o1fa, 1)
    }


    private fun enterStickyImmersiveMode() {
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT >= 29) {
            enterStickyImmersiveMode()
        } else {
            window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN
        }

        setContentView(R.layout.activity_main)

        if (savedInstanceState == null ) {
            NoudarJNI.initAssets(resources.assets)
        }

        if ((application as NoudarApplication).mayEnableSound()) {
            initAudio()
        } else {
            soundPool = null
        }

        imageView.setImageBitmap(bitmap)
    }

    override fun onDestroy() {

        soundPool?.release()
        super.onDestroy()
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {

        var toSend = '.'
        when (keyCode) {
            KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_W -> toSend = 'w'
            KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_S -> toSend = 's'
            KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_Q -> toSend = 'q'
            KeyEvent.KEYCODE_DPAD_RIGHT, KeyEvent.KEYCODE_E -> toSend = 'e'

            KeyEvent.KEYCODE_BUTTON_L1, KeyEvent.KEYCODE_A -> toSend = 'a'
            KeyEvent.KEYCODE_BUTTON_R1, KeyEvent.KEYCODE_D -> toSend = 'd'

            KeyEvent.KEYCODE_BUTTON_A, KeyEvent.KEYCODE_Z -> toSend = 'z'
            KeyEvent.KEYCODE_BUTTON_B, KeyEvent.KEYCODE_X-> toSend = 'x'
            KeyEvent.KEYCODE_BUTTON_C, KeyEvent.KEYCODE_BUTTON_Y, KeyEvent.KEYCODE_C-> toSend = 'c'
            KeyEvent.KEYCODE_BUTTON_START, KeyEvent.KEYCODE_BUTTON_X, KeyEvent.KEYCODE_ENTER -> toSend = '\n'
            else -> return super.onKeyUp(keyCode, event )
        }
        NoudarJNI.sendCommand(toSend)
        return true
    }


    override fun onResume() {
        super.onResume()
        running = true

        Thread( Runnable {
            while (running) {
                runOnUiThread {
                    if  ( !(application as NoudarApplication).hasPhysicalController() ) {

                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            llActions.visibility = View.VISIBLE
                            llDirections.visibility = View.VISIBLE
                        } else {
                            llScreenControllers.visibility = View.VISIBLE
                        }

                        btnUp.setOnClickListener(this)
                        btnDown.setOnClickListener(this)
                        btnFire.setOnClickListener(this)
                        btnPick.setOnClickListener(this)
                        btnAim.setOnClickListener(this)
                        btnLeft.setOnClickListener(this)
                        btnRight.setOnClickListener(this)
                        btnStart.setOnClickListener(this)
                    } else {
                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            llActions.visibility = View.GONE
                            llDirections.visibility = View.GONE
                        } else {
                            llScreenControllers.visibility = View.GONE
                        }
                    }
                }

                Thread.sleep(1000);
            }
        }).start()

        Thread(Runnable {
            while (running) {
                runOnUiThread { redraw() }
                Thread.sleep(75)
            }
        }
        ).start()

        if (soundPool != null) {
            Thread(Runnable {
                while (running) {
                    when (val sound = NoudarJNI.getSoundToPlay()) {
                        0, 1, 2, 3, 4, 5, 6, 7, 8 -> soundPool!!.play(
                            sounds[sound],
                            1f,
                            1f,
                            0,
                            0,
                            1f
                        )
                    }
                    Thread.sleep(10)
                }
            }
            ).start()
        }
    }

    override fun onPause() {
        super.onPause()
        running = false
    }

    private fun redraw() {
        NoudarJNI.getPixelsFromNative(pixels)
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(pixels))
        imageView.invalidate()
    }

    override fun onClick(v: View) {
        var toSend = '.'
        when (v.id) {
            R.id.btnUp -> toSend = 'w'
            R.id.btnDown -> toSend = 's'
            R.id.btnLeft -> toSend = 'q'
            R.id.btnRight -> toSend = 'e'
            R.id.btnFire -> toSend = 'z'
            R.id.btnAim -> toSend = 'x'
            R.id.btnPick -> toSend = 'c'
            R.id.btnStart -> toSend = '\n'
        }
        NoudarJNI.sendCommand(toSend)
    }
}
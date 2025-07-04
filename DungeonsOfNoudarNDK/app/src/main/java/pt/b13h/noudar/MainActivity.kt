package pt.b13h.noudar


import android.app.Presentation
import android.content.Context
import android.content.res.Configuration
import android.graphics.Bitmap
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.MediaRouter
import android.media.SoundPool
import android.os.Build
import android.os.Bundle
import android.view.Display
import android.view.KeyEvent
import android.view.View
import android.view.ViewManager
import android.widget.ImageButton
import android.widget.ImageView
import android.widget.LinearLayout
import androidx.appcompat.app.AppCompatActivity
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity(), View.OnClickListener {

    private var presentation: Presentation? = null
    private var soundPool: SoundPool? = null
    private var sounds = IntArray(7)
    private var pixels = ByteArray(320 * 240 * 4)
    val bitmap: Bitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888)
    private var running = false

    private fun initAudio() {
        soundPool =
            SoundPool.Builder().setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .build()
            ).build()


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

        if (savedInstanceState == null) {
            NoudarJNI.initAssets(resources.assets)
        }

        if ((application as NoudarApplication).mayEnableSound()) {
            initAudio()
        } else {
            soundPool = null
        }
        val imageView = findViewById<ImageView>(R.id.imageView)
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
            KeyEvent.KEYCODE_BUTTON_B, KeyEvent.KEYCODE_X -> toSend = 'x'
            KeyEvent.KEYCODE_BUTTON_C, KeyEvent.KEYCODE_BUTTON_Y, KeyEvent.KEYCODE_C -> toSend = 'c'
            KeyEvent.KEYCODE_BUTTON_START, KeyEvent.KEYCODE_BUTTON_X, KeyEvent.KEYCODE_ENTER -> toSend =
                '\n'

            else -> return super.onKeyUp(keyCode, event)
        }
        NoudarJNI.sendCommand(toSend)
        return true
    }


    override fun onPostResume() {
        super.onPostResume()

        useBestRouteForGameplayPresentation()

        running = true

        Thread {
            while (running) {
                runOnUiThread {
                    if (!(application as NoudarApplication).hasPhysicalController()) {
                        val llActions = findViewById<LinearLayout>(R.id.llActions)
                        val llDirections = findViewById<LinearLayout>(R.id.llDirections)
                        val llScreenControllers =
                            findViewById<LinearLayout>(R.id.llScreenControllers)
                        val btnUp = findViewById<ImageButton>(R.id.btnUp)
                        val btnDown = findViewById<ImageButton>(R.id.btnDown)
                        val btnFire = findViewById<ImageButton>(R.id.btnFire)
                        val btnAim = findViewById<ImageButton>(R.id.btnAim)
                        val btnLeft = findViewById<ImageButton>(R.id.btnLeft)
                        val btnRight = findViewById<ImageButton>(R.id.btnRight)
                        val btnStrafeLeft = findViewById<ImageButton>(R.id.btnStrafeLeft)
                        val btnStrafeRight = findViewById<ImageButton>(R.id.btnStrafeRight)
                        val btnPick = findViewById<ImageButton>(R.id.btnPick)
                        val btnStart = findViewById<ImageButton>(R.id.btnStart)

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

                        btnStrafeLeft.setOnClickListener(this)
                        btnStrafeRight.setOnClickListener(this)

                    } else {
                        val llActions = findViewById<LinearLayout>(R.id.llActions)
                        val llDirections = findViewById<LinearLayout>(R.id.llDirections)
                        val llScreenControllers =
                            findViewById<LinearLayout>(R.id.llScreenControllers)

                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            llActions.visibility = View.GONE
                            llDirections.visibility = View.GONE
                        } else {
                            llScreenControllers.visibility = View.GONE
                        }
                    }
                }

                val route = findSecundaryDisplayRouter()
                if ((this@MainActivity).presentation != null && (route == null || route.presentationDisplay == null)) {
                    presentation = null
                    runOnUiThread { (this@MainActivity).recreate() }
                }

                Thread.sleep(1000)
            }
        }.start()

        Thread {
            while (running) {
                runOnUiThread { redraw() }
                Thread.sleep(75)
            }
        }.start()

        if (soundPool != null) {
            Thread {
                while (running) {
                    when (val sound = NoudarJNI.getSoundToPlay()) {
                        0, 1, 2, 3, 4, 5, 6 -> soundPool!!.play(sounds[sound], 1f, 1f, 0, 0, 1f)
                    }
                    Thread.sleep(10)
                }
            }.start()
        }
    }

    override fun onPause() {
        super.onPause()
        running = false
        presentation?.hide()
        presentation?.dismiss()
        presentation?.cancel()

    }

    private fun redraw() {
        val imageView = findViewById<ImageView>(R.id.imageView)
        NoudarJNI.getPixelsFromNative(pixels)
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(pixels))
        imageView.invalidate()
    }

    override fun onClick(v: View) {
        var toSend = '.'
        when (v.id) {
            R.id.btnStrafeLeft -> toSend = 'a'
            R.id.btnStrafeRight -> toSend = 'd'
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

    private fun findSecundaryDisplayRouter(): MediaRouter.RouteInfo? {
        val mMediaRouter =
            getSystemService(Context.MEDIA_ROUTER_SERVICE) as MediaRouter
        return mMediaRouter.getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_VIDEO)
    }

    private fun useBestRouteForGameplayPresentation() {
        val mRouteInfo = findSecundaryDisplayRouter()
        if (mRouteInfo != null) {
            val presentationDisplay = mRouteInfo.presentationDisplay
            if (presentationDisplay != null) {
                useSecundaryDisplayForGameplayPresentation(presentationDisplay)
                Thread.sleep(1000)
            }
        }
    }


    private fun useSecundaryDisplayForGameplayPresentation(presentationDisplay: Display) {
        val imageView = findViewById<ImageView>(R.id.imageView)
        (imageView.parent as ViewManager).removeView(imageView)
        presentation =
            GamePresentation(this, presentationDisplay, imageView)
        presentation?.show()
    }
}
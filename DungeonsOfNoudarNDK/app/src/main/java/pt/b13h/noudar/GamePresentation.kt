package pt.b13h.noudar

import android.app.Presentation
import android.content.Context
import android.os.Bundle
import android.view.Display
import android.widget.ImageView

class GamePresentation(context: Context?, display: Display?, private val canvas: ImageView) :
    Presentation(context, display) {
    override fun onCreate(savedInstanceState: Bundle) {
        super.onCreate(savedInstanceState)
        setContentView(canvas)
    }
}

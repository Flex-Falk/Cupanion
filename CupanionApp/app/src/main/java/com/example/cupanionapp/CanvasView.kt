package com.example.cupanionapp

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.View
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData

class CanvasView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    private lateinit var userDataViewModel: UserData

    private val _canvasView = MutableLiveData<CanvasView>()
    val canvasView: LiveData<CanvasView> get() = _canvasView

    private var state: String? = null

    // Function to draw lines and names on our filling_quantity_screen.
    private fun createNameAndLine(canvas: Canvas?, nameOfDrink: String, lineHeight: Float, inputColor: Int){
        val paint = Paint()
        paint.strokeWidth = 10f
        paint.textSize = 100f
        paint.color = inputColor

        canvas?.drawText(nameOfDrink, 0f,lineHeight-20, paint)
        canvas?.drawLine(0f,lineHeight,2000f,lineHeight, paint)
    }

    // Function which determines which drink is shown on our filling_quantity_screen.
    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)
        // Depending on our currently selected drink
        // 0f is in the direction of the top edge of our screen
        // 1500f is in direction of the bottom edge of our screen
        if(state == "Spezi mit Schuss"){
            createNameAndLine(canvas,"Rum", 200f, Color.BLACK)
            createNameAndLine(canvas,"Fanta", 600f, Color.BLACK)
            createNameAndLine(canvas,"Cola", 1200f, Color.BLACK)
        }
        if(state == "Caipirinha"){
            createNameAndLine(canvas,"Limette", 300f, Color.BLACK)
            createNameAndLine(canvas,"Cachaca", 500f, Color.BLACK)
            createNameAndLine(canvas,"Rohrzucker", 850f, Color.RED)
            createNameAndLine(canvas,"Eis", 1000f, Color.BLACK)
        }
        if(state == "Spezi"){
            createNameAndLine(canvas,"Fanta", 600f, Color.BLACK)
            createNameAndLine(canvas,"Cola", 1200f, Color.BLACK)
        }
        if(state == "Wasser mit Schuss"){
            createNameAndLine(canvas,"Wasser", 400f, Color.BLACK)
            createNameAndLine(canvas,"Schuss", 1200f, Color.BLACK)
        }
    }

    fun setStateCurrentDrink(s:String?){
        state = s
        _canvasView.value = this
    }
}
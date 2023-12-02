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

    //measured height when filling 0 ml
    private val lowestPointInGlas = 1570f

    private var latestIngredientFillingHeight = lowestPointInGlas

    // 50ml at 970px, 100ml at 1270px -> (1270px-970px)/(100ml-50ml)
    private val posOf100ml = 1270
    private val posOf50ml = 970
    private val ml2PixelRatio = (posOf100ml-posOf50ml)/(100-50)

    // Function returns and updates height of given ingredient volume
    private fun calculateAndUpdateHeightWhenAdding(amountML: Int):Float{
        val addedHeight = amountML * ml2PixelRatio
        latestIngredientFillingHeight -= addedHeight
        return latestIngredientFillingHeight
    }


    // Function to draw lines and names on our filling_quantity_screen.
    private fun createNameAndLine(canvas: Canvas?, nameOfDrink: String, lineHeight: Float, inputColor: Int){
        val paint = Paint()
        paint.strokeWidth = 10f
        paint.textSize = 100f
        paint.color = inputColor

        canvas?.drawText(nameOfDrink, 0f,lineHeight+90, paint)
        canvas?.drawLine(0f,lineHeight,2000f,lineHeight, paint)
    }

    // Function to draw lines and names on our filling_quantity_screen.
    private fun createNameAndLineML(canvas: Canvas?, nameOfDrink: String, volumeML: Int, inputColor: Int){
        val paint = Paint()
        paint.strokeWidth = 10f
        paint.textSize = 100f
        paint.color = inputColor
        val heightWhenAdding = calculateAndUpdateHeightWhenAdding(volumeML)

        canvas?.drawText(nameOfDrink, 0f,heightWhenAdding+90, paint)
        canvas?.drawLine(0f,heightWhenAdding,2000f,heightWhenAdding, paint)
    }

    // Function which determines which drink is shown on our filling_quantity_screen.
    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)
        // Depending on our currently selected drink
        if(state == "Spezi mit Schuss"){
            createNameAndLineML(canvas,"Rum", 30, Color.BLACK)
            createNameAndLineML(canvas,"Fanta", 100, Color.BLACK)
            createNameAndLineML(canvas,"Cola", 100, Color.BLACK)
        }
        if(state == "Caipirinha"){
            createNameAndLineML(canvas,"Rohrzucker", 20, Color.BLACK)
            createNameAndLineML(canvas,"Crushed Ice", 70, Color.BLACK)
            createNameAndLineML(canvas,"Limette", 20, Color.BLACK)
            createNameAndLineML(canvas,"Cachaca", 80, Color.BLACK)
        }
        if(state == "Wasser mit Schuss"){
            createNameAndLineML(canvas,"Wasser", 150, Color.BLACK)
            createNameAndLineML(canvas,"Schuss", 75, Color.BLACK)
        }
        if(state == "Apfel Hochball"){
            createNameAndLineML(canvas,"Apfelschnaps", 120, Color.BLACK)
            createNameAndLineML(canvas,"Cognac", 120, Color.BLACK)
        }
        if(state == "Großmutters Spezi"){
            createNameAndLineML(canvas,"Fanta", 120, Color.BLACK)
            createNameAndLineML(canvas,"Cola", 120, Color.BLACK)
        }
        if(state == "Leckeres Kalibrierungsgetränk"){
            createNameAndLine(canvas,"0ml - Kalibrierung", lowestPointInGlas, Color.BLUE)
            createNameAndLine(canvas,"50ml - Kalibrierung", lowestPointInGlas - ml2PixelRatio*50, Color.BLUE)
            createNameAndLine(canvas,"100ml - Kalibrierung", lowestPointInGlas - ml2PixelRatio*100, Color.BLUE)
        }
    }

    fun setStateCurrentDrink(s:String?){
        state = s
        _canvasView.value = this
    }
}
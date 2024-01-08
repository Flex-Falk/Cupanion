package com.example.cupanionapp

import android.content.Context
import android.util.Log
import android.view.View
import android.widget.Toast
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.androidnetworking.AndroidNetworking.get
import com.androidnetworking.AndroidNetworking.post
import com.androidnetworking.common.Priority
import com.androidnetworking.error.ANError
import com.androidnetworking.interfaces.JSONObjectRequestListener
import com.google.android.material.snackbar.Snackbar
import org.json.JSONObject

/**
 * View Model for the UserData.
 */
class UserData : ViewModel() {
    private val _userData = MutableLiveData<UserData?>()
    val userData: LiveData<UserData?> get() = _userData

    // This represents the username.
    var user_name: String? = ""

    // This represents the drinking goal.
    var user_goal: Int? = 0

    // This represents the drink which is currently being mixed.
    var user_current_drink: String? = ""

    // This list represents the drinks the user has mixed in raw form.
    var user_drinks_list: MutableList<String> = mutableListOf()

    // This list represents the drinks the user has mixed in a printable String form.
    var user_drinks_list_formatted: String? = ""

    // This represents the amount of drinks the user has mixed.
    var user_drinks_number: Int? = 0

    // This shows if the user should drive or not.
    var user_drive: Boolean? = true


    // This shows how many toasts the user has done (NFC functionality required).
    private val _userToasts = MutableLiveData<Int>(0)
    var user_toasts: LiveData<Int> = MutableLiveData<Int>().apply { value = _userToasts.value }
        get() = _userToasts

    // Function to update current drink
    fun updateCurrentDrink(drink: String) {
        user_current_drink = drink

        // Notify observers with the updated user data
        _userData.postValue(this)
    }

    // Function to update user data
    fun updateUserData(name: String, goal: Int) {
        user_name = name
        user_goal = goal

        // Notify observers with the updated user data
        _userData.postValue(this)
    }

    // returns true if a user drunk an drink of alcoholicBeverages list
    fun isDrivingAbilityRestricted(alcoholicBeverages: MutableList<String>, userDrinksList: MutableList<String>): Boolean{
        return userDrinksList.any { element -> alcoholicBeverages.contains(element) }
    }

    // Function for what happens when the mixing of a drink is concluded
    fun finishedMixing(){
        // Append a new drink to the drink list.
        // Check if current_drink is not null before adding.
        user_current_drink?.let {
            user_drinks_list.add(it)
        }

        //update the DrinkList String
        drinkListToString()

        // Update the drink number
        user_drinks_number = user_drinks_list.size

        //list of alcoholic beverages
        val alcoholicBeverages: MutableList<String> = mutableListOf("Spezi mit Schuss", "Caipirinha", "Wasser mit Schuss", "Apfel Hochball")
        // Update if the user should drive or not.
        if (isDrivingAbilityRestricted(alcoholicBeverages, user_drinks_list)) {
            user_drive = false
        }
    }

    // Displays the consumed drinks in a better format
    fun drinkListToString() {
        val drinkCountMap = mutableMapOf<String, Int>()

        // Count occurrences of each drink
        for (drink in user_drinks_list) {
            drinkCountMap[drink] = drinkCountMap.getOrDefault(drink, 0) + 1
        }

        // Generate the formatted string and update the UserDrinksListFormatted
        user_drinks_list_formatted = "\n" + drinkCountMap.entries.joinToString("\n") { "${it.value}x ${it.key}" }
    }

    // Sends the UserData to the ESP32
    fun sendUserData() {
        val userdataForESP = JSONObject().apply {
            put("user_name", user_name)
            put("user_goal", user_goal)
            put("user_current_drink", user_current_drink)
            put("user_drinks_number", user_drinks_number)
            put("user_drinks_list", user_drinks_list_formatted)
            put("user_drive", user_drive)
        }

        post("http://192.168.4.1/post")
            .addJSONObjectBody(userdataForESP)
            .addHeaders("toll", "toll-str")  // Specify the parameter name here
            .setContentType("application/json")
            .setPriority(Priority.MEDIUM)
            .build()
            .getAsJSONObject(object : JSONObjectRequestListener {
                override fun onResponse(response: JSONObject) {
                    Log.d("UserData", "Sent UserData")
                }

                override fun onError(anError: ANError) {
                    Log.e("UserData", "Error sending UserData: $anError")
                }
            })
    }
    // Function to update the toast value gotten from the ESP32
    // Should be called every 5 seconds to be up to date
    fun updateToastValue(view: View) {

        get("http://192.168.4.1/nfc-get")
            .setPriority(Priority.MEDIUM)
            .build()
            .getAsJSONObject(object : JSONObjectRequestListener {

                override fun onResponse(response: JSONObject) {
                    Log.d("NFC", response.toString())

                    val tagName = response.getString("name")
                    val tagToasts_str = response.getString("toasts")
                    var tagToasts = -1
                    try {
                        tagToasts = tagToasts_str.toInt()
                    } catch (nfe: NumberFormatException) {
                        Log.d("NFC", "Could not parse, NumberFormatException")
                    }

                    if(tagToasts != (user_toasts.value ?: 0)){
                        Log.d("NFC", "read name from tag: $tagName")
                        var snackbarMessage = ""
                        if(tagName != ""){
                            snackbarMessage  = "Angestoßen mit $tagName! \nDas ist dein $tagToasts. Anstoßer!"
                        } else {
                            snackbarMessage  = "Das ist dein $tagToasts. Anstoßer!"
                        }
                        //snackbarMessage  = "Das ist dein $tagToasts. Anstoßer!"
                        Snackbar.make(view, snackbarMessage, Snackbar.LENGTH_INDEFINITE)
                            .setAction("Cool!") { }
                            .show()
                        _userToasts.postValue(tagToasts)
                    }
                }
                override fun onError(anError: ANError) {
                    Log.e("NFC", "Error updating Toast Value: $anError")
                }
            })
    }
}
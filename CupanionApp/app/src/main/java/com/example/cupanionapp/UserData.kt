package com.example.cupanionapp

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.androidnetworking.AndroidNetworking
import com.androidnetworking.AndroidNetworking.post
import com.androidnetworking.common.Priority
import com.androidnetworking.error.ANError
import com.androidnetworking.interfaces.JSONObjectRequestListener
import org.json.JSONObject
import com.google.gson.Gson


/**
 * View Model for the UserData.
 */
class UserData : ViewModel() {
    private val _userData = MutableLiveData<UserData>()
    val userData: LiveData<UserData> get() = _userData

    // This represents the username.
    var user_name: String? = null

    // This represents the drinking goal.
    var user_goal: Int? = null

    // This represents the drink which is currently being mixed.
    var current_drink: String? = null

    // This list represents the drinks the user has mixed.
    var user_drinks_list: MutableList<String> = mutableListOf()

    // This represents the amount of drinks the user has mixed.
    var user_drinks_number: Int? = 0

    // This shows if the user should drive or not.
    var user_drive: Boolean? = true


    // This shows how many toasts the user has done (NFC functionality required).
    var user_toasts: Int? = 0

    // Function to update current drink
    fun updateCurrentDrink(drink: String) {
        current_drink = drink

        // Notify observers with the updated user data
        _userData.value = this
    }

    // Function to update user data
    fun updateUserData(name: String, goal: Int) {
        user_name = name
        user_goal = goal

        // Notify observers with the updated user data
        _userData.value = this
    }

    fun finishedMixing(){
        // Append a new drink to the drink list..
        // Check if current_drink is not null before adding.
        current_drink?.let {
            user_drinks_list.add(it)
        }

        // Update the drink number
        user_drinks_number = user_drinks_list.size

        // Update if the user should drive or not.
        if (user_drinks_number != 0) {
            user_drive = false
        }
    }

    //sends the UserData to the ESP32
    fun sendUserData(){
        //format the data into a JSONObject
        val userdata_for_esp = JSONObject()
        userdata_for_esp.put("user_name", user_name)
        userdata_for_esp.put("user_goal", user_goal)
        userdata_for_esp.put("current_drink", current_drink)
        userdata_for_esp.put("user_drinks_number", user_drinks_number)
        userdata_for_esp.put("user_drinks_list", user_drinks_list)
        userdata_for_esp.put("user_drive", user_drive)
        userdata_for_esp.put("user_toasts", user_toasts)

        // Send data via HTTP POST
        post("http://192.168.4.1/post")
            .addJSONObjectBody(userdata_for_esp) // Use addJSONObjectBody to add JSON data
            .setTag("data")
            .setPriority(Priority.MEDIUM)
            .build()
            .getAsJSONObject(object : JSONObjectRequestListener {
                override fun onResponse(response: JSONObject?) {
                }
                override fun onError(anError: ANError?) {
                }
            })
    }
}
package com.example.cupanionapp

import android.os.Bundle
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.fragment.findNavController
import com.example.cupanionapp.databinding.DataDisplayScreenBinding
import com.google.android.material.snackbar.Snackbar

/**
 * Screen where the Data of the user should be displayed.
 */
class DataDisplayScreen : Fragment() {

    private var _binding: DataDisplayScreenBinding? = null
    private lateinit var userDataViewModel: UserData

    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        _binding = DataDisplayScreenBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Access the ViewModel to get user data
        userDataViewModel = ViewModelProvider(requireActivity()).get(UserData::class.java)

        // Sends UserData to the ESP32
        userDataViewModel.sendUserData()

        // Update the Toast Value from the ESP32
        userDataViewModel.updateToastValue()

        // Display user data.
        binding.textViewUserName.text = "Name: ${userDataViewModel.user_name}"
        binding.textViewUserToasts.text = "Mit Leuten angestoßen: ${userDataViewModel.user_toasts}"

        // Display the drinking progress.
        binding.textViewUserProgress.text = "Trinkfortschritt: ${userDataViewModel.user_drinks_number.toString()} / ${userDataViewModel.user_goal.toString()}"

        // Display if the user should drive or not.
        if(userDataViewModel.user_drive == false){
            binding.textViewUserDrive.text = "Fahrtüchtigkeit: Nicht mehr in Ordnung"
        } else{
            binding.textViewUserDrive.text = "Fahrtüchtigkeit: In Ordnung"
        }

        // Display what drinks the user has drunk.
        if(userDataViewModel.user_drinks_number != 0) {
            binding.textViewUserDrinksList.text = "Getrunkene Getränke: ${userDataViewModel.user_drinks_list?.joinToString(", ")}"
        } else {
            binding.textViewUserDrinksList.text ="Bisherige Getränke: noch keine"
        }

        binding.buttonToDrinkSelectionScreen.setOnClickListener {
            findNavController().navigate(R.id.action_toDrinkSelectionScreen)
        }
    }
}
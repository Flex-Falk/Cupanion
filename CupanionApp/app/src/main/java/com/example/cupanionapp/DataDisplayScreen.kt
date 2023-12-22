package com.example.cupanionapp

import android.graphics.Color
import android.os.Bundle
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
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

        // Display user data.
        binding.textViewUserName.text = "Name: ${userDataViewModel.user_name}"
        userDataViewModel.user_toasts.observe(viewLifecycleOwner) { toasts ->
            binding.textViewUserToasts.text = "Mit Leuten angestoßen: ${toasts ?: "0"}"
        }
        // Display the drinking progress.
        if(userDataViewModel.user_drinks_number!! > userDataViewModel.user_goal!!){
            binding.textViewUserProgress.text = "Trinkfortschritt: ${userDataViewModel.user_drinks_number.toString()} / ${userDataViewModel.user_goal.toString()}"
            binding.textViewUserProgress.setTextColor(Color.RED)
        } else if (userDataViewModel.user_drinks_number!! == userDataViewModel.user_goal!!){
            binding.textViewUserProgress.text = "Trinkfortschritt: ${userDataViewModel.user_drinks_number.toString()} / ${userDataViewModel.user_goal.toString()}"
            binding.textViewUserProgress.setTextColor(Color.GREEN)
        } else{
            binding.textViewUserProgress.text = "Trinkfortschritt: ${userDataViewModel.user_drinks_number.toString()} / ${userDataViewModel.user_goal.toString()}"
        }

        // Display if the user should drive or not.
        if(userDataViewModel.user_drive == false){
            binding.textViewUserDrive.text = "Fahrtüchtigkeit: "
            binding.textViewUserDrive2.text = "nicht i.O."
            binding.textViewUserDrive2.setTextColor(Color.RED)
        } else{
            binding.textViewUserDrive.text = "Fahrtüchtigkeit: "
            binding.textViewUserDrive2.text = "In Ordnung"
            binding.textViewUserDrive2.setTextColor(Color.GREEN)
        }

        // Display what drinks the user has drunk.
        if(userDataViewModel.user_drinks_number != 0) {
            binding.textViewUserDrinksList.text = "Getrunkene Getränke:${userDataViewModel.user_drinks_list_formatted}"
        } else {
            binding.textViewUserDrinksList.text ="Bisherige Getränke: noch keine"
        }

        // Snackbar when user has reached given target
        if(userDataViewModel.user_drinks_number!! >= userDataViewModel.user_goal!!){
            Snackbar.make(view, "Trinkziel erreicht!", Snackbar.LENGTH_INDEFINITE)
                .setAction("Endlich!") { }
                .show()
        }

        if(userDataViewModel.user_drinks_number!! > userDataViewModel.user_goal!!){
            Snackbar.make(view, "Trinkziel überschritten!", Snackbar.LENGTH_INDEFINITE)
                .setAction("Alles gut bei dir?") { }
                .show()
        }

        binding.buttonToDrinkSelectionScreen.setOnClickListener {
            findNavController().navigate(R.id.action_toDrinkSelectionScreen)
        }
    }
}
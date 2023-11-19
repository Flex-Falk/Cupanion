package com.example.cupanionapp

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.fragment.findNavController
import com.example.cupanionapp.databinding.FillingQuantityScreenBinding

/**
 * Screen where the filling quantities of the selected drink should be displayed.
 */
class FillingQuantityScreen : Fragment() {

    private var _binding: FillingQuantityScreenBinding? = null
    private lateinit var userDataViewModel: UserData

    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        _binding = FillingQuantityScreenBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        userDataViewModel = ViewModelProvider(requireActivity()).get(UserData::class.java)

        // Sends UserData to the ESP32
        userDataViewModel.sendUserData()

        // Display the current drink being mixed.
        binding.textCurrentDrink.text = "Aktuell wird gemixt: ${userDataViewModel.current_drink}"

        binding.buttonToDrinkSelectionScreen.setOnClickListener {
            userDataViewModel.finishedMixing()      // Update all drink related data when finished mixing.
            findNavController().navigate(R.id.action_toDataDisplayScreen)
        }

        binding.drinkcanvas.setStateCurrentDrink(userDataViewModel.current_drink)
    }
}
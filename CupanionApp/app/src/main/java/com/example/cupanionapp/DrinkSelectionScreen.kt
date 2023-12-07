package com.example.cupanionapp

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.fragment.findNavController
import com.example.cupanionapp.databinding.DrinkSelectionScreenBinding
import com.google.android.material.snackbar.Snackbar

/**
 * Screen where a drink should be selected.
 */
class DrinkSelectionScreen : Fragment() {

    private var _binding: DrinkSelectionScreenBinding? = null
    private lateinit var userDataViewModel: UserData
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        _binding = DrinkSelectionScreenBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        userDataViewModel = ViewModelProvider(requireActivity()).get(UserData::class.java)

        // Sends UserData to the ESP32
        userDataViewModel.sendUserData()

        // Update the Toast Value from the ESP32
        userDataViewModel.updateToastValue()

        // Set click listeners for drink buttons and update the current drink
        binding.buttonSpeziSchuss.setOnClickListener {
            userDataViewModel.updateCurrentDrink("Spezi mit Schuss")
            findNavController().navigate(R.id.action_toFillingQuantityScreen)
        }
        binding.buttonCaipirinha.setOnClickListener {
            userDataViewModel.updateCurrentDrink("Caipirinha")
            findNavController().navigate(R.id.action_toFillingQuantityScreen)
        }
        binding.buttonWasserSchuss.setOnClickListener {
            userDataViewModel.updateCurrentDrink("Wasser mit Schuss")
            findNavController().navigate(R.id.action_toFillingQuantityScreen)
        }
        binding.buttonApfelHochball.setOnClickListener {
            userDataViewModel.updateCurrentDrink("Apfel Hochball")
            findNavController().navigate(R.id.action_toFillingQuantityScreen)
        }
        binding.buttonGrossmuttersSpezi.setOnClickListener {
            userDataViewModel.updateCurrentDrink("Großmutters Spezi")
            findNavController().navigate(R.id.action_toFillingQuantityScreen)
        }
        binding.buttonLeckeresKalibrierungsgetraenk.setOnClickListener {
            userDataViewModel.updateCurrentDrink("Leckeres Kalibrierungs-Drink")
            findNavController().navigate(R.id.action_toFillingQuantityScreen)
        }
    }
}
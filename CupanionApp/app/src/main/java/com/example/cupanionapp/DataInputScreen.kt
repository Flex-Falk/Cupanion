package com.example.cupanionapp

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.fragment.findNavController
import com.example.cupanionapp.databinding.DataInputScreenBinding

/**
 * Screen where the user data should be put in.
 */
class DataInputScreen : Fragment() {

    private var _binding: DataInputScreenBinding? = null
    private lateinit var userDataViewModel: UserData

    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        _binding = DataInputScreenBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        userDataViewModel = ViewModelProvider(requireActivity()).get(UserData::class.java)

        // Sends UserData to the ESP32
        userDataViewModel.sendUserData()

        // Update the Toast Value from the ESP32
        userDataViewModel.updateToastValue()

        // Observe changes in the entire user data
        userDataViewModel.userData.observe(viewLifecycleOwner) { userData ->
            // Update UI with user data
            binding.editTextUserName.setText(userData.user_name)
            binding.editTextUserGoal.setText(userData.user_goal?.toString())
        }

        binding.buttonToDataDisplayScreen.setOnClickListener {
            // Get data from UI
            val name = binding.editTextUserName.text.toString()
            val goal = binding.editTextUserGoal.text.toString()

            // Validate that the entries are not empty
            if (name.isNotEmpty() && goal.isNotEmpty()) {

                // Update user data in the ViewModel
                userDataViewModel.updateUserData(name, goal.toInt())

                // Navigate to DataDisplayScreen
                findNavController().navigate(R.id.action_toDataDisplayScreen)
            } else {
                // Show an error message when the at least one entry is empty.
                Toast.makeText(requireContext(), "Bitte alle Felder füllen.", Toast.LENGTH_SHORT)
                    .show()
            }
        }
    }
}
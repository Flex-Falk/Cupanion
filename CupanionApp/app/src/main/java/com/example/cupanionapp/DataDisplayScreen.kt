package com.example.cupanionapp

import android.os.Bundle
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.navigation.fragment.findNavController
import com.example.cupanionapp.databinding.DataDisplayScreenBinding
import com.google.android.material.snackbar.Snackbar

/**
 * Screen where the Data of the user should be displayed.
 */
class DataDisplayScreen : Fragment() {

    private var _binding: DataDisplayScreenBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
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
        binding.buttonToDrinkSelectionScreen.setOnClickListener {
            Log.d("DataDisplayScreen", "Button Clicked")
            findNavController().navigate(R.id.action_toDrinkSelectionScreen)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
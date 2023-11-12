package com.example.cupanionapp

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.navigation.fragment.findNavController
import com.example.cupanionapp.databinding.FillingQuantityScreenBinding

/**
 * Screen where the filling quantities of the selected drink should be displayed.
 */
class FillingQuantityScreen : Fragment() {

    private var _binding: FillingQuantityScreenBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
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

        binding.buttonToDrinkSelectionScreen.setOnClickListener {
            findNavController().navigate(R.id.action_toDataDisplayScreen)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
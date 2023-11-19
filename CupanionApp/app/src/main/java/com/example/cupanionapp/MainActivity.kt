package com.example.cupanionapp

import android.os.Bundle
import com.google.android.material.snackbar.Snackbar
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import android.view.Menu
import android.view.MenuItem
import com.androidnetworking.AndroidNetworking
import com.example.cupanionapp.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var appBarConfiguration: AppBarConfiguration
    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        WindowCompat.setDecorFitsSystemWindows(window, false)
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Initialize Fast Android Networking
        AndroidNetworking.initialize(applicationContext)

        setSupportActionBar(binding.toolbar)

        val navController = findNavController(R.id.nav_host_fragment_content_main)
        appBarConfiguration = AppBarConfiguration(navController.graph)
        setupActionBarWithNavController(navController, appBarConfiguration)

        // Set up destination change listener
        navController.addOnDestinationChangedListener { _, destination, _ ->
            // Check if the new destination is DataInputScreen or DataDisplayScreen
            if (destination.id == R.id.DataInputScreen || destination.id == R.id.DataDisplayScreen) {
                // Hide the back button when in DataDisplayScreen
                supportActionBar?.setDisplayHomeAsUpEnabled(false)
            } else {
                // Show the back button for other destinations
                supportActionBar?.setDisplayHomeAsUpEnabled(true)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                findNavController(R.id.nav_host_fragment_content_main).navigate(R.id.DataInputScreen)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment_content_main)

        // Check if the current destination is DataDisplayScreen
        if (navController.currentDestination?.id == R.id.DataDisplayScreen) {
            // Do not allow navigating up (disabling back button) when in DataDisplayScreen
            return false
        }
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }

    override fun onBackPressed() {
        val navController = findNavController(R.id.nav_host_fragment_content_main)

        // Check if the current destination is DataInputScreen or DataDisplayScreen
        if (navController.currentDestination?.id == R.id.DataInputScreen || navController.currentDestination?.id == R.id.DataDisplayScreen) {
            // Nullify the back action when in DataInputScreen or DataDisplayScreen
        } else {
            super.onBackPressed()
        }
    }
}
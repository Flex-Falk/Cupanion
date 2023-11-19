#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"

#include <stdlib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Constants for memory sizes for some variables, adjust accordingly if not enough
#define user_name_size 50
#define user_current_drink_size 50
#define user_drinks_list_size 200
#define json_struct_size 1024

// Struct for the UserData
struct UserData {
  char user_name[user_name_size] = "[noch nicht vergeben]";
  int user_goal = 0;
  char user_current_drink[user_current_drink_size] = "[kein Getraenk ausgewaehlt]";
  char user_drinks_list[user_drinks_list_size] = "[noch keine Getraenke getrunken]";
  int user_drinks_number = 0;
  bool user_drive = true;
};

// How many peopole the user toasted
int user_toasts = 0;

// The Webserver on port 80
WebServer server(80);

// Struct fot he received UserData
UserData received_UserData;

// Allocated memory for the image to be displayed
UBYTE *BlackImage;

// Current slideshow index
int slideshowIndex = 0;

/*Networking Stuff ----------------------------------------------------------------*/

// Handles HTTP "/post" requests from the Android device
void handlePost() {

  if (server.hasArg("plain")) {
    // Deserialize the raw received JSON string into the JSON document
    String raw_data = server.arg("plain");

    Serial.print("Received JSON: ");
    Serial.println(raw_data);
    DynamicJsonDocument dynamic_json(json_struct_size);
    DeserializationError error = deserializeJson(dynamic_json, raw_data);

    // Handle error accordingly
    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Populate the received_UserData struct
    strlcpy(received_UserData.user_name, dynamic_json["user_name"] | "[noch nicht vergeben]", sizeof(received_UserData.user_name));
    received_UserData.user_goal = dynamic_json["user_goal"] | 0;
    strlcpy(received_UserData.user_current_drink, dynamic_json["user_current_drink"] | "[kein Getraenk ausgewaehlt]", sizeof(received_UserData.user_current_drink));
    strlcpy(received_UserData.user_drinks_list, dynamic_json["user_drinks_list"] | "[noch keine Getraenke getrunken]", sizeof(received_UserData.user_drinks_list));
    received_UserData.user_drinks_number = dynamic_json["user_drinks_number"] | 0;
    received_UserData.user_drive = dynamic_json["user_drive"] | true;
    received_UserData.user_goal = dynamic_json["user_goal"] | 0;

    // Print the received UserData
    printUserData();
  }
}

// Printing the received UserData to the Serial Monitor in a readable format
void printUserData() {
  Serial.println("Statistiken:");
  Serial.print("Name: ");
  Serial.println(received_UserData.user_name[0]);
  Serial.print("Mit Leuten angestoßen: ");
  Serial.println(user_toasts);
  
  Serial.print("Trinkfortschritt: ");
  Serial.print(received_UserData.user_drinks_number);
  Serial.print(" / ");
  Serial.println(received_UserData.user_goal);

  Serial.print("Fahrtüchtigkeit: ");
  if(received_UserData.user_drive == true){
    Serial.println("In Ordnung");
  } else{
    Serial.println("Nicht mehr in Ordnung");
  }

  Serial.print("Aktuelles Getraenk: ");
  Serial.println(received_UserData.user_current_drink);
  Serial.print("Bisherige Getraenke: ");
  Serial.println(received_UserData.user_drinks_list);

  Serial.println();
}

// setting up all the server functions
void setupServer() {
  WiFi.softAP("ESP32"); // name of the WIFI Network
  server.on("/post", HTTP_POST, handlePost); // how to handle "/post" HTTP requests
  server.begin(); // starts the server

  // Print the ESP32's IP address once to Serial Monitor
  // Serial.println(WiFi.softAPIP());
}

/*Graphics Stuff ----------------------------------------------------------------*/
// increases or resets the slideshowIndex accordingly
void updateSlideshowIndex(){
  if (slideshowIndex == 5){ // end of slideshow reached, reset
    slideshowIndex = 0;
  } else{                   // next slideshow
    slideshowIndex++;
  }
}

// Function to display UserData on the e-ink display
void displayUserData(UBYTE *image) {

  // reset the screen (paint it black, etc.)
  EPD_4IN2_Clear();
  Paint_NewImage(BlackImage, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 270, BLACK);
  Paint_SelectImage(image);
  Paint_Clear(BLACK);

  // select the current slide to display and do so
  if (slideshowIndex == 0){
    Paint_DrawString_EN(10, 50, "Name: ", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(10, 100, received_UserData.user_name, &Font24, BLACK, WHITE);

  } else if (slideshowIndex == 1){
    // Convert int to char before drawing
    char toasts_str[10];
    itoa(user_toasts, toasts_str, 10);
    Paint_DrawString_EN(10, 50, "Mit Leuten angestossen: ", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(10, 100, toasts_str, &Font24, BLACK, WHITE);

  } else if (slideshowIndex == 2){
    // Convert ints to chars before drawing
    char drinks_number_str[10];
    char goal_str[10];
    itoa(received_UserData.user_drinks_number, drinks_number_str, 10);
    itoa(received_UserData.user_goal, goal_str, 10);

    char progress_str[20];
    strcat(progress_str, drinks_number_str);
    strcat(progress_str, " / ");
    strcat(progress_str, goal_str);

    Paint_DrawString_EN(10, 50, "Trinkfortschritt: ", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(10, 100, progress_str, &Font24, BLACK, WHITE);
  
  } else if (slideshowIndex == 3){
    Paint_DrawString_EN(10, 50, "Fahrtuechtigkeit: ", &Font24, BLACK, WHITE);
    if(received_UserData.user_drive == true){
      Paint_DrawString_EN(10, 100, "In Ordnung", &Font24, BLACK, WHITE);
    } else{
      Paint_DrawString_EN(10, 100, "Nicht mehr in Ordnung", &Font24, BLACK, WHITE);
    }
  } else if (slideshowIndex == 4){
    Paint_DrawString_EN(10, 50, "Aktuelles Getraenk: ", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(10, 100, received_UserData.user_current_drink, &Font24, BLACK, WHITE);
  
  } else{
    Paint_DrawString_EN(10, 50, "Bisherige Getraenke: ", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(10, 100, received_UserData.user_drinks_list, &Font24, BLACK, WHITE);
  }  
  
  EPD_4IN2_Display(image);
}

/* Entry point ----------------------------------------------------------------*/
void setup(){   
  Serial.begin(115200); // Baud Rate of the Serial Monitor

  setupServer(); //setup the server

  // initiliazing the e-ink-display
  DEV_Module_Init();
  EPD_4IN2_Init_Fast();
  EPD_4IN2_Clear();

  // Allocating memory for the image to be displayed
  UWORD Imagesize = ((EPD_4IN2_WIDTH % 8 == 0) ? (EPD_4IN2_WIDTH / 8 ) : (EPD_4IN2_WIDTH / 8 + 1)) * EPD_4IN2_HEIGHT;
  BlackImage = (UBYTE *)malloc(Imagesize);
}

// a constant for the delay between slides in milliseconds
const unsigned long delayBetweenSlides = 7500;  // 7,5 seconds

// Variable to keep track of the last time the display was updated
unsigned long lastUpdateTime = 0;

/* The main loop -------------------------------------------------------------*/
void loop() {
  // Handling further interaction with the server
  server.handleClient();
  
  // Check if enough time has passed since the last update
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= delayBetweenSlides) {
    // Display the UserData at the current index in the slideshow  
    displayUserData(BlackImage);

    // Prepare for the next slide
    updateSlideshowIndex();

    // Update the last update time
    lastUpdateTime = currentTime;
  }
}
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"

#include <stdlib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Constants for memory sizes for some variables, adjust accordingly if not enough
#define user_name_size 50
#define user_last_known_name_size 50
#define user_last_known_drink_size 50
#define user_current_drink_size 50
#define user_drinks_list_size 200
#define json_struct_size 1024

// Struct for the UserData
struct UserData {
  char user_name[user_name_size] = "";
  int user_goal = 0;
  char user_current_drink[user_current_drink_size] = "";
  char user_drinks_list[user_drinks_list_size] = "";
  int user_drinks_number = 0;
  bool user_drive = true;

  char user_last_known_name[user_last_known_name_size] = "";
  char user_last_known_drink[user_last_known_drink_size] = "";
};

// How many peopole the user toasted
int user_toasts = 0;

// The Webserver on port 80
WebServer server(80);

// Struct fot he received UserData
UserData received_UserData;

// Allocated memory for the image to be displayed
UBYTE *DisplayImage;

// Checks if an NFC Event has been triggered
bool nfc_event_occured = false;

/*Networking Stuff ----------------------------------------------------------------*/

// Handles HTTP "/post" requests from the Android device
void handlePost() {
    Serial.println("Received a POST request...");
    server.send(200, "text/plain", "POST request received successfully");

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
    strlcpy(received_UserData.user_name, dynamic_json["user_name"] | "", sizeof(received_UserData.user_name));
    received_UserData.user_goal = dynamic_json["user_goal"] | 0;
    strlcpy(received_UserData.user_current_drink, dynamic_json["user_current_drink"] | "", sizeof(received_UserData.user_current_drink));
    strlcpy(received_UserData.user_drinks_list, dynamic_json["user_drinks_list"] | "", sizeof(received_UserData.user_drinks_list));
    received_UserData.user_drinks_number = dynamic_json["user_drinks_number"] | 0;
    received_UserData.user_drive = dynamic_json["user_drive"] | true;

    // Print the received UserData
    printUserData();

    //if there is new data for the screen, update the "last known" variables and then the display
    if (strcmp(received_UserData.user_current_drink, received_UserData.user_last_known_drink) != 0 || strcmp(received_UserData.user_name, received_UserData.user_last_known_name) != 0){
      strlcpy(received_UserData.user_last_known_name, received_UserData.user_name, sizeof(received_UserData.user_last_known_name));
      strlcpy(received_UserData.user_last_known_drink, received_UserData.user_current_drink, sizeof(received_UserData.user_last_known_drink));
      normalDisplayUpdate(DisplayImage);
    }
    } else {
        server.send(400, "text/plain", "Bad Request");
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
  Serial.print("Server is listening for incoming connections under the following IP-Adress: ");
  Serial.println(WiFi.softAPIP().toString() + "\n");
}

/*Graphics Stuff ----------------------------------------------------------------*/

// Function to display UserData on the e-ink display
void normalDisplayUpdate(UBYTE *image) {

  // reset the screen (paint it white, etc.)
  EPD_4IN2_Clear();
  Paint_NewImage(DisplayImage, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 270, WHITE);
  Paint_SelectImage(image);
  Paint_Clear(WHITE);

  Paint_DrawString_EN(10, 50, "Hey, mein Name ist ", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 100, received_UserData.user_name, &Font24, WHITE, BLACK);

  Paint_DrawString_EN(10, 200, "In meinem Glas befindet sich aktuell ", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 300, received_UserData.user_current_drink, &Font24, WHITE, BLACK);
  
  EPD_4IN2_Display(image);
}

// the special display update when an NFC event happened, displays more user data
void NFCDisplayUpdate (UBYTE *image){
  // reset the screen (paint it white, etc.)
  EPD_4IN2_Clear();
  Paint_NewImage(DisplayImage, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 270, WHITE);
  Paint_SelectImage(image);
  Paint_Clear(WHITE);

  char toasts_str[10];
  itoa(user_toasts, toasts_str, 10);
  Paint_DrawString_EN(10, 50, "Ich habe schon mit Leuten angestossen: ", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(10, 100, toasts_str, &Font12, WHITE, BLACK);

  char drinks_number_str[10];
  char goal_str[10];
  itoa(received_UserData.user_drinks_number, drinks_number_str, 10);
  itoa(received_UserData.user_goal, goal_str, 10);

  char progress_str[20];
  strcat(progress_str, drinks_number_str);
  strcat(progress_str, " / ");
  strcat(progress_str, goal_str);

  Paint_DrawString_EN(10, 150, "Das ist mein bisheriger Trinkfortschritt: ", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(10, 200, progress_str, &Font12, WHITE, BLACK);

  Paint_DrawString_EN(10, 250, "Meine bisherigen Getraenke sind ", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(10, 300, received_UserData.user_drinks_list, &Font12, WHITE, BLACK);

  Paint_DrawString_EN(10, 400, "Schoen dich kennenzulernen!", &Font12, WHITE, BLACK);

  delay(7500);

  EPD_4IN2_Display(DisplayImage);
}

/* Entry point ----------------------------------------------------------------*/
void setup(){   
  Serial.begin(115200); // Baud Rate of the Serial Monitor

  // initiliazing the e-ink-display
  Serial.println("ESP32 server is starting...");
  setupServer(); //setup the server

  // initiliazing the e-ink-display
  Serial.println("E-paper display initialization starts");
  DEV_Module_Init();
  EPD_4IN2_Init_Fast();
  EPD_4IN2_Clear();

  // Allocating memory for the image to be displayed
  UWORD Imagesize = ((EPD_4IN2_WIDTH % 8 == 0) ? (EPD_4IN2_WIDTH / 8 ) : (EPD_4IN2_WIDTH / 8 + 1)) * EPD_4IN2_HEIGHT;
  DisplayImage = (UBYTE *)malloc(Imagesize);

  normalDisplayUpdate(DisplayImage);

  Serial.println("E-paper display initialization finished");
}

/* The main loop -------------------------------------------------------------*/
void loop() {
  // NFC events should alwyays have the higher relevance than other stuff
  if (nfc_event_occured == true){
    NFCDisplayUpdate(DisplayImage);
    nfc_event_occured = false;
  } else {
    // Handling further interaction with the server
    server.handleClient();
  }
}
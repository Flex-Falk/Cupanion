#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "imagedata.h"
#include <stdlib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Constants for memory sizes for some variables, adjust accordingly if not enough
#define user_name_size 50
#define current_drink_size 50
#define user_drinks_list_size 200
#define json_struct_size 1024

// Struct for the UserData
struct UserData {
  char user_name[user_name_size];
  int user_goal;
  char current_drink[current_drink_size];
  char user_drinks_list[user_drinks_list_size];
  int user_drinks_number;
  bool user_drive;
  int user_toasts;
};

// The Webserver on port 80
WebServer server(80);

// Struct fot he received UserData
UserData received_UserData;

/*Networking Stuff ----------------------------------------------------------------*/

// Handles HTTP "/post" requests from the Android device
void handlePost() {

  if (server.hasArg("plain")) {
    // Deserialize the raw received JSON string into the JSON document
    String raw_data = server.arg("plain");
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
    strlcpy(received_UserData.current_drink, dynamic_json["current_drink"] | "", sizeof(received_UserData.current_drink));
    strlcpy(received_UserData.user_drinks_list, dynamic_json["user_drinks_list"] | "", sizeof(received_UserData.user_drinks_list));
    received_UserData.user_drinks_number = dynamic_json["user_drinks_number"] | 0;
    received_UserData.user_drive = dynamic_json["user_drive"] | false;
    received_UserData.user_toasts = dynamic_json["user_toasts"] | 0;
    received_UserData.user_goal = dynamic_json["user_goal"];

    // Print the received UserData
    printUserData();
  }
}

// Printing the received UserData to the Serial Monitor in a readable format
void printUserData() {
  Serial.println("Statistiken:");
  Serial.print("Name: ");
  Serial.println(received_UserData.user_name[0] != '\0' ? received_UserData.user_name : "[noch nicht vergeben]");
  Serial.print("Mit Leuten angestoßen: ");
  Serial.println(received_UserData.user_toasts);
  
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

  Serial.print("Aktuelles Getränk: ");
  Serial.println(received_UserData.current_drink);
  Serial.print("Bisherige Getränke: ");
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

/* Entry point ----------------------------------------------------------------*/
void setup(){   
  Serial.begin(115200); // Baud Rate of the Serial Monitor

  setupServer(); //setup the server
}

/* The main loop -------------------------------------------------------------*/
void loop() {
  // Handling futher interaction with the server
  server.handleClient();
}

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
  char user_name[user_name_size] = "[noch nicht vergeben]";
  int user_goal = 0;
  char user_current_drink[user_current_drink_size] = "unbekannt";
  char user_drinks_list[user_drinks_list_size] = "noch nichts";
  int user_drinks_number = 0;
  bool user_drive = true;

  char user_last_known_name[user_last_known_name_size] = "unbekannt";
  char user_last_known_drink[user_last_known_drink_size] = "noch nichts";
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
    Serial.print("check");

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
    strlcpy(received_UserData.user_name, dynamic_json["user_name"] | "unbekannt", sizeof(received_UserData.user_name));
    received_UserData.user_goal = dynamic_json["user_goal"] | 0;
    strlcpy(received_UserData.user_current_drink, dynamic_json["user_current_drink"] | "noch nichts", sizeof(received_UserData.user_current_drink));
    strlcpy(received_UserData.user_drinks_list, dynamic_json["user_drinks_list"] | "noch nichts", sizeof(received_UserData.user_drinks_list));
    received_UserData.user_drinks_number = dynamic_json["user_drinks_number"] | 0;
    received_UserData.user_drive = dynamic_json["user_drive"] | true;

    // Print the received UserData
    printUserData();

    //if there is new data for the screen, update the "last known" variables and then the display
    if (strcmp(received_UserData.user_current_drink, received_UserData.user_last_known_drink) != 0 || strcmp(received_UserData.user_name, received_UserData.user_last_known_name) != 0){
      strlcpy(received_UserData.user_last_known_name, received_UserData.user_name, sizeof(received_UserData.user_last_known_name));
      strlcpy(received_UserData.user_last_known_drink, received_UserData.user_current_drink, sizeof(received_UserData.user_last_known_drink));
      //normalDisplayUpdate();
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
  Serial.println(WiFi.softAPIP().toString() + "\n");}

/*Graphics Stuff ----------------------------------------------------------------*/

// the normal display update if the name or current drink have changed, displays only the basic userdata
void normalDisplayUpdate (){
  // reset the screen (paint it black, etc.)
  EPD_4IN2_Clear();
  Paint_NewImage(DisplayImage, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 270, BLACK);
  Paint_SelectImage(DisplayImage);
  Paint_Clear(BLACK);

  Paint_DrawString_EN(10, 50, "Hey, mein Name ist ", &Font24, BLACK, WHITE);
  Paint_DrawString_EN(10, 100, received_UserData.user_name, &Font24, BLACK, WHITE);

  Paint_DrawString_EN(10, 200, "In meinem Glas befindet sich aktuell ", &Font24, BLACK, WHITE);
  Paint_DrawString_EN(10, 250, received_UserData.user_current_drink, &Font24, BLACK, WHITE);

  EPD_4IN2_Display(DisplayImage);
}

// the special display update when an NFC event happened, displays more user data
void NFCDisplayUpdate (){
  // reset the screen (paint it black, etc.)
  EPD_4IN2_Clear();
  Paint_NewImage(DisplayImage, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 270, BLACK);
  Paint_SelectImage(DisplayImage);
  Paint_Clear(BLACK);

  char toasts_str[10];
  itoa(user_toasts, toasts_str, 10);
  Paint_DrawString_EN(10, 50, "Ich habe schon mit Leuten angestossen: ", &Font12, BLACK, WHITE);
  Paint_DrawString_EN(10, 100, toasts_str, &Font12, BLACK, WHITE);

  Paint_DrawString_EN(10, 150, "In meinem Glas befindet sich aktuell ", &Font12, BLACK, WHITE);
  Paint_DrawString_EN(10, 200, received_UserData.user_current_drink, &Font12, BLACK, WHITE);

  char drinks_number_str[10];
  char goal_str[10];
  itoa(received_UserData.user_drinks_number, drinks_number_str, 10);
  itoa(received_UserData.user_goal, goal_str, 10);

  char progress_str[20];
  strcat(progress_str, drinks_number_str);
  strcat(progress_str, " / ");
  strcat(progress_str, goal_str);

  Paint_DrawString_EN(10, 250, "Das ist mein bisheriger Trinkfortschritt: ", &Font12, BLACK, WHITE);
  Paint_DrawString_EN(10, 300, progress_str, &Font12, BLACK, WHITE);

  Paint_DrawString_EN(10, 350, "Meine bisherigen Getraenke sind ", &Font12, BLACK, WHITE);
  Paint_DrawString_EN(10, 400, received_UserData.user_drinks_list, &Font12, BLACK, WHITE);

  Paint_DrawString_EN(10, 500, "Schön dich kennenzulernen!", &Font12, BLACK, WHITE);

  delay(7500);

  EPD_4IN2_Display(DisplayImage);
}

/* Entry point ----------------------------------------------------------------*/
void setup(){   
  Serial.begin(115200); // Baud Rate of the Serial Monitor

  // initiliazing the e-ink-display
  Serial.println("ESP32 server is starting...");
  setupServer(); //setup the server
  
  DEV_Delay_ms(1000);

  Serial.println("E-paper display initialization starts");

  //EPD_4IN2_Init_Fast();
  //EPD_4IN2_Clear();

  //Allocating memory for the image to be displayed
  UWORD Imagesize = ((EPD_4IN2_WIDTH % 8 == 0) ? (EPD_4IN2_WIDTH / 8 ) : (EPD_4IN2_WIDTH / 8 + 1)) * EPD_4IN2_HEIGHT;
  DisplayImage = (UBYTE *)malloc(Imagesize);

  // First display update
  //normalDisplayUpdate();

  Serial.println("E-paper display initialization finished");
}

/* The main loop -------------------------------------------------------------*/
void loop() {

  // NFC events should alwyays have the higher relevance than other stuff
  if (nfc_event_occured == true){
    NFCDisplayUpdate();
    nfc_event_occured = false;
  } else {
    // Handling further interaction with the server
    server.handleClient();
  }
}
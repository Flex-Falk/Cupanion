#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "imagedata.h"

#include <stdlib.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <WebServer.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"

#include <Adafruit_NeoPixel.h>

#include <Adafruit_PN532.h>

// Constants for memory sizes for some variables, adjust accordingly if not enough
#define user_name_size 50
#define user_last_known_name_size 50
#define user_last_known_drink_size 50
#define user_current_drink_size 50
#define user_drinks_list_size 200
#define json_struct_size 1024

#define PN532_IRQ   (2)
#define PN532_RESET (3) 


// Variables for LED strip
#define LED_PIN    13
#define LED_COUNT 6
bool LED_on = false;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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

//Create nfc object and initialise communication to the reader with I2C protocol
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// How many peopole the user toasted
int user_toasts = 0;

// The Webserver on port 80
AsyncWebServer server(80);

// Struct fot he received UserData
UserData received_UserData;

// Allocated memory for the image to be displayed
UBYTE *DisplayImage;

// Checks if an NFC Event has been triggered
bool nfc_event_occured = false;

char nameFromTag[50];

/*LED Stuff ----------------------------------------------------------------*/

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

/*NFC Stuff ----------------------------------------------------------------*/

void nfc_init(){
  Serial.println("Starting NFC Communication");
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("NFC functionality not available");
    return;
  }
  else{
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
    Serial.println("NFC functionality ready");  
  }
}

const int maxDataSize = 4 * 60;
const int bufferSize = 240;

bool checkForNFC_Information(){
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t data[32];
  uint8_t complete_data[maxDataSize];
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(success){
    if(uidLength == 7){ //uid Length of 7 means the tag is ntag15, ntag13 or ntag16 which are needed  
      Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");
      for (uint8_t i = 0; i < 60; i++){
        success = nfc.ntag2xx_ReadPage(i, data);   //Read 4 Bytes at a time
        if (success){
          for(int u=0; u<4;u++){
            complete_data[(i*4)+u] = data[u];
          }
        }
      }     
    }else{ 
      Serial.println("This doesn't seem to be an NTAG2xx tag (UUID length != 7 bytes)!"); 
      return false; 
    }

    char textBuffer[bufferSize];

    for(int z=0; z<bufferSize; z++){
      if(complete_data[z] != 0){ 
        textBuffer[z] = complete_data[z]; //only
      }else{
        textBuffer[z] = 32; //If the character in the complete_data is not usable, it is replaced with a new line
        } 
    }

    // Print out the entire textBuffer
    Serial.println("Text from the Tag:");
    for (int i = 0; i < bufferSize; i++) {
      Serial.print(textBuffer[i]);
      Serial.print(" ");
    }
    Serial.println("");

    int namebegin = findWord(textBuffer, "FN:", 0, 1);
    int nameend = findWord(textBuffer, "\n", namebegin, 0);
    char nameToSend[nameend-namebegin+1];
    
    Serial.print("Name from the Tag: ");
    for(int z=0; z<(nameend-namebegin);z++){
      Serial.print(textBuffer[namebegin+z]);
      nameToSend[z] = textBuffer[namebegin+z];
      nameFromTag[z] = textBuffer[namebegin+z];
    }
  
    Serial.println("");
    user_toasts++;
    nfc_event_occured = true;
  }
  return true;
}

//Find the index of the first character of the given word (wordToFind) inside the String (textChar) which contains the NFC Tags entire content
//If the String contains the searched word multiple times, the function will return the index of the first character of the first found word
//To access the information behind the first found word, the user can provide a startIndex for the search
//To find the index of the first character after the searched word, set addWordLen to True
int findWord(char textChar[], char wordToFind[], int startIndex, bool addWordLen){
    char* textCharP = textChar;
    char* wordToFindP = wordToFind;
    int WordLength = strlen(wordToFindP);
    //WordLength = WordLength - 1; //Null terminierung nicht vergleichen
    int TextLength = strlen(textChar);
    
    char compareChar[WordLength];
    
    for(int i=startIndex; i<(TextLength-WordLength); i++){   
      for(int c=0; c<WordLength; c++){
        compareChar[c] = textCharP[i+c];
      }
    
      int wrongWord = 0;
      for(int w=0; w<WordLength; w++){
        if(compareChar[w]==wordToFindP[w]){}
        else{
          wrongWord = 1;
        }
      }
      
      if(!wrongWord){
        if(addWordLen){return i+WordLength;}
        else{  return i;}
      }
       
    }
  return -1;
}

/*Networking Stuff ----------------------------------------------------------------*/
String postContent = ""; //String to send on GET request

void handle_NFC_getRequest(){
  postContent = ""; //clear postContent
  DynamicJsonDocument doc(json_struct_size);
  doc["name"] = nameFromTag;
  doc["toasts"] = user_toasts;
  Serial.println(postContent);

  serializeJson(doc, postContent); 
}

// Handles HTTP "/post" requests from the Android device
void handlePost(String raw_data) {
  Serial.println("Received a POST request...");

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
}

// Printing the received UserData to the Serial Monitor in a readable format
void printUserData() {
  Serial.println("Statistiken:");
  Serial.print("Name: ");
  Serial.println(received_UserData.user_name);
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
  WiFi.softAP("CupanionRED"); // name of the WIFI Network

  // Print the ESP32's IP address once to Serial Monitor
  Serial.print("Server is listening for incoming connections under the following IP-Adress: ");
  Serial.println(WiFi.softAPIP().toString() + "\n");

  //server.on("/post", HTTP_POST, handlePost); 
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/post", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<200> data;
    if (json.is<JsonArray>())
    {
      data = json.as<JsonArray>();
    }
    else if (json.is<JsonObject>())
    {
      data = json.as<JsonObject>();
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);

    handlePost(response);
  });
  server.addHandler(handler);

  server.on("/nfc-get", HTTP_GET, [](AsyncWebServerRequest *request) {
    handle_NFC_getRequest();
    request->send(200, "application/json", postContent);
  });

  server.begin(); // starts the server
}

/*Graphics Stuff ----------------------------------------------------------------*/

void padAndTruncateString(char* input, char* result, int totalLength) {
    int inputLength = strlen(input);

    if (inputLength > totalLength) {
        // If the input is longer than the total length, truncate it
        strncpy(result, input, totalLength);
        result[totalLength] = '\0';
    } else {
        // Calculate the number of spaces to add on both sides
        int spacesToAdd = (totalLength - inputLength) / 2;

        // Create a temporary buffer to store the padded string
        char paddedString[totalLength + 1];

        // Fill the buffer with spaces on the left
        memset(paddedString, ' ', spacesToAdd);
        paddedString[spacesToAdd] = '\0';

        // Concatenate the original string
        strcat(paddedString, input);

        // Add spaces on the right if needed
        int remainingSpaces = totalLength - strlen(paddedString);
        if (remainingSpaces > 0) {
            strcat(paddedString, " ");
        }

        // Copy the result back to the destination string
        strcpy(result, paddedString);
    }
}

// Function to display UserData on the e-ink display
void normalDisplayUpdate(UBYTE *image) {

  // reset the screen (paint it white, etc.)
  EPD_4IN2_Clear();
  Paint_NewImage(DisplayImage, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 270, WHITE);
  Paint_SelectImage(image);
  Paint_Clear(WHITE);

  Paint_DrawString_EN(100, 40, "Hallo!", &Font24, WHITE, BLACK);

  char optimized_user_name[18] = "";
  char optimized_current_drink[18] = "";

  padAndTruncateString(received_UserData.user_name, optimized_user_name, 17);
  padAndTruncateString(received_UserData.user_current_drink, optimized_current_drink, 17);

  Paint_DrawString_EN(30, 100, "Mein Name ist:", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 130, optimized_user_name, &Font24, WHITE, BLACK);
  
  Paint_DrawString_EN(30, 190, "In meinem Glas", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(32, 220, "befindet sich:", &Font24, WHITE, BLACK);
  
  Paint_DrawString_EN(10, 250, optimized_current_drink, &Font24, WHITE, BLACK);

  Paint_DrawString_EN(80, 300, "Lass uns", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(70, 330, "anstossen!", &Font24, WHITE, BLACK);

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
  Paint_DrawString_EN(10, 50, "Ich habe schon so", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(15, 80, "oft angestossen:", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(145, 110, toasts_str, &Font24, WHITE, BLACK);

  char drinks_number_str[10];
  char goal_str[10];
  itoa(received_UserData.user_drinks_number, drinks_number_str, 10);
  itoa(received_UserData.user_goal, goal_str, 10);

  Paint_DrawString_EN(20, 170, "Mein bisheriger", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 200, "Trinkfortschritt:", &Font24, WHITE, BLACK);
  
  Paint_DrawString_EN(110, 230, drinks_number_str, &Font24, WHITE, BLACK);
  Paint_DrawString_EN(150, 230, "/", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(185, 230, goal_str, &Font24, WHITE, BLACK);

  Paint_DrawString_EN(45, 300, "Schoen dich", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(25, 330, "kennenzulernen!", &Font24, WHITE, BLACK);

  delay(7500);

  EPD_4IN2_Display(DisplayImage);
}

/* Entry point ----------------------------------------------------------------*/
void setup(){   
  Serial.begin(115200); // Baud Rate of the Serial Monitor

  // initiliazing NFC reader
  nfc_init();

  // initiliazing the e-ink-display
  Serial.println("ESP32 server is starting...");
  setupServer(); //setup the server

  // initiliazing the e-ink-display
  Serial.println("E-paper display initialization starts");
  DEV_Module_Init();
  EPD_4IN2_Init_Fast();
  EPD_4IN2_Clear();

  //initializing LED strip
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Allocating memory for the image to be displayed
  UWORD Imagesize = ((EPD_4IN2_WIDTH % 8 == 0) ? (EPD_4IN2_WIDTH / 8 ) : (EPD_4IN2_WIDTH / 8 + 1)) * EPD_4IN2_HEIGHT;
  DisplayImage = (UBYTE *)malloc(Imagesize);


  Serial.print("E-paper display has been initialized with the following width and height: ");
  Serial.print (EPD_4IN2_WIDTH);
  Serial.print (" ");
  Serial.print (EPD_4IN2_HEIGHT);

  normalDisplayUpdate(DisplayImage);
}


/* The main loop -------------------------------------------------------------*/
void loop() {

  //Checks for NFC Tags every second
  //delay(1000);
  checkForNFC_Information();
  
  // NFC events should alwyays have the higher relevance than other stuff
  if (nfc_event_occured == true){
    //LED_on = true;
    theaterChaseRainbow(70);
    NFCDisplayUpdate(DisplayImage);
    //LED_on = false;
    //theaterChaseRainbow(70);
    delay(10000);
    nfc_event_occured = false;
    normalDisplayUpdate(DisplayImage);
  }
}
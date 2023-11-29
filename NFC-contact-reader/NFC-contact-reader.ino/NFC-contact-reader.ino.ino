/**************************************************************************/
/*!
    @file     readntag203.pde
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    This example will wait for any NTAG203 or NTAG213 card or tag,
    and will attempt to read from it.

    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC breakout
      ----> https://www.adafruit.com/products/364

    Check out the links above for our tutorials and wiring diagrams
    These chips use SPI or I2C to communicate.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>


// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
//Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  Serial.println("Waiting for an ISO14443A Card ...");

}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card

    int integerSave = 0;
    char textSave [1];
    char uidText [uidLength*2];
    for(int countt=0; countt<uidLength; countt++){
      integerSave = uid[countt];
      itoa(integerSave, textSave, 16); //convert number to String, converted number is shown in hexadecimal (16) (decimal would be 10)
      //strcat(uidText, " ");
      if(uid[countt]>=0 && uid[countt]<=9){strcat(uidText, "0");}
      strcat(uidText, textSave);
      if(countt!=uidLength-1){strcat(uidText, ":");} //only append a space if the number is not the last
    }

    for(int u=0; u<strlen(uidText); u++){
      Serial.print(uidText[u]);
    }
    Serial.println("");
    
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    if (uidLength == 7)
    {
      uint8_t data[32];
      uint8_t complete_data[4*60];

      // We probably have an NTAG2xx card (though it could be Ultralight as well)
      Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");

      // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
      // starting at page 4 ... larger cards just add pages to the end of
      // this range:

      // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf

      // TAG Type       PAGES   USER START    USER STOP
      // --------       -----   ----------    ---------
      // NTAG 203       42      4             39
      // NTAG 213       45      4             39
      // NTAG 215       135     4             129
      // NTAG 216       231     4             225

      for (uint8_t i = 0; i < 60; i++){
      
        success = nfc.ntag2xx_ReadPage(i, data);   //Read 4 Bytes at a time
        if (success){
          for(int revolve=0; revolve<4;revolve++){
            complete_data[(i*4)+revolve] = data[revolve];
          }
        }
      }
      
      Serial.println("finishedReading");

      char texti[240];
      char vcard[] = "VCARD";

      for(int printi = 0;printi<240;printi++){
        if(complete_data[printi] != 0){ texti[printi] = complete_data[printi]; }
        else{texti[printi] = 32; }
          
        Serial.print(texti[printi]);
        Serial.print(" ");
      }
      
        Serial.println("");

        Serial.print("Texti länge");
        Serial.println(strlen(texti));

        int namebegin = findWord(texti, "FN:", 0, 1);
        Serial.print("Name Beginn:");
        Serial.println(namebegin);
        int nameend = findWord(texti, "\n", namebegin, 0);
        Serial.print("Name End:");
        Serial.println(nameend);

        char nameToSend[nameend-namebegin+1];
        for(int z=0; z<(nameend-namebegin);z++){
          Serial.print(texti[namebegin+z]);
          nameToSend[z] = texti[namebegin+z];
        }
        Serial.println("");
        Serial.println("");
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
  }
}

char retrieveInfo(char infoToFind[], char text[]){
    char* infoToFindP = infoToFind;
    char* textP = text;

    //findWord
    
}

int findWord(char textChar[], char wordToFind[], int startIndex, bool addWordLen){
    char* textCharP = textChar;
    char* wordToFindP = wordToFind;
    int WordLength = strlen(wordToFindP);
    //WordLength = WordLength - 1; //Null terminierung nicht vergleichen
    int TextLength = strlen(textChar);
    Serial.print("Text Length:");
    Serial.print(TextLength);
    char compareChar[WordLength];
    
    for(int i=startIndex; i<(TextLength-WordLength); i++){
    
      for(int c=0; c<WordLength; c++){
        compareChar[c] = textCharP[i+c];
        //Serial.print(compareChar[c]);
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

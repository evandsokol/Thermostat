/******** Includes ********/
#include "DHT.h"
#include <Encoder.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

/******** Initializations and Definitions ********/

// Debug definition
#define DEBUG

// Temperature Control Modes
#define TEMP_OFF 0
#define TEMP_HEAT 1
#define TEMP_COOL 2

// Humidity Control Modes
#define DHUM_OFF 10
#define DHUM_ON 11

// Fan Control Modes
#define FAN_OFF 20
#define FAN_ON 21

// Value for uninitialized sensors
#define SENSOR_UNINIT -999

// SPI for PCD8544/Nokia5110
// For hardware SPI, define: Data/Command (DC), Chip Enable (CE), Reset(RST) pins
// The remaining PCD8544 pins map to the following Arduino pins: (Serial Clock) CLK => SCLK, (Data Input) DIN => MOSI
// For software SPI, all pins must be defined.
#define PCD8544_RST_PIN 4
#define PCD8544_CE_PIN 5
#define PCD8544_DC_PIN 6
#define PCD8544_DIN_PIN 7
#define PCD8544_CLK_PIN 8

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
#define ENCODER_DT_PIN 2
#define ENCODER_CLK_PIN 3

// Pin for DHT sensor data communication
#define DHT_PIN 9     

// Define type of DHT being used: DHT11//DHT21//DHT22
#define DHT_TYPE DHT11

Encoder myEnc(ENCODER_DT_PIN, ENCODER_CLK_PIN);

// Initialize DHT sensor.
DHT dht(DHT_PIN, DHT_TYPE);

// Software SPI initialization (slower updates, more flexible pin options):
Adafruit_PCD8544 display = Adafruit_PCD8544(PCD8544_CLK_PIN, PCD8544_DIN_PIN, PCD8544_DC_PIN, PCD8544_CE_PIN, PCD8544_RST_PIN);

// Hardware SPI initialization(faster, but must use certain hardware pins):
// Adafruit_PCD8544 display = Adafruit_PCD8544(PCD8544_DC_PIN, PCD8544_CE_PIN, PCD8544_RST_PIN);
// Note with hardware SPI MISO and SCLK pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

//Screen Contrast (0-100)
#define CONTRAST 20

// Menu Options
char back = "Back";
char menu_opt1 = "Temp Mode";
char menu_opt2 = "Set Temp";
char menu_opt3 = "Dehumidifer Mode";
char menu_opt4 = "Set Humidity";
char menu_opt5 = "Fan Mode";

* char settings menu[6]

/******** Setup ********/
void setup() {

  #ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Thermostat Test");
  #endif

  // DHT init
  dht.begin();

  // display init
  display.begin();
  display.setContrast(CONTRAST);
  display.clearDisplay();
  display.println("  Loading... ");
  display.display();
  display.clearDisplay();

  // display.clearDisplay();   // clears the screen and buffer
  // display.setTextSize(1);
  // display.setTextColor(BLACK);
  // display.setCursor(0,0);
  // display.println("Hello, world!");
  // display.setTextColor(WHITE, BLACK); // 'inverted' text
  // display.print("0x"); 
  // display.println(0xDEADBEEF, HEX);
  // display.display();
  // display.setRotation(1);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
  // display.invertDisplay(true);

}

/******** Declaration of Global Variables ********/
// Initializaiton for rotary encoder position
long oldPosition  = -999;

// Clear sensor error flags
bool DHT_Err = false;

// Clear element error flags
// N/A currently

// Initialize thermostat modes
int TempMode = TEMP_OFF;
int DhumMode = DHUM_OFF;
int FanMode = FAN_OFF;

// Initialize sensor values and set points
float InsideTemp = SENSOR_UNINIT;
float SetTemp = SENSOR_UNINIT;
float InsideHum = SENSOR_UNINIT;
float SetHum = SENSOR_UNINIT;

/******** Main Loop ********/
void loop() {
  // put your main code here, to run repeatedly:
  // Wait a few seconds between measurements.
  delay(2000);

  ElementUpdate();
  DisplayStatus();

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
  else {
    Serial.println(newPosition);
  }

}


/******** Functions ********/
void ElementUpdate(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  InsideHum = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  InsideTemp = dht.readTemperature(true);

  // Check if any reads failed and exit early or continue to process element update.
  if (isnan(InsideHum) || isnan(InsideTemp)) {
    Serial.println("Failed to read from DHT sensor!");
    DHT_Err = true;
    ElementsOff();
    return;
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(InsideHum);
    Serial.print(" %\t");
    Serial.print("Temperature: ");;
    Serial.print(InsideTemp);
    Serial.println(" *F\t");
  }

  
}

void ElementsOff(){
  Serial.print("Turning Elements Off");
}

void DisplayStatus(){
  // Temperature
  display.print("Temp:  ");
  if (InsideTemp<100){
    display.print(" ");
  }
  display.print(int(InsideTemp));
  display.println("F ");

  if (TempMode == TEMP_OFF){
    display.println("  OFF ");
  }
  else{
    switch (TempMode) {
      case TEMP_HEAT :
        display.print(" HEAT  ");
        break;
      case TEMP_COOL :
        display.print(" COOL  ");
        break;
      default :
        display.print(" ERR  ");
    }
      
    if (InsideTemp<100){
      display.print(" ");
    }
    display.print(int(SetTemp));
    display.println("F");
    if (SetTemp<100){
      display.print(" ");
    }
    display.print(int(SetTemp));
    display.println("F");
  }
  
  

  // Humidity
  display.print("Hmdty: ");
  if (InsideHum<100){
    display.print(" ");
  }
  display.print(int(InsideHum)); 
  display.println("%");
  if (DhumMode == DHUM_OFF) {
    display.println("  OFF");
  }
  else {
    display.print("   ON  ");
    display.print(SetHum);
    display.println("%");
  }
  
  // Fan
  display.print("Fan:    ");
  if (FanMode == FAN_OFF) {
    display.println("OFF");
  }
  else {
    display.println(" ON");
  }
  
  display.display();
  display.clearDisplay();
}

void DisplaySettings(int selectedRow, int topRow){
  for (int row=topRow; row++; row < topRow+6){
    switch(row){
      case 1: 
        printSettingItem(row, selectedRow, topRow, "Back");
      case 2:
        printSettingItem(row, selectedRow, topRow, "Temp Mode");
      case 3:
        printSettingItem(row, selectedRow, topRow, "Set Temp");
      case 4:
        printSettingItem(row, selectedRow, topRow, "Dehumidifer Mode");
      case 5:
        printSettingItem(row, selectedRow, topRow, "Set Humidity");
      case 6:
        printSettingItem(row, selectedRow, topRow, "Fan Mode");
    }
  }
}

void printSettingItem(int row, int selectedRow, int topRow, char charString){
  if (row == selectedRow) {
    display.setTextColor(WHITE, BLACK);
    display.println(charString);
    display.setTextColor(BLACK);
  }
  if (row >= topRow + 6){
    break;
  }
}


#include "ethereum_provider.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <iomanip>

// #define MY_ADDRESS "0x07Aca15D34f6A01B909267dbBA9139Fbff7c278F"                        // Put your wallet address here
// #define TARGET_ADDRESS "0x07Aca15D34f6A01B909267dbBA9139Fbff7c278F"                    // put your second address here
#define CONTRACT_ADDRESS "0x5FbDB2315678afecb367f032d93F642f64180aa3" // put your contract address here
// #define PRIVATE_KEY "871ccd03a445db4f3e42622f423f64be3df9aae5c2b371dd3b842b331ccb16ee" // put your contract address here
#define LED_BUILTIN 2
#define LED_WIFI 4
#define LED_BLOCKCHAIN 15
#define BTN_EX_CONFIG 16
#define SV_HOST "be.freshfood.com.vn"
#define SV_PORT 443
#define THREADHOLD_DISTANCE 100     // 100m
#define TIME_PUSH_TRANSACTION 60000 // 1 minute

String serverName = "https://be.freshfood.lalo.com.vn/";

String dataWifi = "";
String ssid = "P5";
String pass = "anhthien85";

String dataBlockchain = "";
String address = "0x70997970C51812dc3A010C7d01b50e0d17dc79C8 ";
String privateKey = "59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d";
uint256_t productId;

JsonArray stations;
String stationString;
size_t stationSize = 0;
int checkCounter = 0;

String dataGPS = "";

static const int RXPin = 26, TXPin = 27;

const char *wifiPath = "/wifi.txt";
const char *blockchainPath = "/blockchain.txt";
const char *gpsPath = "/gps.txt";

// Begin Button
int buttonState = HIGH;                 // Current state of the button
int lastButtonState = HIGH;             // Previous state of the button
unsigned long lastDebounceTime = 0;     // Time of the last button state change
unsigned long debounceDelay = 50;       // Debounce delay in milliseconds
unsigned long longPressDuration = 1500; // Duration threshold for a long press in milliseconds
unsigned long buttonPressStartTime = 0; // Time when the button was pressed
bool isButtonPressed = false;           // Flag to track button press
bool isLongPress = false;               // Flag to indicate long press
volatile bool configRequested = false;
bool gpsInitPush = false;

// End Button

void initSPIFFS();
void writeFile(fs::FS &fs, const char *path, const char *message);
String readFile(fs::FS &fs, const char *path);

void initWifi();
bool wifiStatus();

void checkButtonPress();
void buttonShortPressedAction();
void buttonLongPressedAction();

String getLocation();
String getTimestamp();
bool transtactionStatus(string hash);
DynamicJsonDocument getStationsFromServer(String ownerAddress);
bool checkDistance(float longitude, float latitude);
void sendMailToServer(DynamicJsonDocument jsonDoc);

void gpsMode();
void configMode();

#include "ethereum_provider.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <iomanip>

#define CONTRACT_ADDRESS "0xdFdE6B33f13de2CA1A75A6F7169f50541B14f75b" // put your contract address here

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
string newOwnerAddress = "";
string newOwnerName = "";
string newOwnerDescription = "";
JsonArray stations;
String stationString;
size_t stationSize = 0;
int checkCounter = 0;
bool transferSuccess = false;

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
bool checkDistance(double longitude, double latitude);
void sendMailToServer(String data);

void gpsMode();
void configMode();

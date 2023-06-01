#include "ethereum_provider.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <iomanip>

#define MY_ADDRESS "0x90F79bf6EB2c4f870365E785982E1f101E93b906"                          // Put your wallet address here
#define TARGET_ADDRESS "0x90F79bf6EB2c4f870365E785982E1f101E93b906"                      // put your second address here
#define PRIVATE_KEY "0x7c852118294e51e653712a81e05800f419141751be58f605c371e15141b007a6" // put your contract address here
#define CONTRACT_ADDRESS "0x5FbDB2315678afecb367f032d93F642f64180aa3"                    // put your contract address here

String dataWifi = "";
String ssid = "";
String pass = "";

static const int RXPin = 13, TXPin = 12;

const char *wifiPath = "/wifi.txt";
static const char *PARAM_INPUT_1 = "ssid";
static const char *PARAM_INPUT_2 = "pass";
static const char *PARAM_INPUT_3 = "private_key";
static const char *PARAM_INPUT_4 = "gps_serial";

#define BUTTON_WIFI_MODE 16
#define FLASH_LED 4

// Begin Button
int buttonState = HIGH;                 // Current state of the button
int lastButtonState = HIGH;             // Previous state of the button
unsigned long lastDebounceTime = 0;     // Time of the last button state change
unsigned long debounceDelay = 50;       // Debounce delay in milliseconds
unsigned long longPressDuration = 1500; // Duration threshold for a long press in milliseconds
unsigned long buttonPressStartTime = 0; // Time when the button was pressed
bool isButtonPressed = false;           // Flag to track button press
bool isLongPress = false;               // Flag to indicate long press
volatile bool apModeRequested = false;
// End Button

void initSPIFFS();
void writeFile(fs::FS &fs, const char *path, const char *message);
String readFile(fs::FS &fs, const char *path);
void initWifi();
void startAPMode();
void checkButtonPress();
void buttonShortPressedAction();
void buttonLongPressedAction();
String getLocation();
String getTimestamp();
std::time_t ConvertToTimestamp(const std::string &datetimeStr);

static const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title>ESP Wi-Fi Manager</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style type="text/css">
        html {
            font-family: Arial, Helvetica, sans-serif;
            display: inline-block;
            text-align: center;
        }

        h1 {
            font-size: 1.8rem;
            color: white;
        }

        p {
            font-size: 1.4rem;
        }

        .topnav {
            overflow: hidden;
            background-color: #0a1128;
        }

        body {
            margin: 0;
        }

        .content {
            padding: 5%;
        }

        .card-grid {
            max-width: 800px;
            margin: 0 auto;
            display: grid;
            grid-gap: 2rem;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
        }

        .card {
            background-color: white;
            box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, 0.5);
        }

        .card-title {
            font-size: 1.2rem;
            font-weight: bold;
            color: #034078;
        }

        input[type="submit"] {
            border: none;
            color: #fefcfb;
            background-color: #034078;
            padding: 15px 15px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            width: 100px;
            margin-right: 10px;
            border-radius: 4px;
            transition-duration: 0.4s;
        }

        input[type="submit"]:hover {
            background-color: #1282a2;
        }

        input[type="text"],
        input[type="number"],
        select {
            width: 50%;
            padding: 12px 20px;
            margin: 18px;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

        label {
            font-size: 1.2rem;
        }

        .value {
            font-size: 1.2rem;
            color: #1282a2;
        }

        .state {
            font-size: 1.2rem;
            color: #1282a2;
        }

        button {
            border: none;
            color: #fefcfb;
            padding: 15px 32px;
            text-align: center;
            font-size: 16px;
            width: 100px;
            border-radius: 4px;
            transition-duration: 0.4s;
        }

        .button-on {
            background-color: #034078;
        }

        .button-on:hover {
            background-color: #1282a2;
        }

        .button-off {
            background-color: #858585;
        }

        .button-off:hover {
            background-color: #252524;
        }
    </style>
</head>

<body>
    <div class="topnav">
        <h1>ESP Wi-Fi Manager</h1>
    </div>
    <div class="content">
        <div class="card-grid">
            <div class="card">
                <form action="/" method="POST">
                    <p>
                        <div>
                        <p>Network</p>
                        <label for="ssid">SSID</label>
                        <input type="text" id="ssid" name="ssid" value="P5"><br>

                        <label for="pass">Password</label>
                        <input type="text" id="pass" name="pass" value="anhthien85"><br>
                        </div>
                        
                        <div>
                        <p>Ethereum</p>
                        <label for="private_key">Private Key</label>
                        <input type="text" id="private_key" name="private_key" placeholder="Please add private key"><br>
                        
                        <label for="gps_serial">Serial</label>
                        <input type="text" id="gps_serial" name="gps_serial" placeholder="Please add gps serial"><br>
                        
                        </div>
                        <input type="submit" value="Submit">
                    </p>
                </form>
            </div>
        </div>
    </div>
</body>

</html>
)rawliteral";
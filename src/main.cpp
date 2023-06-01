#include "main.h"

AsyncWebServer server(80);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
Web3 *web3 = new Web3(HARDHAT_ID);
EthereumProvider ethereumProvider(PRIVATE_KEY, MY_ADDRESS);

void buttonInterrupt()
{
  apModeRequested = true;
}

void setup()
{
  Serial.begin(9600);
  ss.begin(9600);
  pinMode(BUTTON_WIFI_MODE, INPUT_PULLUP);
  pinMode(FLASH_LED, OUTPUT);
  digitalWrite(FLASH_LED, LOW);

  attachInterrupt(digitalPinToInterrupt(BUTTON_WIFI_MODE), buttonInterrupt, FALLING);
  initSPIFFS();
  initWifi();

  String data = readFile(SPIFFS, "/data.txt");
  Serial.print("data: ");
  Serial.println(data);

  if (WiFi.status() == WL_CONNECTED)
  {
    ethereumProvider.setupWeb3(web3);
    ethereumProvider.setContractAddress(CONTRACT_ADDRESS);

    String ownerName = "Device 2";
    String ownerDesc = "Delivery";

    digitalWrite(FLASH_LED, HIGH);
    string result = ethereumProvider.registerOwner(ownerName.c_str(), ownerDesc.c_str());
    Serial.print("registerOwner: ");
    Serial.println(result.c_str());
    digitalWrite(FLASH_LED, LOW);
  }
}

uint32_t lastTime = 0;

void loop()
{
  while (ss.available() > 0)
  {
    checkButtonPress();
    // if (apModeRequested)
    // {
    //   startAPMode();
    //   apModeRequested = false;
    // }
    // Serial.println("ss available");
    if (gps.encode(ss.read()))
    {
      String location = getLocation();
      Serial.print("Location: ");
      Serial.println(location);
      String timestamp = getTimestamp();
      Serial.print("Time: ");
      Serial.println(timestamp);

      if (millis() - lastTime > 60000)
      {
        lastTime = millis();
        if (WiFi.status() == WL_CONNECTED)
        {
          digitalWrite(FLASH_LED, HIGH);
          uint256_t productId = 0;
          ethereumProvider.addLog(productId, "delivery", "delivery", location.c_str(), timestamp.c_str());
          digitalWrite(FLASH_LED, LOW);
        }
        Serial.println("write file");
        writeFile(SPIFFS, "/data.txt", (location + ";" + timestamp).c_str());
        break;
      }
    }
    else
    {
      // Serial.println("gps not encode");
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true)
      ;
  }
}

String getLocation()
{
  String result = "";

  if (gps.location.isValid())
  {
    result = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  }

  return result;
}

String getTimestamp()
{
  tm timeNow;
  String dhy = "";
  String hms = "";
  if (gps.date.isValid())
  {
    dhy = String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
    timeNow.tm_year = gps.date.year() - 1900;
    timeNow.tm_mon = gps.date.month() - 1;
    timeNow.tm_mday = gps.date.day();
  }

  if (gps.time.isValid())
  {
    hms += String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
    timeNow.tm_hour = gps.time.hour();
    timeNow.tm_min = gps.time.minute();
    timeNow.tm_sec = gps.time.second();
  }
  Serial.print(dhy + " " + hms);

  time_t timestamp = mktime(&timeNow);

  String result = String(timestamp);
  return result;
}

void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
}

String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("Failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent += String((char)file.read());
  }
  return fileContent;
}

void initWifi()
{
  dataWifi = readFile(SPIFFS, wifiPath);

  int countBreakData = 0;
  for (int i = 0; i < dataWifi.length(); i++)
  {
    if (dataWifi[i] == '|')
    {
      countBreakData++;
      continue;
    }

    if (countBreakData == 1)
    {
      ssid += dataWifi[i];
    }
    else if (countBreakData == 0)
    {
      pass += dataWifi[i];
    }
  }

  ssid = "P5";
  pass = "anhthien85";

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter > 10)
    {
      break;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void checkButtonPress()
{
  int reading = digitalRead(BUTTON_WIFI_MODE); // Read the button state

  if (reading != lastButtonState)
  {
    lastDebounceTime = millis(); // Update the debounce time
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading; // Update the button state

      if (buttonState == LOW)
      {
        // Button is pressed
        isButtonPressed = true;
        buttonPressStartTime = millis(); // Record the button press start time
      }
      else
      {
        // Button is released
        if (isButtonPressed)
        {
          unsigned long buttonPressDuration = millis() - buttonPressStartTime;

          if (buttonPressDuration >= longPressDuration)
          {
            // Long press detected
            isLongPress = true;
            buttonLongPressedAction();
          }
          else
          {
            // Short press detected
            buttonShortPressedAction();
          }
        }
        isButtonPressed = false;
        isLongPress = false;
      }
    }
  }

  lastButtonState = reading; // Update the last button state
}

void buttonShortPressedAction()
{
  Serial.println("Button short pressed");
}

void buttonLongPressedAction()
{
  Serial.println("Button long pressed");
  startAPMode();
}

void startAPMode()
{
  WiFi.disconnect();
  delay(1000);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-AP", "123456789");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  dataWifi = "";

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", index_html); });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              int params = request->params();
                            for (int i = 0; i < params; i++)
                            {
                                AsyncWebParameter *p = request->getParam(i);
                                if (p->isPost())
                                {
                                    // HTTP POST ssid value
                                    if (p->name() == PARAM_INPUT_1)
                                    {
                                          ssid = p->value().c_str();
                                          Serial.print("SSID set to: ");
                                          Serial.println(ssid);
                                          dataWifi += ssid;
                                    }
                                    // HTTP POST pass value
                                    if (p->name() == PARAM_INPUT_2)
                                    {
                                          pass = p->value().c_str();
                                          Serial.print("Password set to: ");
                                          Serial.println(pass);
                                          dataWifi += pass;
                                    }

                                    if (p->name() == PARAM_INPUT_3)
                                    {
                                        Serial.println(p->value().c_str());
                                    }
                                }

                                dataWifi += "|";
                            }
                              Serial.println("Data Wifi: " + dataWifi);
                              writeFile(SPIFFS, wifiPath, dataWifi.c_str());
              request->send(200, "text/plain", "Done. ESP will restart, please waiting esp reconnect to new wifi.");
                               delay(3000);
                               ESP.restart(); });
  server.begin();
}

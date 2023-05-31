#include "main.h"

SocketIoClient socketIo;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
Web3 *web3 = new Web3(FRESHFOOD_ID);
EthereumProvider ethereumProvider;

void buttonInterrupt()
{
  configRequested = true;
}

void setup()
{
  Serial.begin(9600);
  ss.begin(9600);
  pinMode(BTN_EX_CONFIG, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_EX_CONFIG), buttonInterrupt, FALLING);
  initSPIFFS();
  initWifi();

  socketIo.on("request_transfer", socketOnRequestTransfer);
  socketIo.begin(SV_HOST, SV_PORT, "/socket.io/?transport=websocket");

  String gpsData = readFile(SPIFFS, gpsPath);
  Serial.print("gps: ");
  Serial.println(gpsData);

  if (WiFi.status() == WL_CONNECTED)
  {
    ethereumProvider.setAdress(address.c_str());
    ethereumProvider.setPrivateKey(privateKey.c_str());
    ethereumProvider.setupWeb3(web3);
    ethereumProvider.setContractAddress(CONTRACT_ADDRESS);

    string result = ethereumProvider.registerOwner("Trinh", "nguoi yeu tui");
    Serial.print("registerOwner: ");
    Serial.println(result.c_str());
    ethereumProvider.addLog(1, "delivery", "delivery", "1234567;87654321", 1684736442);
  }
}

uint32_t lastTime = 0;

void loop()
{
  bool gpsInitPush = false;

  while (ss.available() > 0)
  {
    socketIo.loop();

    checkButtonPress();
    if (configRequested)
    {
      break;
    }

    if (gps.encode(ss.read()))
    {
      String location = getLocation();
      Serial.print("Location: ");
      Serial.println(location);
      String timestamp = getTimestamp();
      Serial.print("Time: ");
      Serial.println(timestamp);

      if (millis() - lastTime > 60000 || !gpsInitPush)
      {
        lastTime = millis();

        if (WiFi.status() == WL_CONNECTED)
        {
          string result = ethereumProvider.addLog(3, "delivery", "delivery", location.c_str(), timestamp.toInt());
          if (result.substr(0, 2) == "0x")
          {
            writeFile(SPIFFS, gpsPath, (location + ";" + timestamp).c_str());
          }
        }

        if (!gpsInitPush)
        {
          gpsInitPush = true;
        }

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

  while (Serial.available() > 0)
  {
    char dataConfig[128];
    memset(dataConfig, '\0', 128);
    Serial.readBytesUntil('\n', dataConfig, 128);

    // char *token = strtok(dataConfig, "|");
    // string strToken = token;

    string temp = dataConfig;
    temp = temp.substr(0, temp.find('\0')).c_str();

    bool isAddress = temp.substr(0, 2) == "0x";

    // for (int i = 0; i < 2; i++)
    // {
    //   if (i == 0)
    //   {
    //     string strToken = token;
    //     isAddress = strToken.substr(0, 2) == "0x";
    //     if (isAddress)
    //     {
    //       address = token;
    //     }
    //     else
    //     {
    //       ssid = token;
    //     }
    //   }
    //   else if (i == 1)
    //   {
    //     string temp = token;
    //     if (isAddress)
    //     {
    //       privateKey = temp.substr(0, 64).c_str();
    //     }
    //     else
    //     {
    //       pass = temp.substr(0, temp.find('\0')).c_str();
    //     }
    //   }
    //   token = strtok(NULL, "|");
    // }

    // const char *token = temp.c_str();

    if (isAddress)
    {
      ss.print("blockchain: ");
      ss.println(temp.c_str());
      writeFile(SPIFFS, blockchainPath, temp.c_str());
    }
    else
    {
      ss.print("wifi: ");
      ss.println(temp.c_str());
      writeFile(SPIFFS, wifiPath, temp.c_str());
    }

    delay(1000);
    ESP.restart();
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

  dataWifi = readFile(SPIFFS, wifiPath);
  if (dataWifi != "")
  {
    string temp = dataWifi.c_str(); //
    temp = temp.substr(0, temp.find('\0')).c_str();
    ssid = temp.substr(0, temp.find('|')).c_str();
    pass = temp.substr(temp.find('|') + 1, temp.length()).c_str();
  }

  dataBlockchain = readFile(SPIFFS, blockchainPath);
  {
    string temp = dataBlockchain.c_str();
    temp = temp.substr(0, temp.find('\0')).c_str();
    address = temp.substr(0, temp.find('|')).c_str();
    privateKey = temp.substr(temp.find('|') + 1, temp.length()).c_str();
  }

  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.print("pass: ");
  Serial.println(pass);
  Serial.print("address: ");
  Serial.println(address);
  Serial.print("privateKey: ");
  Serial.println(privateKey);

  // char *temp2;
  // strcpy(temp2, dataBlockchain.c_str());
  // Serial.print("dataBlockchain: ");
  // Serial.println(temp2);
  // token = strtok(temp2, "|");
  // for (int i = 0; i < 2; i++)
  // {
  //   if (i == 0)
  //   {
  //     address = token;
  //   }
  //   else if (i == 1)
  //   {
  //     string temp = token;
  //     privateKey = temp.substr(0, 64).c_str();
  //   }
  //   token = strtok(NULL, "|");
  // }
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  ss.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    ss.println("Failed to open file for writing");
    return;
  }
  // clear data in file

  if (file.print(message))
  {
    ss.println("File written");
  }
  else
  {
    ss.println("Write failed");
  }
}

String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("Failed to open file for reading");
    return "";
  }

  Serial.println("Read from file: ");
  String result = "";
  while (file.available())
  {
    result += (char)file.read();
  }
  Serial.println(result);
  return result;
}

void initWifi()
{
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
  int reading = digitalRead(BTN_EX_CONFIG); // Read the button state

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
}

void socketOnRequestTransfer(const char *payload, size_t length)
{
  Serial.printf("got socketOnRequestTransfer: %s\n", payload);
}
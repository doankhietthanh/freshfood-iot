#include "main.h"

// SocketIoClient socketIo;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
Web3 *web3 = new Web3(FRESHFOOD_ID);
EthereumProvider ethereumProvider;

uint32_t lastTime = 0;

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
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_BLOCKCHAIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_BLOCKCHAIN, LOW);

  initSPIFFS();
  initWifi();

  // socketIo.on("request_transfer", socketOnRequestTransfer);
  // socketIo.begin(SV_HOST, SV_PORT, "/socket.io/?transport=websocket");

  dataGPS = readFile(SPIFFS, gpsPath);
  Serial.print("gps: ");
  Serial.println(dataGPS);

  if (wifiStatus())
  {
    ethereumProvider.setAdress(address.c_str());
    ethereumProvider.setPrivateKey(privateKey.c_str());
    ethereumProvider.setupWeb3(web3);
    ethereumProvider.setContractAddress(CONTRACT_ADDRESS);

    // string result = ethereumProvider.registerOwner("GPS device", "Blockchain IoT");
    // Serial.print("registerOwner: ");
    // Serial.println(result.c_str());
    DynamicJsonDocument jsonDocument = getStationsFromServer(address);

    JsonObject root = jsonDocument.as<JsonObject>();
    int productIdInt = root["productId"].as<int>();
    stations = root["stations"].as<JsonArray>();

    productId = productIdInt;

    for (JsonObject station : stations)
    {
      string name = station["name"].as<string>();
      float longitude = station["longitude"].as<float>();
      float latitude = station["latitude"].as<float>();

      Serial.print("Name: ");
      Serial.println(name.c_str());

      Serial.print("Longitude: ");
      Serial.println(longitude, 6);

      Serial.print("Latitude: ");
      Serial.println(latitude, 6);

      Serial.println("-----");
    }
  }
}

void loop()
{
  if (configRequested)
  {
    Serial.println("Config requested");
    configMode();
    configRequested = false;
  }
  else
  {
    gpsMode();
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

bool wifiStatus()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_WIFI, LOW);
    Serial.println("Wifi disconnected");
    return false;
  }

  digitalWrite(LED_WIFI, HIGH);
  return true;
}

bool transtactionStatus(string hash)
{
  if (hash.substr(0, 2) != "0x")
  {
    return false;
  }
  return true;
}

void gpsMode()
{

  while (ss.available() > 0)
  {
    // socketIo.loop();

    if (configRequested)
    {
      Serial.println("Press btn ex config");
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

        if (wifiStatus())
        {
          digitalWrite(LED_BLOCKCHAIN, HIGH);
          if (location != "" && timestamp != "")
          {
            string result = ethereumProvider.addLog(productId, "delivery", "delivery", location.c_str(), timestamp.c_str());
            if (transtactionStatus(result))
            {
              writeFile(SPIFFS, gpsPath, (location + ";" + timestamp).c_str());
            }
          }
          else
          {
            string gpsTemp = dataGPS.c_str(); //
            gpsTemp = gpsTemp.substr(0, gpsTemp.find('\0')).c_str();
            string locationTemp = gpsTemp.substr(0, gpsTemp.find(';')).c_str();
            string timeTemp = gpsTemp.substr(gpsTemp.find(';') + 1, gpsTemp.length()).c_str();
            if (locationTemp == "" || timeTemp == "")
            {
              locationTemp = "0,0";
              timeTemp = "0";
            }
            string result = ethereumProvider.addLog(productId, "delivery", "delivery", locationTemp.c_str(), timeTemp.c_str());
          }
          digitalWrite(LED_BLOCKCHAIN, LOW);

          int checkCounter = 0;
          std::string productIdStr;
          productIdStr << productId;
          JsonObject jsonMail;

          if (stations.size() == 0)
          {
            string subject = "#" + productIdStr + " - Transfer to new owner";
            jsonMail["to"] = "doankhietthanh@gmail.com";
            jsonMail["subject"] = subject.c_str();
            jsonMail["name"] = "Doan Khiet Thanh";
            jsonMail["content"] = "Transfer to new owner successfull";

            sendMailToServer(jsonMail);
          }
          else
          {
            for (JsonObject station : stations)
            {
              string name = station["name"].as<string>();
              float longitude = station["longitude"].as<float>();
              float latitude = station["latitude"].as<float>();

              if (checkDistance(latitude, longitude))
              {
                // remove station
                stations.remove(checkCounter);
                jsonMail["to"] = "doankhietthanh@gmail.com";
                jsonMail["subject"] = "Product is checked at " + name + " station";
                jsonMail["name"] = "Doan Khiet Thanh Test";
                jsonMail["content"] = "checked successfull";
                sendMailToServer(jsonMail);
                checkCounter++;
              }
            }
          }
        }

        if (!gpsInitPush)
        {
          gpsInitPush = true;
        }

        break;
      }
    }
  }
}

void configMode()
{
  digitalWrite(LED_BUILTIN, HIGH);

  while (Serial.available() > 0)
  {
    Serial.println("Ex config available");

    char dataConfig[128];
    memset(dataConfig, '\0', 128);
    Serial.readBytesUntil('\n', dataConfig, 128);

    string temp = dataConfig;
    temp = temp.substr(0, temp.find('\0')).c_str();

    bool isAddress = temp.substr(0, 2) == "0x";

    if (isAddress)
    {
      writeFile(SPIFFS, blockchainPath, temp.c_str());
      digitalWrite(LED_BUILTIN, LOW);
      ESP.restart();
    }
    else
    {
      writeFile(SPIFFS, wifiPath, temp.c_str());
      digitalWrite(LED_BUILTIN, LOW);
      ESP.restart();
    }
  }
}

DynamicJsonDocument getStationsFromServer(String ownerAddress)
{
  DynamicJsonDocument doc(4096);
  HTTPClient http;
  String query = serverName + "devices/serial/" + ownerAddress;
  http.begin(query);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    deserializeJson(doc, payload);
  }
  http.end();
  return doc;
}

bool checkDistance(float longitude, float latitude)
{
  float distance = gps.distanceBetween(latitude, longitude, gps.location.lat(), gps.location.lng());
  Serial.print("Distance: ");
  Serial.println(distance);
  if (distance <= THREADHOLD_DISTANCE)
  {
    return true;
  }
  return false;
}

void sendMailToServer(JsonObject object)
{
  String objectString;
  serializeJson(object, objectString);
  String query = serverName + "mail/send";
  HTTPClient http;
  http.begin(query);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(objectString);
  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println(payload);
  }
  http.end();
}

// void socketOnRequestTransfer(const char *payload, size_t length)
// {
//   Serial.printf("got socketOnRequestTransfer: %s\n", payload);
// }

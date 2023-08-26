#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoOSCWiFi.h>
#include <ezButton.h>
#include <ArduinoJson.h>  //Warning - only versions prior 6 are compatible (5.11)
#include <FS.h>
#include <SPIFFS.h>
#include <EasyLed.h>

WiFiManager wm;
WiFiManagerParameter custom_osc_ip("oscip", "OSC IP", "", 16);
WiFiManagerParameter custom_osc_port("oscport", "OSC Port", "53000", 8);
WiFiManagerParameter custom_osc_address("oscaddress", "OSC Address", "/go", 8);
WiFiManagerParameter custom_osc_argument("oscargument", "OSC Argument", "", 8);
WiFiManagerParameter custom_button_pin("buttonpin", "Button Pin (GPIO)", "13", 2);

//----------------------------------------------------------------
//Double Reset Detector

#if defined(ESP32)
  #define USE_SPIFFS            true
  #define ESP_DRD_USE_EEPROM    true
#else  
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.  
#endif
#include <ESP_WiFiManager.h>                    //https://github.com/khoih-prog/ESP_WiFiManager
#define DRD_TIMEOUT             0.1
#define DRD_ADDRESS             0
#include <ESP_DoubleResetDetector.h>            //https://github.com/khoih-prog/ESP_DoubleResetDetector
DoubleResetDetector* drd;
const int PIN_LED       = 3;
bool      initialConfig = false;

EasyLed led(2, EasyLed::ActiveLevel::High);

ezButton* button = nullptr;

void clearJson() {
  if (SPIFFS.exists("/config.json")) {
    Serial.println("Clearing config file");
    File configFile = SPIFFS.open("/config.json", "w");
    if (configFile) {
      configFile.close();
    }
  }
}

//Double Reset Detector run function()
void doubleResetConnector () {  
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset()) { Serial.println(F("DRD")); initialConfig = true; }
  // ESP_WiFiManager ESP_wifiManager("ConfigOnDoubleReset");
  // if (ESP_wifiManager.WiFi_SSID() == "") { Serial.println(F("No AP credentials")); initialConfig = true; }
  if (initialConfig) {
    //reset settings - wipe credentials for testing
    wm.resetSettings();
    Serial.println(F("Starting Config Portal")); led.flash();

    delay(1000);
    clearJson();
    // if (!ESP_wifiManager.startConfigPortal()) { Serial.println(F("Not connected to WiFi")); }
    // else { Serial.println(F("connected")); }
  }
  // else { WiFi.mode(WIFI_STA); WiFi.begin(); } 
  // unsigned long startedAt = millis();
  // digitalWrite(PIN_LED, LOW); Serial.print(F("After waiting "));
  // int connRes = WiFi.waitForConnectResult();
  // float waited = (millis() - startedAt);
  // Serial.print(waited / 1000); Serial.print(F(" secs , Connection result is ")); Serial.println(connRes);
  // if (WiFi.status() != WL_CONNECTED) { Serial.println(F("Failed to connect")); }
  // else { Serial.print(F("Local IP: ")); Serial.println(WiFi.localIP()); }
}

//----------------------------------------------------------------
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  led.flash();
}

//Save parameters
void saveCustomConfig () {
  // Diagnostic: Check if the function is called
  Serial.println("saveCustomConfig called");

  const char *host = custom_osc_ip.getValue();
  const char *send_port = custom_osc_port.getValue();
  const char *oscaddr = custom_osc_address.getValue();
  const char *oscarg = custom_osc_argument.getValue();
  const char *buttonpin = custom_button_pin.getValue();

  // Diagnostic: Print the variables to confirm they contain data
  Serial.println(host);
  Serial.println(send_port);
  Serial.println(oscaddr);
  Serial.println(oscarg);
  Serial.println(buttonpin);

  // Actual saving
  Serial.println("saving config");
  led.on();

  DynamicJsonDocument jsonDoc(1024);
  JsonObject json = jsonDoc.to<JsonObject>();
  
  json["host"] = host;
  json["send_port"] = send_port;
  json["oscaddr"] = oscaddr;
  json["oscarg"] = oscarg;
  json["buttonpin"] = buttonpin;

  File configFile = SPIFFS.open("/config.json", "w");
  serializeJson(json, Serial);
  serializeJson(json, configFile);
  

  // Check if the file opened successfully
  if (!configFile) {
    Serial.println("failed to open config file for writing");
    return;
  }

  // Check the return value of serializeJson to confirm serialization
  if (serializeJson(json, configFile) == 0) {
    Serial.println("Serialization failed");
    return;
  } else {
    Serial.println("Serialization succeeded");
  }

  //configFile.flush();
  Serial.println("File flushed");
  
  //configFile.close();
  Serial.println("File closed");
}


//----------------------------------------------------------------
//Serial print initial params
void saveParamsCallback () {
  Serial.println("Get Params:");
  Serial.print(custom_osc_ip.getID());
  Serial.print(" : ");
  Serial.println(custom_osc_ip.getValue());
  
  Serial.print(custom_osc_port.getID());
  Serial.print(" : ");
  Serial.println(custom_osc_port.getValue());
  
  Serial.print(custom_osc_address.getID());
  Serial.print(" : ");
  Serial.println(custom_osc_address.getValue());
  
  Serial.print(custom_osc_argument.getID());
  Serial.print(" : ");
  Serial.println(custom_osc_argument.getValue());

  Serial.print(custom_button_pin.getID());
  Serial.print(" : ");
  Serial.println(custom_button_pin.getValue());
}

//----------------------------------------------------------------
//Declare and link params to consts
const char *buttonpin = custom_button_pin.getValue();

const char *host = custom_osc_ip.getValue();
const char *send_port = custom_osc_port.getValue();
const char *oscaddr = custom_osc_address.getValue();
const char *oscarg = custom_osc_argument.getValue();

//ezButton button(std::stoi(buttonpin));

//----------------------------------------------------------------

void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    Serial.begin(115200);
    doubleResetConnector();
    led.flash();
    wm.setDarkMode(true);

    


    //reset settings - wipe credentials for testing
    // wm.resetSettings();

    wm.addParameter(&custom_osc_ip);
    wm.addParameter(&custom_osc_port);
    wm.addParameter(&custom_osc_address);
    wm.addParameter(&custom_osc_argument);
    wm.addParameter(&custom_button_pin);
    wm.setShowStaticFields(true);
    wm.setConfigPortalBlocking(true);

    
    
    
    //----------------------------------------------------------------

    //Get config file if there is one

    //
    if (SPIFFS.begin()) {
  Serial.println("mounted file system");
  if (SPIFFS.exists("/config.json")) {
    Serial.println("reading config file");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);

      DynamicJsonDocument jsonDoc(1024);
      DeserializationError error = deserializeJson(jsonDoc, buf.get());

      if (!error) {
        JsonObject json = jsonDoc.as<JsonObject>();
        serializeJson(json, Serial);
        
        // Updating these requires a different strategy than simply using `strcpy`
        // Depending on how these variables are declared and used, you may need to allocate memory for them.
        // Here I've assumed they can be safely modified.
        strcpy(const_cast<char *>(host), json["host"]);
        strcpy(const_cast<char *>(send_port), json["send_port"]);
        strcpy(const_cast<char *>(oscaddr), json["oscaddr"]);
        strcpy(const_cast<char *>(oscarg), json["oscarg"]);
        strcpy(const_cast<char *>(buttonpin), json["buttonpin"]);
        int buttonPinValue = std::stoi(buttonpin);
        button = new ezButton(buttonPinValue);
        button->setDebounceTime(50);
        Serial.println(buttonpin);
      }
      else
      {
        Serial.println("failed to load json config");
        SPIFFS.format();
      }
    }
  }
  ezButton button(std::stoi(buttonpin));
    }

  //----------------------------------------------------------------

  wm.setSaveParamsCallback(saveParamsCallback);

  // automatically connect using saved credentials if they exist
  // If connection fails it starts an access point with the specified name
  wm.setAPCallback(configModeCallback);

  if (wm.autoConnect("esp_osc"))
  {
    wm.setAPCallback(configModeCallback);
    Serial.println("This is the wifimodeinside");
    Serial.println(WiFi.getMode());
        Serial.println("connected...yeey :)");
        saveCustomConfig();
        //Serial.println("Rebooting...");
        //delay(4000); // Add a delay if necessary
        //ESP.restart(1);
        // digitalWrite(3, HIGH);
    }
    else {
        Serial.println("Configportal running");
        led.flash();
    }
    
}

void loop() {
    drd->loop();
    wm.process();
    OscWiFi.update();
    button->loop();

    // put your main code here, to run repeatedly:
    if(button->isPressed())
  {
    Serial.println("The button is pressed");
    Serial.println(host);
    Serial.println(send_port);
    Serial.println(oscaddr);
    Serial.println(oscarg);
    Serial.println(buttonpin);
    OscWiFi.send(host, std::stoi(send_port), oscaddr, oscarg);
    // return;
  }
  if (button->isReleased())
  {
    Serial.println("The button is released");
    //OscWiFi.send(host, send_port, "/off");
    // or do that separately
    // OscWiFi.parse(); // to receive osc
    // OscWiFi.post(); // to publish osc
  }
}
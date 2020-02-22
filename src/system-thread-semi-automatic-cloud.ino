#include "application.h"

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

STARTUP(prepare());

#define MYDEBUG TRUE

char strVersion[20] = "20200221-1355-01-P1";

unsigned long last5SecondLoop = 0UL;

bool boolConnected = false; // whether we've got wifi online to Particle Cloud

void prepare() {
    WiFi.setListenTimeout(300); // 5 mins
    WiFi.selectAntenna(ANT_INTERNAL);
}


byte mac[6];

void setup() {

    Serial.begin(115200);
    // Delay to start particle serial monitor
    delay (5000);

    // Spark read/write variables
    Particle.variable("version", strVersion, STRING);

    // Spark functions
    Particle.function("listenMode", callListenMode);
    Particle.function("reset", callReset);

    // Let's get going
    Serial.println("Setup checkpoint 1 before WiFi.on");

    WiFi.on();
    delay(300);

    WiFiAccessPoint ap[5];
    int found = WiFi.getCredentials(ap, 5);
    for (int i = 0; i < found; i++) {
        Serial.print("ssid: ");
        Serial.println(ap[i].ssid);
        // security is one of WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2, WLAN_SEC_WPA_ENTERPRISE, WLAN_SEC_WPA2_ENTERPRISE
        Serial.print("security: ");
        Serial.println(ap[i].security);
        // cipher is one of WLAN_CIPHER_AES, WLAN_CIPHER_TKIP or WLAN_CIPHER_AES_TKIP
        Serial.print("cipher: ");
        Serial.println(ap[i].cipher);
    }

    Serial.println("Setup checkpoint 2 before WiFi.connect");

    WiFi.connect();

    delay(300);

    Serial.println("Setup checkpoint 3 before Particle.connect");

    Particle.connect();

    Serial.println("Setup checkpoint 4 after Particle.connect");

    Particle.publish("debug", "Setup checkpoint 5 finished", 60, PRIVATE);
    Particle.publish("getTestSubscribe", "inside setup", 30, PRIVATE);


}


void cbGetSubscribe(const char *event, const char *data) {

    Particle.publish("debug", "cbGetSubscribe going: " + String(data), 60, PRIVATE);
    Serial.println("cbGetSubscribe fired ");

}

void loop() {

    // What is the time mr wolf?
    unsigned long now = millis();

    // Every 5 seconds
    if (now - last5SecondLoop >= 5000UL) {
        last5SecondLoop = now;
        if (MYDEBUG == TRUE) {
          Serial.println(" in 5sec loop");
        }

        if (Particle.connected() == true){
          if (boolConnected == false){

              Serial.println("boolConnected block, before .subscribe ");
              bool Subscribed = Particle.subscribe(String("hook-response/getTestSubscribe_") + System.deviceID(), cbGetSubscribe, MY_DEVICES);

              // Call the webhook
              Particle.publish("getTestSubscribe", "inside boolConnected", 30, PRIVATE);

              Particle.publish("firmware-version", String(strVersion), 60, PRIVATE);

              boolConnected = true;
          }

          Serial.println("Particle.connected = true");

        } else {
          Serial.println("Particle.connected = FALSE");
          boolConnected = false;
        }
    }
}

void delayReset(){
    System.reset();
}
Timer timerReset(1500, delayReset, TRUE); // one_shot = true, ie only execute once

int callReset(String nonsenseArg){
    Serial.println("Going to reset device now due to cloud function call!");
    Particle.publish("info", "Going to reset device now due to cloud function call!", 60, PRIVATE);
    timerReset.start(); // Trigger a listen mode AFTER the function has returned!!
    return 1;
}

void delayListenMode(){
    WiFi.listen();
}
Timer timerListen(1500, delayListenMode, TRUE); // one_shot = true, ie only execute once

int callListenMode(String nonsenseArg){
    Serial.println("Function listenMode called");
    Particle.publish("info", "Going to listen mode now due to cloud function call!", 60, PRIVATE);
    timerListen.start(); // Trigger a listen mode AFTER the function has returned!!
    return 1; // this needs to be sent back for the client calling the function to close neatly
}



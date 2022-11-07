#include <Arduino.h>
#include <twilio.hpp>
#include <CountDown.h>
#include <BfButton.h>
#include <config.h>

//borrowed initial reed switch code from https://lastminuteengineers.com/reed-switch-arduino-tutorial/
//also borrowed from https://esp32io.com/tutorials/esp32-door-sensor as well

BfButton btn(BfButton::STANDALONE_DIGITAL, buttonPin, true, LOW);
CountDown CD(CountDown::SECONDS);
bool washing = false;
bool messagesent = false;   // limit sending of texts

Twilio *twilio;
//WiFiUDP ntpUDP;

void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  Serial.print(btn->getID());
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      //Serial.println(" short pressed.");
      CD.start(timer);
      if(washing == true){
        washing = false;
        digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
        digitalWrite(blueLED, LOW);  // Turn the LED off
        Serial.println("Washing stopped");
      } else {
        washing = true;
        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED on
        digitalWrite(blueLED, HIGH);  // Turn the LED on
        Serial.println("Washing started");
        messagesent = false;
      }
      break;
    case BfButton::DOUBLE_PRESS:
      Serial.println(" double pressed.");
      break;
    case BfButton::LONG_PRESS:
      washing = false;
      digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
      digitalWrite(blueLED, LOW);  // Turn the LED off
      Serial.println("Washing stopped");
      break;
  }
}

void sendit() {
  Serial.println("Sending MMS");
  twilio = new Twilio(account_sid, auth_token);
  delay(1000);
  String response;
  bool success = true;
  //for (int i=0; i<2; i++) {
  //  Serial.println(to_numbers[i]);
  //}
  //bool success = twilio->send_message(to_number, from_number, message, response);
  if (success) {
    Serial.println("Sent message successfully!");
    messagesent = true;
    washing = false;
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
    digitalWrite(blueLED, LOW);  // Turn the LED off
  } else {
    Serial.println(response);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(" ");
  Serial.println("*** System Starting ***");
  //pinMode(sensorPin, INPUT_PULLUP);	// Enable internal pull-up for the reed switch
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(sensorPin, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(blueLED, OUTPUT);
    btn.onPress(pressHandler)
     .onDoublePress(pressHandler) // default timeout
     .onPressFor(pressHandler, 1000); // custom timeout for 1 second
  Serial.print("Connecting to WiFi network '");  // Connect to wifi
  Serial.print(ssid);
  Serial.println("' ...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(500);
  }
  Serial.print("Connected with IP ");  // show IP and boot message
  Serial.println(WiFi.localIP());
  setCpuFrequencyMhz(80);
  Serial.println("*** Startup Completed ***");
}

void loop() {

  btn.read();
  while(washing == true) {
    //Serial.println("In Washing");
    btn.read();
    int motion = digitalRead(sensorPin);  // Read the state of the sensor
    if (motion == HIGH) {   //movement detected
      Serial.println("Motion Detected");
      CD.start(timer);
    }
    if ( messagesent == false && CD.remaining() < 1  ) {
      Serial.print("Sending message after ");
      Serial.print(timer);
      Serial.println(" seconds");
      sendit();
      washing = false;
    }
  Serial.println(CD.remaining());  
  delay(250);  
  } //end of washing
}  // end of loop
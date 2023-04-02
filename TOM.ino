#include "esp32notifications.h"

#define MOTOR 27
#define HARDWARE_STANDARD

#ifdef HARDWARE_STANDARD
    #define BUTTON_A    25 // left button - use this GPIO pin
    #define BUTTON_B    26 // center button - use this GPIO pin
    #define BUTTON_C    27 // right button - use this GPIO pin
#else
    #error Hardware buttons not supported!
#endif

BLENotifications notifications;

uint32_t incomingCallNotificationUUID;

void onBLEStateChanged(BLENotifications::State state) {
  switch(state) {
      case BLENotifications::StateConnected:
          Serial.println("StateConnected - connected to a phone or tablet"); 
          break;

      case BLENotifications::StateDisconnected:
          Serial.println("StateDisconnected - disconnected from a phone or tablet"); 
          /* We need to startAdvertising on disconnection, otherwise the ESP 32 will now be invisible.
          IMO it would make sense to put this in the library to happen automatically, but some people in the Espressif forums
          were requesting that they would like manual control over re-advertising.*/
          notifications.startAdvertising(); 
          break; 
  }
}



void onNotificationArrived(const ArduinoNotification * notification, const Notification * rawNotificationData) {
    Serial.print("Got notification: ");   
    Serial.println(notification->title); // The title, ie name of who sent the message
    Serial.println(notification->message); // The detail, ie "be home for dinner at 7".
    Serial.println(notification->type);  // Which app sent it
    Serial.println(notifications.getNotificationCategoryDescription(notification->category));  // ie "social media"
    Serial.println(notification->categoryCount); // How may other notifications are there from this app (ie badge number)
    if (notification->category == CategoryIDIncomingCall) {
		// If this is an incoming call, store it so that we can later send a user action.
        incomingCallNotificationUUID = notification->uuid;
        Serial.println("--- INCOMING CALL: PRESS A TO ACCEPT, C TO REJECT ---"); 
    }
    else {
        incomingCallNotificationUUID = 0; // Make invalid - no incoming call
    }
    digitalWrite(MOTOR, HIGH);
    delay(1000);
    digitalWrite(MOTOR, LOW);
}


void onNotificationRemoved(const ArduinoNotification * notification, const Notification * rawNotificationData) {
     Serial.print("Removed notification: ");   
     Serial.println(notification->title);
     Serial.println(notification->message);
     Serial.println(notification->type);  
}


// Standard Arduino function which is called once when the device first starts up
void setup() {
  // Button configuration. It is usual to have buttons configured as INPUT_PULLUP in the hardware design,
  // but check the details for your specific device 
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

    Serial.begin(115200);
    while(!Serial) {
        delay(10);
    }

    Serial.println("ESP32-ANCS-Notifications Example");
    Serial.println("------------------------------------------");    

    // Set up the BLENotification library
    notifications.begin("BLEConnection device name");
    notifications.setConnectionStateChangedCallback(onBLEStateChanged);
    notifications.setNotificationCallback(onNotificationArrived);
    notifications.setRemovedCallback(onNotificationRemoved);
}


// Standard Arduino function that is called in an endless loop after setup
void loop() {   
    if (incomingCallNotificationUUID > 0) {
		// Check to see if the user has pressed an action button
	    if (digitalRead(BUTTON_A) == LOW) {
	      Serial.println("Positive action."); 
	        notifications.actionPositive(incomingCallNotificationUUID);
	    }
	    else if (digitalRead(BUTTON_C) == LOW) {
	      Serial.println("Negative action."); 
	        notifications.actionNegative(incomingCallNotificationUUID);
	    }
    }
}
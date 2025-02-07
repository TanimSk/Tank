#include <Arduino.h>
#include <SparkFun_TB6612.h>

// For the nRF24L01 receiver
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// LED
#define BULITIN_LED 2

// For the motor driver
#define PWMA 13
#define AIN2 14
#define AIN1 27

#define STBY 26

#define BIN1 25
#define BIN2 33
#define PWMB 32

// Define nRF24L01 pins
#define CE_PIN 4
#define CSN_PIN 5

// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

// Initializing motors.  The library will allow you to initialize as many
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// Create an RF24 object
RF24 radio(CE_PIN, CSN_PIN);

// Define the address of the communication pipe
const byte address[6] = "12345";

// Variable to store received data
char receivedData[32] = "";

void setup()
{
   Serial.begin(115200);
   pinMode(BULITIN_LED, OUTPUT);

   // Initialize the nRF24L01 module
   while (!radio.begin())
   {
      Serial.println("Error initializing the nRF24L01 module. Check your connections...");
      delay(4000);
   }

   // Set the communication pipe and enable listening mode
   radio.openReadingPipe(0, address);
   radio.setPALevel(RF24_PA_HIGH); // Power level
   radio.startListening();         // Start receiving data

   Serial.println("Receiver is ready.");
   brake(motor1, motor2);
}

// Extract the motor speeds from the received data
int motorSpeeds[2] = {0, 0};

void setMotorSpeeds(char data[])
{
   int newMotorSpeeds[2] = {0, 0};

   // data = "100,200"
   char *token = strtok(data, ",");
   if (token == NULL)
      return;

   newMotorSpeeds[0] = atoi(token);

   token = strtok(NULL, ",");
   if (token == NULL)
      return;

   newMotorSpeeds[1] = atoi(token);

   // Update the motor speeds
   motorSpeeds[0] = newMotorSpeeds[0];
   motorSpeeds[1] = newMotorSpeeds[1];
}

void loop()
{
   // forward(motor1, motor2, 100);
   // delay(2000);
   // // Use of brake again.
   // delay(2000);

   // Check if data is available
   if (radio.available())
   {
      digitalWrite(BULITIN_LED, HIGH);
      // Read the incoming data
      radio.read(&receivedData, sizeof(receivedData));
      setMotorSpeeds(receivedData);

      // Print the received data
      Serial.print("Motor speeds: ");
      Serial.print(motorSpeeds[0]);
      Serial.print(", ");
      Serial.println(motorSpeeds[1]);

      // Set the motor speeds
      if (motorSpeeds[0] != 0 && motorSpeeds[1] != 0)
      {
         motor1.drive(motorSpeeds[0]);
         motor2.drive(motorSpeeds[1]);
      }
      if (motorSpeeds[0] == 0)
      {
         motor1.brake();
      }
      if (motorSpeeds[1] == 0)
      {
         motor2.brake();
      }

      delay(100);
      return;
   }

   digitalWrite(BULITIN_LED, LOW);
   delay(100);
}

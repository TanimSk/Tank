#include <Arduino.h>
#include <SparkFun_TB6612.h>

// For the nRF24L01 receiver
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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

   // Initialize the nRF24L01 module
   while (!radio.begin())
   {
      Serial.println("Error initializing the nRF24L01 module. Check your connections...");
      delay(4000);
   }

   // Set the communication pipe and enable listening mode
   radio.openReadingPipe(0, address);
   radio.setPALevel(RF24_PA_LOW); // Power level
   radio.startListening();        // Start receiving data

   Serial.println("Receiver is ready.");
}

void loop()
{
   // forward(motor1, motor2, 100);
   // delay(2000);
   // // Use of brake again.
   // brake(motor1, motor2);
   // delay(2000);

   // Check if data is available
   if (radio.available())
   {
      // Read the incoming data
      radio.read(&receivedData, sizeof(receivedData));
      Serial.print("Received: ");
      Serial.println(receivedData);
   }

   delay(100);
}

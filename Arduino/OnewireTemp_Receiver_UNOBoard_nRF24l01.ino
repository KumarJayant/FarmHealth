#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>



#include <SPI.h>


/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN  9
#define CSN_PIN 10
#define MAX_TEMP 65

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe


/*-----( Declare objects )-----*/
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
/*-----( Declare Variables )-----*/
float joystick[2];  // 2 element array holding Joystick readings

void setup()   /****** SETUP: RUNS ONCE ******/
{

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);

  Serial.begin(9600);
  // delay(2000);
  Serial.println("Nrf24L01 Receiver Starting");
  radio.begin();
  radio.openReadingPipe(1, pipe);
  //  radio.openReadingPipe(pipe);
  radio.startListening();;
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  if ( radio.available() )
  {
    // Fetch the data payload
    radio.read( joystick, sizeof(joystick) );
    Serial.print("Celcius = ");
    Serial.print(joystick[0]);
    Serial.print(" Farenheit = ");
    Serial.println(joystick[1]);
    if (joystick[0] > MAX_TEMP)
    {
      digitalWrite(3, HIGH);   // set the Red LED on
      //delay(1000);        // wait for a second
      //digitalWrite(4, LOW);
    }
    else
    {
      digitalWrite(4, HIGH);    // set the Green LED off
     // delay(1000);              // wait for a second
      //digitalWrite(3, LOW);
    }
  }
  else
  {
    Serial.println("No radio available");
  }

  delay(1000);

}//--(end main loop )---


/*-----( Declare User-written Functions )-----*/

//NONE
//*********( THE END )***********


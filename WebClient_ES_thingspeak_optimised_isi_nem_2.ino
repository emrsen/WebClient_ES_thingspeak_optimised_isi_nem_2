/*************************************************** 
  This is an example for the Adafruit CC3000 Wifi Breakout & Shield

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
 /*
This example does a test of the TCP client capability:
  * Initialization
  * Optional: SSID scan
  * AP connection
  * DHCP printout
  * DNS lookup
  * Optional: Ping
  * Connect to website and print out webpage contents
  * Disconnect
SmartConfig is still beta and kind of works but is not fully vetted!
It might not work on all networks!
*/
#include <Adafruit_CC3000.h>
#include <ccspi.h>
//#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include "DHT.h"
#include <SFE_BMP180.h>
#include <Wire.h>
// #include <ThingSpeak.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "Avatar2012"           // cannot be longer than 32 characters!
#define WLAN_PASS       "Alp_Ozlem_99"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "api.thingspeak.com" // See? No 'http://' in front of it
#define WEBPAGE      "thingspeak.com/channels/64583"
Adafruit_CC3000_Client   client;
uint32_t ip;


#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

int sayac = 1;
int boom = 199;

SFE_BMP180 pressure; // You will need to create an SFE_BMP180 object, here called "pressure":

#define ALTITUDE 45.0 // Altitude of Emre home in meters

/**************************************************************************/
/*!
    @brief  Thingspeak Emre deneme channel yarları
*/
/**************************************************************************/
unsigned long testChannelNumber = 64583;
String testChannelWriteAPIKey = "U8ONCCO120N37PGG";
long previousMillis = 0;        // will store last time TS was updated
long WRITE_DELAY_FOR_THINGSPEAK = 300000;

void(* resetFunc) (void) = 0;//declare reset function at address 0

void init_network()
{
 /* Initialise the module */
 sayac = sayac + 10;
 Serial.print(F("\nSayac degeri :  ")); Serial.println(sayac); // Sayac degeri kontrol.
 if (sayac >= boom)
 {
    Serial.println("BOOMM");
    resetFunc();  //call reset
 }
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  // Optional SSID scan
  // listSSIDResults();
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(120); // ToDo: Insert a DHCP timeout!
     sayac = sayac + 1;
      Serial.print(F("\nSayac degeri :  ")); Serial.println(sayac); // Sayac degeri kontrol.
      if (sayac >= boom)
        {
        Serial.println("BOOMM");
         resetFunc();  //call reset
        }
    }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
    sayac = 1;
    }

  ip = 0;
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);

  // ThingSpeak.begin(client);
 
  } 
}

void init_bmp180()
{
if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    resetFunc();  //call reset
  }
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  init_network();
  
  Serial.println("DHT22 test!");

  dht.begin();
  
  init_bmp180();
}

void loop(void)
{
  double P; // Pressure data
  P = getPressure(); // Get a new pressure reading:
  Serial.print("baseline pressure: ");
  Serial.print(P);
  Serial.println(" mb");  

   
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  Serial.print(F("\nTS Channel Number :  ")); Serial.println(testChannelNumber);
  Serial.print(F("\nTS API Key :  ")); Serial.println(testChannelWriteAPIKey);
  Serial.print(F("\nTS IP adresi :  ")); cc3000.printIPdotsRev(ip);
  // ThingSpeak.setField(1,sicaklik);
  //ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey); 
  // ThingSpeak.writeField(testChannelNumber, 1, sicaklik, testChannelWriteAPIKey);

  
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
  if (www.connected()) 
    {
      //   www.fastrprint(F("api.thingspeak.com/update?key=U8ONCCO120N37PGG&field1=18"));
      //   www.println();
      String postStr = testChannelWriteAPIKey;
      postStr +="&field1=";
      postStr += String(t);
      postStr +="&field2=";
      postStr += String(h);
      postStr +="&field3=";
      postStr += String(hic);
      postStr += "\r\n\r\n";

      www.print("POST /update HTTP/1.1\n");
      www.print("Host: api.thingspeak.com\n");
      www.print("Connection: close\n");
      www.print("X-THINGSPEAKAPIKEY: "+testChannelWriteAPIKey+"\n");
      www.print("Content-Type: application/x-www-form-urlencoded\n");
      www.print("Content-Length: ");
      www.print(postStr.length());
      www.print("\n\n");
      www.print(postStr);
    Serial.print(F("\nThingspeak e yazdik Yazilan Deger  :")); Serial.println(postStr);
    sayac = sayac + 1;
    if (sayac >= boom)
      {
      Serial.println("BOOMM");
      resetFunc();  //call reset
      }
    Serial.print(F("\nSayac degeri :  ")); Serial.println(sayac); // Sayac degeri kontrol. 
    Serial.print("Bekliycez Delay  :"); Serial.println(WRITE_DELAY_FOR_THINGSPEAK);   
    delay(WRITE_DELAY_FOR_THINGSPEAK); // Always wait to ensure that rate limit isn't hit
    } 
  else 
    {
    Serial.println(F("Connection failed"));  
    init_network();  
    // return;
    }
  www.stop();
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    init_network(); 
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

double getPressure()
{
  char status;
  double T,P,p0,a;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}


// ESP8266 Garage door monitor and activator
// Based on the ESP8266WebServer examples for client and server
// Uses Ticker to do a nonblocking loop every 1 second to check status

#include <Ticker.h>
#include <ESP8266WiFi.h>

#define DEBUG true // flag to turn on/off debugging
#define Serial if(DEBUG)Serial 
#define CLOSED HIGH	
#define OPEN LOW

const int statusPin = 13; //pin for the magnetic contact switch
const int activatePin = 12; //pin for the relay module
const int ledPin = 0; //onboard LED for the Adafruit ESP8266 HUZZAH breakout

Ticker ticker;
WiFiServer server(80);
WiFiClient client;
const IPAddress updateserver(192,168,1,100);
const int updateport = 8084;

const char* SSID = "YourWifiSSID";
const char* PASS = "YourWifiPassword";

int Status = -1;
bool SendUpdate = false;

void setup() {
	Serial.begin(115200);
	pinMode(statusPin, INPUT_PULLUP);
	pinMode(activatePin, OUTPUT);
	digitalWrite(activatePin, LOW);
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);
	delay(50);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);

	WiFi.begin(SSID, PASS);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	Serial.println("WiFi connected");
	delay(1000);

	//Start the server
	server.begin();
	Serial.println("Server started");

	//Print the IP Address
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());
	delay(1000);

	//Check the door status every 1 second
	ticker.attach(1, CheckDoorStatus);
}

void loop() 
{
	if (SendUpdate)
	{
		SendStatusUpdate();
		SendUpdate = false;
	}

	client = server.available();
	if (client)  // if you get a client
	{
		char getLine[128];
		int i = 0;
		bool getLineFound = false;
		bool currentLineIsBlank = true;

		Serial.println("");
		Serial.println("new client");

		while (client.connected()) // loop while the client's connected
		{
			if (client.available()) // if there's bytes to read from the client
			{
				char c = client.read(); // read 1 byte from client
				Serial.print(c);

				if (!getLineFound && i < sizeof(getLine))
				{
					//save the char to getLine
					getLine[i] = c;
					i++;
				}

				if (c == '\n' && currentLineIsBlank) // respond to client only after last line is received, last line is blank and ends with \n
				{
					ProcessRequest(getLine);
					SendResponse(client);
					break;
				}

				if (c == '\n') // end of the line, next char read will be a new line
				{
					if (!getLineFound) //the GET line is the first line from the client, save it for later
					{
						getLineFound = true;

						//strip off the HTTP/1.1 from the end of the getLine
						const char *ptr = strstr(getLine, "HTTP/1.1");
						if (ptr)
							getLine[ptr - getLine - 1] = '\0';
					}

					currentLineIsBlank = true;
				}
				else if (c != '\r') //text char received
				{
					currentLineIsBlank = false;
				}
			} //end if (client.available())
		} //end while (client.connected())


		// close the connection
		delay(1); //allow client to receive the data
		client.stop();
		Serial.println("client disconnected");
	} //end if (client)
}

void CheckDoorStatus()
{
	int newStatus = digitalRead(statusPin);
	if (newStatus == Status)
	{
		//new status is the same as the current status, return
		return;
	}
	else
	{
		Status = newStatus;
		Serial.print("Status has changed to: ");
		if (Status == CLOSED)
		{
			Serial.println("CLOSED");
		}
		else
		{
			Serial.println("OPEN");
		}

		SendUpdate = true;
	}
}

void SendResponse(WiFiClient& client)
{
	client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	client.println("<!DOCTYPE HTML>\r\n<html>\r\n<head>\r\n<title>Garage Door Monitor</title><link rel=\"shortcut icon\" type=\"image / x - icon\" href=\"http://arduino.cc/en/favicon.png\" /></head>");
	client.println("<body>\r\n<center>\r\n<h1>Garage Door Status</h1>\r\n<hr>\r\n<br>\r\nGarage Door is now: ");

	if (Status == CLOSED) {
		client.print("<span style='background-color:#00FF00; font-size:18pt'>Closed</span>\r\n<br>");
	}
	else {
		client.print("<span style='background-color:#FF0000; font-size:18pt'>Open</span>\r\n<br>");
	}

	client.println("</center>\r\n</html>");
}

void ProcessRequest(char* getLine)
{
	if (strstr(getLine, "GET /ACTIVATE") != NULL)
	{
		Serial.println("Activating garage door");
		SendUpdate = true; //refresh the client
		ActivateDoor();
	}
	
}

void SendStatusUpdate()
{
  // Connect to the Domoticz server
	if (client.connect(updateserver, updateport)) 
	{
		digitalWrite(ledPin, LOW);
		Serial.println("connected to update server");
		// Send the Domoticz API request:
		client.print("GET /json.htm?type=command&param=udevice&idx=12&nvalue=");
		if (Status == CLOSED)
		{
			client.print("0");
		}
		else
		{
			client.print("1");
		}
		client.println(" HTTP/1.1");
		client.println("Host: www.google.com");
		client.println("Connection: close");
		client.println();

		client.println();

		while (client.connected() && !client.available()) delay(1); //waits for data
		while (client.connected() || client.available()) { //connected or data available
			char c = client.read();
			Serial.print(c);
		}

		Serial.println();
		Serial.println("disconnecting.");
		Serial.println("==================");
		Serial.println();
		client.stop();
		digitalWrite(ledPin, HIGH);
	}
	else
	{
		Serial.println("connection to update server failed");
		Serial.println();
	}
}

void ActivateDoor()
{
	Serial.println("Turning relay on");
	digitalWrite(activatePin, HIGH);
	delay(1000);
	Serial.println("Turning relay off");
	digitalWrite(activatePin, LOW);
}


/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using the WiFi module.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * Board with NINA module (Arduino MKR WiFi 1010, MKR VIDOR 4000 and UNO WiFi Rev.2)

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */


#include <SPI.h>
#include <WiFiNINA.h>


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "MCHP1";        // your network SSID (name)
char pass[] = "pic.avr.sam";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// individual srings for data.
char current_scan[7],current_temp[7],current_humidity[7],current_pressure[7],current_speed[7],current_direction[7],current_uv[7];
void data_stream_parse(void);

float tempc; 
float tempf; 
float humidity; 
float baromin; 
float dewptf;
float UVindex;
int winddir;
int windspeedmph;

  
void read_weather_data(void);


char SERVER[] = "weatherstation.wunderground.com";    
char WEBPAGE[] = "GET /weatherstation/updateweatherstation.php?";
char ID[] = "KCASUNNY216";
char PASSWORD[] = "jk0343uo";


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):

WiFiClient client;

void setup() 
{
  //Initialize serial and wait for port to open:
  Serial.begin(38400);
  Serial1.begin(38400);		// external data stream from sesnor node
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }


  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  
}

void loop() 
{
  
  
  
  read_sensor_data();
  data_stream_print();
  
  //Send data to Weather Underground
 if (client.connect(SERVER, 80)) 
 { 
      
	read_weather_data();
   Serial.println("Sending DATA ");
      
  
    client.print(WEBPAGE); 
    client.print("ID=");
    client.print(ID);
    client.print("&PASSWORD=");
    client.print(PASSWORD);
    client.print("&dateutc=");
    client.print("now");    //can use instead of RTC if sending in real time
  
    client.print("&tempf=");
    client.print(current_temp);
    client.print("&baromin=");
    client.print(current_pressure);
    
	
	// client.print("&dewptf=");
    // client.print(dewptf);
	
	client.print("&windspeedmph=");
	client.print(current_speed);
	client.print("&winddir=");
	client.print(current_direction);
	
    client.print("&humidity=");
    client.print(current_humidity);
    client.print("&uv=");
    client.print(current_uv);
    client.print("&action=updateraw");//Standard update
    
    client.print(" HTTP/1.0\r\n");
  	client.print("Accept: text/html\r\n");
  	client.print("Host: ");
  	client.print(SERVER);
  	client.print("\r\n\r\n");
    client.println();
    Serial.println("Upload complete");
	  get_server_response();
      
      
   }//End send loop 
   else 
   {
      Serial.println(F("Connection failed"));
	  client.stop();
      return;
   }
    
	
	client.stop();
    delay(8000); 
 
 
}

void get_server_response(void)
{
	
	Serial.println("<< ");
	
	if (client.available())
	{
		char c = client.read();
		Serial.print(c);
	}
	
	Serial.println(" >>");
	
	
	
}





void data_stream_print(void)
{
	
	Serial.println("Waiting for sensor data stream");
	read_sensor_data();
	
	Serial.println("extracted data");
	Serial.print(current_scan);
	Serial.print(' ');
	Serial.print(current_temp);
	Serial.print(' ');
	Serial.print(current_humidity);
	Serial.print(' ');
	Serial.print(current_pressure);
	Serial.print(' ');
	Serial.print(current_speed);
	Serial.print(' ');
	Serial.print(current_direction);
	Serial.print(' ');
	Serial.println(current_uv);
	
	
}


void read_weather_data(void)
{
	tempf = 11.0 * 9.0/5.0;
	tempf += 32.0;
    humidity = 54.0; 
	UVindex = 0.0;
	winddir = 254;
	windspeedmph = 2.0;
	
}

char frame_buffer[100];




enum {FRAME_IDLE,FRAME_READ};

void read_sensor_data(void)
{
	uint8_t done = false;
	uint8_t index,data_index;
	uint8_t frame_state = FRAME_IDLE;
	char buffer_data;
	
	for(index = 0;index < 100;index++)		// clear frame buffer
		frame_buffer[index] = 0;
	
	while(!done)
	{
		
		// if(read_rx_buffer(&buffer_data))
		if(Serial1.available() > 0)
		{
				
			buffer_data = Serial1.read();	
		
			if(buffer_data == '@'&& frame_state == FRAME_IDLE)
			{
				index = 0;
				frame_state = FRAME_READ;
				frame_buffer[index] = buffer_data;
				index++;
				
			}
			
			else if(buffer_data == '*' && frame_state == FRAME_READ)
			{
				frame_buffer[index++] = '*';
				frame_buffer[index] = 0;
				frame_state = FRAME_IDLE;
				done = true;
				
				
			}
			else frame_buffer[index++] = buffer_data;
		}		
			
	} // end while not done
	Serial.println(frame_buffer);
	
	
	// extract current scan
	
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 's')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_scan[data_index++] = frame_buffer[index++];
			current_scan[data_index] = 0;
			done = true;
			
		}
		
	}
	
	// extract temp
	
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 't')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_temp[data_index++] = frame_buffer[index++];
			current_temp[data_index] = 0;
			done = true;
			
		}
		
	}
	
	// extract humidity
	
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 'h')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_humidity[data_index++] = frame_buffer[index++];
			current_humidity[data_index] = 0;
			done = true;
			
		}
		
	}
	
	// extract pressure
	
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 'p')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_pressure[data_index++] = frame_buffer[index++];
			current_pressure[data_index] = 0;
			done = true;
			
		}
		
	}
	
	// extract wind speed
	
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 'v')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_speed[data_index++] = frame_buffer[index++];
			current_speed[index] = 0;
			done = true;
			
		}
		
	}
	
	
	// extract wind direction
	
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 'd')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_direction[data_index++] = frame_buffer[index++];
			current_direction[index] = 0;
			done = true;
			
		}
		
	}
	
	// extract uv index
	done = false;
	index = 0;
	while(!done)
	{
		if(frame_buffer[index++] == 'i')
		{
			index++;			// move past ':'			
			data_index = 0;
			while(frame_buffer[index] != ' ')
				current_uv[data_index++] = frame_buffer[index++];
			current_uv[index] = 0;
			done = true;
			
		}
		
	}
	
	
} // end read_sensor_data



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

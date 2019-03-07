/*

	weather station v 1.0
	Bob Martin / Microchip Technology 
	Feb 2019
	
	Credit / Kudos / Acknowledgment Section
	 	
		 This is a library for the BME280 humidity, temperature & pressure sensor
		 Designed specifically to work with the Adafruit BME280 Breakout
		 ----> http://www.adafruit.com/products/2650
		 These sensors use I2C or SPI to communicate, 2 or 4 pins are required
		 to interface. The device's I2C address is either 0x76 or 0x77.
		 Adafruit invests time and resources providing this open source code,
		 please support Adafruit andopen-source hardware by purchasing products
		 from Adafruit!
		 Written by Limor Fried & Kevin Townsend for Adafruit Industries.
		 BSD license, all text above must be included in any redistribution
		 
		 
		 


*/





#include <Arduino.h>
#include <stdio.h>
#include <string.h>



// Arduino specific libraries

#include "TimerOne.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6075.h>



#define SEALEVELPRESSURE_HPA (1013.25)
#define TRUE	1
#define FALSE	0
#define pWIND_SPEED 2
#define pWIND_DIR	A0

//Beginning of Auto generated function prototypes by Atmel Studio
void printValues();
//End of Auto generated function prototypes by Atmel Studio

enum  {WIND_SPEED_IDLE,WIND_SPEED_INIT, WIND_SPEED_NEG_EDGE, WIND_SPEED_DEBOUNCE1,WIND_SPEED_POS_EDGE,WIND_SPEED_DEBOUNCE2};

float wind_voltage[16] = {0.32, 0.41, 0.45, 0.62, 0.90, 1.19, 1.40, 1.98, 2.25, 2.93, 3.08, 3.43, 3.84, 4.04, 4.62, 4.78};
float wind_degrees[16] = {112.5, 67.5, 90,  157.5, 135, 202.5, 180 ,22.5,  45,  247.5, 225, 337.5, 0,   292.5, 270,  315};

uint8_t new_wind_sample = FALSE;
uint16_t pulse_count = 0;
uint8_t speed_state = WIND_SPEED_INIT;
uint8_t wind_sample_ready = FALSE;


struct _weather_data
{
	float wind_speed;
	float wind_dir;
	float temp;
	float uv_index;
	float humidity;	
	float pressure;
	
	
} sensor_data,*psensor_data;;
	

int sample_wind_dir(void);
uint8_t sample_wind_speed(void);
void clear_sample_timer(void);
uint16_t read_pulse_count(void);
void set_sample_enable(void);
uint8_t read_sample_enable(void);
void start_sample(void);
uint8_t read_sample_status(void);
void timer1_init(void);
void T1_isr();
void sample_bme280(void);
void sample_uv(void);
void tx_data(void);

// uv sensor
Adafruit_VEML6075 uv = Adafruit_VEML6075();

// BME280 sensor
Adafruit_BME280 bme280; // I2C




void setup()
 {
	
	bool status;

    psensor_data = &sensor_data;				// init the pointer

	Serial.begin(38400);
    Serial.println(F("\r\nWeather Station v1.0"));
  
    /*
    status = bme280.begin();  
    if (!status) {
        Serial.println(">> BME280 sensor problem");
        while (1);
    }

	else Serial.println(">> BME280 sensor init");

    if (! uv.begin()) 
        {
          Serial.println(">> VEML6075 sensor problem");
          while(1);
        }
    else Serial.println(">> VEML6075 sensor init");
    */
	timer1_init();
	Serial.println(">> Wind sensor init");
	
	psensor_data->wind_dir = 234.0;
	psensor_data->wind_speed = 11.2;
	
	
   
}


void timer1_init(void)
{
	
	// timer 1 init
	
	Timer1.initialize(5000); 		// 5 ms (5000 us) tick on timer 1
	Timer1.attachInterrupt( T1_isr ); // attach the service routine here
	
}


void format_stream(void);


void loop() 
{ 
    // sample_bme280();
	// sample_uv();
	// sample_wind_speed();
	// sample_wind_dir();
	format_stream();
	tx_data();
    delay(1000);
	
}

#define MILLIBAR2INHG .02952

void sample_bme280(void)
{
	
	static float sensor_val;
	
	sensor_val = bme280.readTemperature();
	sensor_val *= 1.80;
	sensor_val += 32.0;
	psensor_data->temp = sensor_val;
	 
	psensor_data->humidity = bme280.readHumidity();
		
	sensor_val = bme280.readPressure() / 100.0;
	sensor_val *= MILLIBAR2INHG;
	psensor_data->pressure = sensor_val;

}


void sample_uv(void)
{
	
    psensor_data->uv_index = uv.readUVI();
		
}


//@s:308000 t:100.00 h:100.00 p:30.18 v:11.20 d:234.00 i:10.00 *   

void format_stream(void)
{
	
	// populate synthetic data
	
	psensor_data->temp = 22.00;
	psensor_data->humidity = 100.00;
	psensor_data->pressure = 29.92;
	psensor_data->wind_speed = 45.12;
	psensor_data->wind_dir = 124.3;
	psensor_data->uv_index = 1.22;
	
	
	
	
	
}




void tx_data(void) 
{
    
	static unsigned long scan_count = 0;
	
	
	Serial.print("@s:");
	Serial.print(scan_count++);
	Serial.print(" t:");
	Serial.print(psensor_data->temp);
	Serial.print(" h:");
	Serial.print(psensor_data->humidity);
	Serial.print(" p:");
	Serial.print(psensor_data->pressure);
	Serial.print(" v:");
	Serial.print(psensor_data->wind_speed);
	Serial.print(" d:");
	Serial.print(psensor_data->wind_dir);
	Serial.print(" i:");
	Serial.print(psensor_data->uv_index);
	Serial.println(" *");
	
}




#define ADC_VREF	5.0
#define ADC_DELTA	0.02

int sample_wind_dir(void)
{
	float sensorValue,wdir;
	int index;
	
	sensorValue = analogRead(pWIND_DIR);
	
	//Scale to 5 V
	sensorValue = (sensorValue*ADC_VREF)/1024.0;
	
	for(index = 0; index< 16; index++)
	{
		if(sensorValue <= wind_voltage[index] - ADC_DELTA)
		break;
	}
	
	//Correct the index
	if (index)
		index--;
	psensor_data->wind_dir = wind_degrees[index];
	
}


uint8_t sample_wind_speed(void)
{
	
	uint16_t pulses;
	static float speed_sample;
	
	
	start_sample();
	
	while(!read_sample_status());
	
	pulses = read_pulse_count();
	speed_sample = (float)pulses;
	speed_sample  = speed_sample * 1.492 / 5.0;
	psensor_data->wind_speed = speed_sample;
	
	return(TRUE);
} // end sample wind


unsigned long debounce_timer,sample_timer;


void clear_sample_timer(void)
{
	
	noInterrupts();
	sample_timer = 0;
	interrupts();
}


uint16_t read_pulse_count(void)
{
	uint16_t pulses;
	
	noInterrupts();
	pulses = pulse_count;
	interrupts();
	return(pulses);
}


void start_sample(void)
{
	noInterrupts();
	speed_state = WIND_SPEED_INIT;
	new_wind_sample = FALSE;
	interrupts();
}

uint8_t read_sample_status(void)
{
	uint8_t	sts;
	
	noInterrupts();
	sts = new_wind_sample;
	interrupts();
	return(sts);
	
}




void T1_isr()
{
	
	PORTB |= 0x01;
	
	debounce_timer++;
	
	if(speed_state != WIND_SPEED_IDLE)
	sample_timer++;
	
	if(sample_timer > 999)	// 5 seconds elapsed
	{
		
		sample_timer = 0;
		speed_state = WIND_SPEED_IDLE;
		new_wind_sample = TRUE;
	}
	
	
	switch(speed_state)
	{
		
		case WIND_SPEED_IDLE:
		
		break;
		

		case WIND_SPEED_INIT:
		pulse_count = 0;
		sample_timer = 0;
		speed_state = WIND_SPEED_NEG_EDGE;
		break;
		
		case WIND_SPEED_NEG_EDGE :
		if(!(PINB & (1 << 2)))		// check for falling edge, direct register read
		{
			
			debounce_timer = 0;
			speed_state = WIND_SPEED_DEBOUNCE1;
			
		}
		
		break;
		
		case WIND_SPEED_DEBOUNCE1:
		if(debounce_timer == 2)			// 10ms de-bounce
		{
			if(PINB & (1 << 2))	// false edge, go back
			{
				speed_state = WIND_SPEED_NEG_EDGE;
				
			}
			else				// valid pos edge, look for rising edge
			{
				pulse_count++;
				speed_state = WIND_SPEED_POS_EDGE;
			}
			
		}
		break;
		
		case WIND_SPEED_POS_EDGE:
		
		if(PINB & (1 << 2))	// detected rising edge
		{
			
			debounce_timer = 0;
			speed_state = WIND_SPEED_DEBOUNCE2;
			
		}
		break;
		
		case WIND_SPEED_DEBOUNCE2:
		
		if(debounce_timer == 2)
		{
			if(PINB & (1 << 2)) 				// true positive edge
			speed_state = WIND_SPEED_NEG_EDGE;
			
			else speed_state = WIND_SPEED_POS_EDGE; // false positive edge, go back
			
		}
		
		break;
		
		
	} // end switch
	PORTB &= ~(0x01);
} // end isr

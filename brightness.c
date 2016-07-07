/*
 * brightness.c
 * 
 * set the brightness of the pi-top
 * 
 * usage (new_value is a screen brightness value between 3 and 10
 * 	brightness increase
 *  brightness decrease
 *  brightness new_value
 * 
 * Copyright 2016  rricharz
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <string.h>
#include <wiringPiSPI.h>

#define MAXCOUNT 10		// maximum number of spi transfer attemps

#define LIDBITMASK		0x04
#define	SCREENOFFMASK	0x02
#define PARITYMASK		0x80
#define BRIGHTNESSMASK  0x78
#define SHUTDOWNMASK	0X01

int lidbit;
int screenoffbit;
int parity;
int brightness;
int shutdown;

//////////////////////////////
int parity7(unsigned char data)
//////////////////////////////
// Calclate the parity of bits 0 - 6
{
	int i;
	int p = 0;
	for (i = 0; i < 7; i++) {
		if (data & 1) p = !p;
		data = data >> 1;
	}
	return p;
}

///////////////////////////////
int analyze(unsigned char data)
///////////////////////////////
// analyze data byte and set global variables
// return 1 of parity is ok
// Sending: bit 8 = parity of bit 1-7
{
	lidbit			= (data & LIDBITMASK) != 0;
	screenoffbit	= (data & SCREENOFFMASK) != 0;
	parity			= (data & PARITYMASK) != 0;
	brightness		= (data & BRIGHTNESSMASK) >> 3;
	shutdown		= (data & SHUTDOWNMASK) != 0;
	
	// printf("lid = %d, screen = %d, parity = %d, shutdown = %d, brightness = %d\n", lidbit, screenoffbit, parity, shutdown, brightness);
	
	return (parity7(data) == parity);
}

///////////////
int calculate()
///////////////
// Calculate data byte using global variables
// Set brightness and status parity bits
// Receiving: bit 8 = brightness parity, bit 3 = status parity
{
	int data = brightness << 3;
	if (parity7(brightness))
		data += PARITYMASK;
	if (shutdown)
		data += SHUTDOWNMASK;
	if (screenoffbit)
		data += SCREENOFFMASK;
	if (parity7(data & 3))
		data += LIDBITMASK;		// parity ofthe two state bits
	return data;		
}

///////////////////////////////
int main(int argc, char **argv)
///////////////////////////////
{
	unsigned char data, new_data;
	int count, ok;
	
	printf("brightness version 1.0\n");
	
	int spi = wiringPiSPISetup(1, 9600);
	if (spi < 0) {
	  printf("Cannot initialize spi driver\n");
	  return 1;
	}
	 
	// printf("spi handle = %d\n", spi);
	
	// send 0xFF and receive current status of pi-top-hub
	count = 0;
	data = 0xff;
	printf("Sending: 0x%X\n", data);
	do {
		data = 0xff;
		ok = wiringPiSPIDataRW(1, &data, 1);
		if (ok) {
			ok = analyze(data);
		}
	}
	while ((!ok) && (count++ < MAXCOUNT));
	// printf("count = %d\n", count);
	
	if (ok) {
		printf("Receiving: 0x%X\n", data);
		printf("Current brightness = %d\n", brightness);
		
		// calculate new brightness
		if (argc == 2) {
			if (strcmp(argv[1],"increase") == 0) {
				brightness++;
			}
			else if (strcmp(argv[1],"decrease") == 0) {
				brightness--;
			}
			else {
				sscanf(argv[1],"%d", &brightness);
			}
		}
		else
			brightness = 9;		
		if (brightness > 10)
			brightness = 10;
		if (brightness < 3)
			brightness = 3;
		printf("Requested brightness = %d\n", brightness);
		
		// calculate data to send
		shutdown = 0;
		screenoffbit = 0;		
		new_data = calculate();
				
		// send new data until accepted
		count = 0;
		data = new_data;
		printf("Sending: 0x%X\n", data);
		do {
			data = new_data;
			ok = wiringPiSPIDataRW(1, &data, 1);
			if (ok) {
				ok = (data & BRIGHTNESSMASK) == (new_data & BRIGHTNESSMASK);
			}
		}
		while ((!ok) && (count++ < MAXCOUNT));
		// printf("count = %d\n", count);
		if (ok) {
			printf("Receiving: 0x%X\n", data);
			printf("New brightness = %d\n", brightness);
		}
	}
	else
	  printf("reading current brightness not successful\n");
	return 0;
}


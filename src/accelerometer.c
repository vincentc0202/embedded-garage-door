#include "SPI.h"
#include "SysTimer.h"
#include "accelerometer.h"

void accWrite(uint8_t addr, uint8_t val){
	// byte 1: command byte
	// bit 15: write (R/W) = 0
	// bit 14: MB = 0
	// bits 13-8: command bits
	// byte 2: data byte (bits 7-0)
	uint16_t data = (0 << 15 | 0 << 14 | addr << 8 | val);
	SPI_Transfer_Data(data);
}

uint8_t accRead(uint8_t addr){
	// byte 1: command byte
	// bit 15: read (R/!W) = 1
	// bit 14: MB (always set to 0)
	// bits 13-8: address
	// byte 2: empty (bits 7-0)
	uint16_t data = (1 << 15 | 0 << 14 | addr << 8);
	return SPI_Transfer_Data(data);
}

void initAcc(void){
	// These are found in pages 26-28 of the Acc datasheet
	// set full range mode (B = 1011 bc last two bits give it +- 16 g range
	accWrite(0x31, 0x0B);
	// enable measurement
	accWrite(0x2D, 0x08);
	// BW to 100 Hz
	accWrite(0x2C, 0x0A);
}

void readValues(double* x, double* y, double* z){
	// read values into x,y,z using accRead
	int16_t raw_x = (accRead(0x33) << 8) | accRead(0x32); 
	int16_t raw_y = (accRead(0x35) << 8) | accRead(0x34); 
	int16_t raw_z = (accRead(0x37) << 8) | accRead(0x36); 

	double scale = 0.004;  // Scale factor in g/LSB for full resolution mode (page 1)
	
	*x = raw_x * scale;
	*y = raw_y * scale;
	*z = raw_z * scale;
}

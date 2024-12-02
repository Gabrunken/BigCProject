#include <bitscalculations.h>
#include <bigc.h>

unsigned char bigc_GetBitsOn(unsigned char byte)
{
	unsigned char bitsSet = 0;
	for(unsigned char bit = 0; bit < 8; bit++)
	{
		bitsSet += byte & 0x00000001; //"byte" & 0x00000001 becomes equal to 1 if the first bit of "byte" is set to 1.
		byte >>= 1; //Shift every bit to the right
	}

	return bitsSet;
}

unsigned char bigc_GetBitInPosition(unsigned char byte, unsigned char position)
{
	return (byte & (0x00000001 << position)) == BIGC_TRUE;
}
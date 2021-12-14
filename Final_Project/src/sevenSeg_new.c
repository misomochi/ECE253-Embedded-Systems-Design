#include "sevenSeg_new.h"

static const int digit_segs[10] =  {
		0b00111111,	 //0 //GNU C extension allows binary constants
		0b00000110,	 //1
		0b01011011,  //2
		0b01001111,  //3
		0b01100110,  //4
		0b01101101,  //5
		0b01111101,  //6
		0b00000111,  //7
		0b01111111,  //8
		0b01101111,  //9
};

/*void sevenseg_draw_digit (int position, int value)
{
	int segs, segs_mask, digit_mask, combined_mask;

	segs  = digit_segs[value];
	segs_mask = 127^segs;
	digit_mask = 255 ^ (1<<position);
   	combined_mask = segs_mask | (digit_mask << 7);

   	Xil_Out32(XPAR_SEVENSEG_0_S00_AXI_BASEADDR, combined_mask);

   	return;
}*/

void sevenseg_draw_digit (int position, int segs) {
	int segs_mask, digit_mask, combined_mask;

	segs_mask = 127 ^ segs;
	digit_mask = 255 ^ (1 << position);
	combined_mask = segs_mask | (digit_mask << 7);

	Xil_Out32(XPAR_SEVENSEG_0_S00_AXI_BASEADDR, combined_mask);

	return;
}

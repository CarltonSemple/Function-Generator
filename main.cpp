#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

// Read function
byte read() {
	// flag goes to 1 upon receiving
	// loop until it receives
	if(SCI1SR1_RDRF != 1)
		return 0;
	
	// return the data
	return SCI1DRL;
}

void transmit(byte data){
	while(SCI1SR1_TDRE != 1);
	SCI1DRL = data;  
}

// set up SPI control register 1
void setup_SPI1()
{
	SPI0CR1_SPE = 1;  // Enable SPI 1
	SPI0CR1_MSTR = 1; // make microcrontroller the master
	SPI0CR1_CPOL = 1; // polarity
	SPI0CR1_CPHA = 0; // phase
	SPI0CR1_SSOE = 1; // slave select output enable 
	SPI0CR2_MODFEN = 1; // Mode Fault Enable Bit
	SPI0CR2_XFRW = 1; // 1 = 16-bit 
}

void startTimer(int delayCount){
	TC0 = TC0 + delayCount; // count this many cycles
	while(!TFLG1_C0F);  // loop while the timer isn't finished
}

// Convert character array to integer
int toNumber(char chArray[], int digitCount)
{
	// Act differently based on the number of digits
	int number = -1;
	int hundredths = -1, tenths = -1, ones = -1;

	// lowest array index has the highest digit

	switch (digitCount)
	{
	case 1:
		ones = chArray[0] - '0';
		number = ones;
		break;
	case 2:
		ones = chArray[1] - '0';	tenths = chArray[0] - '0';
		number = 10 * tenths + ones;
		break;
	case 3:
		ones = chArray[2] - '0';	tenths = chArray[1] - '0';	hundredths = chArray[0] - '0';
		number = 100 * hundredths + 10 * tenths + ones;
		break;
	default:	// shouldn't occur
		break;
	}

	return number;
}

int algorithm_1_100_hz(int inputNum)
{
	// input Number is divisor
	return 18000 / inputNum;	// this is the number that is given to the delay timer
}

int algorithm_100hz_1khz(int inputNum)
{
	return (inputNum - 100) / 10;
}



                 
byte sineLookup[34] = { 127.5000 ,151.3911,  174.4359,  195.8179,  214.7798,  230.6497,  242.8654,  250.9944,
                            
254.7484,  253.9946,  248.7597,  239.2291,  225.7404,  208.7716,  188.9236,  166.8997,

143.4800,  119.4942,   95.7920,   73.2131,   52.5574 ,  34.5565,   19.8482,    8.9535 ,

2.2584, 0 ,   2.2584 ,   8.9535 ,  19.8482,   34.5565 ,  52.5574 ,  73.2131 ,

95.7920,  119.4942}; 
                  
static byte triangleLookup[36] = { 127.5000,  142.4998,  157.4995,  172.4993,  187.4990,  202.4988,  217.4985,  232.4983,	247.4980,

254.9993,  239.9995,  224.9997,  210.0000,  195.0002,  180.0005,  165.0007,  150.0010, 135.0012,  120.0015,  105.0017,   90.0019,   75.0022,   60.0024,   45.0027,   30.0029,

15.0032,    0.0034, 0.0023,   15.0020,   30.0018,   45.0015,   60.0013,   75.0010,   90.0008,  105.0005,	120.0003
};
           
    
static byte squareLookup[35] = {  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};  
      

void main(void) {
	// to begin, 
	byte i = 0;
	byte divisor, count =0 , subtractor;
	char receivedData;  // for reading from realterm
	byte letter=97, test;
	int userInput = 1; // the input # from the user
	int three, two, one;
	//double forty = 40000000, thirtyfour = 34;
	//int timer = 1;
	byte temp = -1;
	byte inverse;
	
	TIOS = 0x01; // set timers to output
	
	TSCR1_TEN = 1; // enable timer
	TSCR1_TFFCA = 1; // clear
	// delay amount
	
	TSCR2_PR2 = 1;  // enable a smaller measurement. dividing by 16   
	TSCR2_PR1 = 0;      
	TSCR2_PR0 = 1;      
                         
	// set up SPI 0
	setup_SPI1();
	
	// Receiving data ********************
	// set baud rate
	SCI1BDH = 0x00;
	SCI1BDL = 0x28;
	//SCI1BD = 260;
	// Enable Receive and transmission
	SCI1CR2_RE = 1;
	SCI1CR2_TE = 1; // enable
	                      
	// set MODRR to 0x50 (Port M)
	MODRR = 0x50;
  
   
	// EXTRAS   Speedup
	
	SYNR = 0x49;
	REFDV = 0x42;
	while(!CRGFLG_LOCK);
	
	// wait for lock
	//CLKSEL_PLLSEL = 1; // clock select register's highest bit
	CLKSEL = 0x80;
    

	for(;;) 
	{
		test = read(); // letter
		if(test != 0)
		{
			if((test != 97) && (test != 115) && (test != 100)) 
		   		temp = test;
			else
			{
				temp = -1;
				letter = test;
			}
		}

		if(temp == 49){ // 1 - decrement by 10
			userInput = userInput - 10;
			temp = 0;
		}
		
		if(temp == 50){ // 2 - increment by 10
			userInput = userInput + 10;
			temp = 0;
		}
    
		if(temp == 51){  // 3 - decrement by 1
			userInput = userInput - 1;
			temp = 0;      
		}
    
		if(temp == 52){  // 4 - increment by 1
			userInput = userInput + 1;
			temp = 0;      
		}
		
		if(temp == 53){  // 5 - decrement by 100
			userInput = userInput - 100;
			temp = 0;      
		}
		
		if(temp == 54){  // 6 - increment by 100
			userInput = userInput + 100;
			temp = 0;      
		}

		while(SPI0SR_SPTEF != 1);
     
		if(letter == 97)
		{
			// a: Sine ********************************************
			// Give the SPI data register any command + data
			// SEND
			SPI0DR = 0x0F00 +  sineLookup[i]; // delay + offset
			i++;
			if(i==33)
			i=0; 
		} 
		else if(letter == 115)
		{
			// s: Square ********************************************
			// Give the SPI data register any command + data
			// SEND
			SPI0DR = 0x0F00 +  squareLookup[i]; // delay + offset
			i++;
			if(i==34)
			i=0;
		}
		else if(letter == 100) 
		{
			// d: Triangle ********************************************
			// Give the SPI data register any command + data
			// SEND
			SPI0DR = 0x0F00 +  triangleLookup[i]; // delay + offset
			i++;
			if(i==35)
			i=0;
		}
		
		startTimer(36764/userInput); 
	} /* loop forever */
}

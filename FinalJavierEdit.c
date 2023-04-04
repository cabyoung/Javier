/*
	author: Courtney Young
	version: v3.0
  Adapted from: Subharthi Banerjee, Ph.D. and Matthew Boeding

	README

/// The sole reason to provide this code is to make your TFTLCD (ILI9341)
/// up and running

/// Note: Most of the code is in one place. This is not ideal and may be changed
// in the future

/// Use C or inline assembly program as you please.

/// ** the code uses P0 for 8-bit interface
/// ** IOM --> P3^4
/// ** CD --> P3^5
////  I recommend leaving these definitions for UART implementation later.
////
/// ** RD --> P3^7
/// ** WR --> P3^6

/// Refer to the header file to change decoding addresses for your specific design.

/// Please do not post any of the code from this course to GITHUB.

*/


///  *************** IMPORTANT *********************

// It may need redfinition of pins like
// #include <reg51.h> // change microcontroller header when using AT89C55WD


# include "ecen4330lcdh.h"
# include "font.h"

// keypad configuration
uint8_t keypad[4][4] =	{{'1','2','3','A'},
				{'4','5','6','B'},
				{'7','8','9','C'},
				{'F','0','E','D'} };
uint8_t colloc, rowloc;
// store it in a variable the lcd address
__xdata uint8_t* lcd_address = (uint8_t __xdata*) __LCD_ADDRESS__;
__xdata uint8_t* seg7_address = (uint8_t __xdata*) __SEG_7_ADDRESS__;
__xdata uint8_t* phot_address = (uint8_t __xdata*) __PHOT_ADDRESS__;
__xdata uint8_t* temp_address = (uint8_t __xdata*) __TEMP_ADDRESS__;
__xdata uint8_t* read_ram_address;


 # define write8inline(d) {			\
 	IOM = 1;							\
	*lcd_address = d; 						\
	IOM = 0;							\
}


# define write8 write8inline
// data write
# define write8DataInline(d) {	\
	CD = 1; 					\
	write8(d);					\
}
// command or register write
# define write8RegInline(d) {	\
	CD = 0; 					\
	write8(d);					\
}

// inline definitions
# define write8Reg write8RegInline
# define write8Data write8DataInline



uint16_t cursor_x, cursor_y;  /// cursor_y and cursor_x globals
uint8_t textsize, rotation; /// textsize and rotation
uint16_t
    textcolor,      ////< 16-bit background color for print()
    textbgcolor;    ////< 16-bit text color for print()
uint16_t
    _width,         ////< Display width as modified by current rotation
    _height;        ////< Display height as modified by current rotation
volatile unsigned char received_byte=0;
volatile unsigned char received_flag=0;

void ISR_receive() __interrupt (4){
	if (RI == 1){
		received_byte = SBUF;
		RI = 0;
		received_flag = 1;
	}
}

inline void iowrite8(uint8_t __xdata* map_address, uint8_t d) {
	IOM = 1;
	*map_address = d;
	IOM = 0;
}

void delay (int16_t d) /// x 1ms
{
	int i,j;
	for (i=0;i<d;i++) /// this is For(); loop delay used to define delay value in Embedded C
	{
	for (j=0;j<1000;j++);
	}
}

void writeRegister8(uint8_t a, uint8_t d) {
	//IOM = 0;
	CD = __CMD__;
	write8(a);
	CD = __DATA__;
	write8(d);
	//IOM = 1;
}



void writeRegister16(uint16_t a, uint16_t d){
	uint8_t hi, lo;
 	hi = (a) >> 8;
 	lo = (a);
	//IOM = 0;
 //	CD = 0;
 	write8Reg(hi);
 	write8Reg(lo);
  	hi = (d) >> 8;
  	lo = (d);
  	CD = 1 ;
  	write8Data(hi);
  	write8Data(lo);
//	IOM =1;
}


void setCursor(uint16_t x, uint16_t y){
	cursor_x = x;
	cursor_y = y;
}
// set text color
void setTextColor(uint16_t x, uint16_t y){
	textcolor =  x;
	textbgcolor = y;
}

// set text size
void setTextSize(uint8_t s){
	if (s > 8) return;
	textsize = (s>0) ? s : 1 ;
}

void setRotation(uint8_t flag){
	switch(flag) {
		case 0:
			flag = (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
			_width = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
		case 1:
			flag = (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
			_width = TFTHEIGHT;
			_height = TFTWIDTH;
			break;
		case 2:
			flag = (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
			_width = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
	  case 3:
			flag = (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
			_width = TFTHEIGHT;
			_height = TFTWIDTH;
			break;
		default:
			flag = (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
			_width = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
	}
	writeRegister8(ILI9341_MEMCONTROL, flag);
}

// set address definition
void setAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2){
	//IOM =0;
	write8Reg(0x2A);
	write8Data(x1 >> 8);
	write8Data(x1);
	write8Data(x2 >> 8);
	write8Data(x2);

	write8Reg(0x2B);
	write8Data(y1 >> 8);
	write8Data(y1);
	write8Data(y2 >> 8);
	write8Data(y2);
	//write8Reg(0x2C);
  //IOM =1;


}

void TFT_LCD_INIT(void){
	//char ID[5];
	///int id;
	_width = TFTWIDTH;
	_height = TFTHEIGHT;

	// all low
	IOM = 0;
	//RDN = 1;
	CD = 1;

	write8Reg(0x00);
	write8Data(0x00);write8Data(0x00);write8Data(0x00);
	//IOM = 1;
	delay(200);

	//IOM = 0;

	writeRegister8(ILI9341_SOFTRESET, 0);
    delay(50);
    writeRegister8(ILI9341_DISPLAYOFF, 0);
    delay(10);






    writeRegister8(ILI9341_POWERCONTROL1, 0x23);
    writeRegister8(ILI9341_POWERCONTROL2, 0x11);
    write8Reg(ILI9341_VCOMCONTROL1);
		write8Data(0x3d);
		write8Data(0x30);
    writeRegister8(ILI9341_VCOMCONTROL2, 0xaa);
    writeRegister8(ILI9341_MEMCONTROL, ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
    write8Reg(ILI9341_PIXELFORMAT);
	write8Data(0x55);write8Data(0x00);
    writeRegister16(ILI9341_FRAMECONTROL, 0x001B);

    writeRegister8(ILI9341_ENTRYMODE, 0x07);
    /* writeRegister32(ILI9341_DISPLAYFUNC, 0x0A822700);*/



    writeRegister8(ILI9341_SLEEPOUT, 0);
    delay(150);
    writeRegister8(ILI9341_DISPLAYON, 0);
    delay(500);
		setAddress(0,0,_width-1,_height-1);
     ///************* Start Initial Sequence ILI9341 controller **********///

	 // IOM = 1;
}
void drawPixel(uint16_t x3,uint16_t y3,uint16_t color1)
{

	// not using to speed up
	//if ((x3 < 0) ||(x3 >= TFTWIDTH) || (y3 < 0) || (y3 >= TFTHEIGHT))
	//{
	//	return;
	//}
	setAddress(x3,y3,x3+1,y3+1);

	//IOM = 0;

    CD=0; write8(0x2C);

	CD = 1;
	write8(color1>>8);write8(color1);
	//IOM = 1;
}

// draw a circle with this function



void fillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){
	if ((x >= TFTWIDTH) || (y >= TFTHEIGHT))
	{
		return;
	}

	if ((x+w-1) >= TFTWIDTH)
	{
		w = TFTWIDTH-x;
	}

	if ((y+h-1) >= TFTHEIGHT)
	{
		h = TFTHEIGHT-y;
	}

	setAddress(x, y, x+w-1, y+h-1);
    //IOM = 0;


	write8Reg(0x2C);
	//IOM = 1; IOM = 0;
	CD = 1;
	for(y=h; y>0; y--)
	{
		for(x=w; x>0; x--)
		{

			write8(color>>8); write8(color);

		}
	}
	//IOM = 1;
}

void fillScreen(uint16_t Color){
	//uint8_t VH,VL;
	long len = (long)TFTWIDTH * (long)TFTHEIGHT;

	 int blocks;

   uint8_t  i, hi = Color >> 8,
              lo = Color;

    blocks = (uint16_t)(len / 64); // 64 pixels/block
	setAddress(0,0,TFTWIDTH-1,TFTHEIGHT-1);

	//IOM = 0;


	write8Reg(0x2C);
	//IOM = 1; IOM = 0;
		CD = 1;
		write8(hi); write8(lo);

		len--;
		while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass
      do {

				write8(hi); write8(lo);write8(hi); write8(lo);
				write8(hi); write8(lo);write8(hi); write8(lo);
      } while(--i);
    }
    for(i = (char)len & 63; i--; ) {

      write8(hi); write8(lo);

    }

	//IOM = 1;

}
void drawChar(int16_t x, int16_t y, uint8_t c,uint16_t color, uint16_t bg, uint8_t size){
	if ((x >=TFTWIDTH) || // Clip right
	    (y >=TFTHEIGHT)           || // Clip bottom
	    ((x + 6 * size - 1) < 0) || // Clip left
	    ((y + 8 * size - 1) < 0))   // Clip top
	{
		return;
	}

	for (int8_t i=0; i<6; i++ )
	{
		uint8_t line;

		if (i == 5)
		{
			line = 0x0;
		}
		else
		{
			line = pgm_read_byte(font+(c*5)+i);
		}

		for (int8_t j = 0; j<8; j++)
		{
			if (line & 0x1)
			{
				if (size == 1) // default size
				{
					drawPixel(x+i, y+j, color);
				}
				else {  // big size
					fillRect(x+(i*size), y+(j*size), size, size, color);
				}
			} else if (bg != color)
			{
				if (size == 1) // default size
				{
					drawPixel(x+i, y+j, bg);
				}
				else
				{  // big size
					fillRect(x+i*size, y+j*size, size, size, bg);
				}
			}

			line >>= 1;
		}
	}

}

void write(uint8_t c)//write a character at setted coordinates after setting location and colour
{
	if (c == '\n')
	{
		cursor_y += textsize*8;
		cursor_x  = 0;
	}
	else if (c == '\r')
	{
		// skip em
	}
	else
	{
		drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
		cursor_x += textsize*6;
	}
}
void LCD_string_write(int8_t *str)
{
	int16_t i;
	for(i=0;str[i]!=0;i++)	/* Send each char of string till the NULL */
	{
		write(str[i]);	/* Call transmit data function */
	}
}

unsigned char ioread8(unsigned char __xdata* map_address){		//will this read work??
	__xdata unsigned char d;
	IOM = 1;
	d = *map_address;//read in source value
	d = *map_address;
	d = *map_address;
	d = *map_address;
	IOM = 0;
	return d;
}

unsigned char ramRead8(unsigned char __xdata* ram_address){		//will this read work??
	__xdata unsigned char d;
	IOM = 0;
	d = *ram_address;//read in source value
	d = *ram_address;
	d = *ram_address;
	d = *ram_address;
	IOM = 1;
	return d;
}

void LCD_clear(){
	fillScreen(BLACK);
	setTextColor(CYAN, BLACK);
	setRotation(1);
	setCursor(0,0);
}

// test RAM function
void testRAM(uint8_t d){
	uint32_t i;
	uint32_t j;
	__xdata uint8_t* ram_address;

	for (i = __START_RAM__; i < __END_RAM__; i++) {
		IOM = 0;
		ram_address = (uint8_t __xdata*)(i);//write
		*ram_address = d;
		IOM = 1;
	}
	for (j = __START_RAM__; j < __END_RAM__; j++) {
		IOM = 0;
		ram_address = (uint8_t __xdata*)(j); 
		uint8_t value = *(uint8_t*)ram_address;//read in value
		if (value != d){
			LCD_clear();
			setTextSize(3);
			__code unsigned char* fail = "FAIL\n";
			LCD_string_write(fail);
			break; //go to main??
		}
		IOM = 1;
	}
	//break; //go to main??
}

void freeType() {
	uint8_t count = 0;
	uint8_t d;
	while(1){


		if (count == 8) {
			d = '\n';
			count = 0;
			write(d);
		}
		else{
			d = keyDetect();
			write(d);
		}

		count++;
	}
}

uint8_t keyDetect(){
	__KEYPAD_PORT__=0xF0;			/*set port direction as input-output*/
	do
	{
		__KEYPAD_PORT__ = 0xF0;
		colloc = __KEYPAD_PORT__;
		colloc&= 0xF0;	/* mask port for column read only */
	}while(colloc != 0xF0);		/* read status of column */
	do
	{
		do
		{
			delay(20);	/* 20ms key debounce time */
			colloc = (__KEYPAD_PORT__ & 0xF0);	/* read status of column */
		}while(colloc == 0xF0);	/* check for any key press */

		delay(1);
		colloc = (__KEYPAD_PORT__ & 0xF0);
	}while(colloc == 0xF0);
	while(1)
	{
		/* now check for rows */
		__KEYPAD_PORT__= 0xFE;											/* check for pressed key in 1st row */
		colloc = (__KEYPAD_PORT__ & 0xF0);
		if(colloc != 0xF0)
		{
			rowloc = 0;
			break;
		}

		__KEYPAD_PORT__ = 0xFD;									/* check for pressed key in 2nd row */
		colloc = (__KEYPAD_PORT__ & 0xF0);
		if(colloc != 0xF0)
		{
			rowloc = 1;
			break;
		}

	__KEYPAD_PORT__ = 0xFB;			/* check for pressed key in 3rd row */
	colloc = (__KEYPAD_PORT__ & 0xF0);
	if(colloc != 0xF0)
	{
		rowloc = 2;
		break;
	}

	__KEYPAD_PORT__ = 0xF7;			/* check for pressed key in 4th row */
	colloc = (__KEYPAD_PORT__ & 0xF0);
	if(colloc != 0xF0)
	{
		rowloc = 3;
		break;
	}
}

	if(colloc == 0xE0)
	{
		return(keypad[rowloc][0]);
	}
	else if(colloc == 0xD0)
	{
		return(keypad[rowloc][1]);
	}
	else if(colloc == 0xB0)
	{
		return(keypad[rowloc][2]);
	}
	else
	{
		return(keypad[rowloc][3]);
	}
}

uint16_t reverse(uint8_t d) {
	uint16_t rev = 0;
	uint16_t val = 0;
	while(d >= 1){

		val = d%10;
		d = d/10;
		rev = rev * 10 + val;
	}
	return rev;
}

void asciiToDec(uint8_t d) {
	uint8_t val;
	uint16_t id;
	id = reverse(d);
	while (id >= 1){

		val = id % 10;
		id = id/10;
		write(val + '0');
	}
	//write('\n');
}

void asciiToHexFourDig(uint16_t d) {//HEX2ASCII
	uint8_t val;
	uint8_t store[4];
	uint8_t i =0;
	store[0] = 0;
	store[1] = 0;
	store[2] = 0;
	store[3] = 0;
	while (d >= 1){

		val = d % 16;
		d = d/16;
		if (val <= 9) {

			store[i] = val + '0';
		}
		else {
			store[i] = (val%10) + 'A';
		}
		i++;
	}
	i = 0;
	while (i < 4){
		if (store[i] == 0){
			store[i] = '0';
		}
		i++;
	}
	write(store[3]);
	write(store[2]);
	write(store[1]);
	write(store[0]);
	
	//write('\n');
}

void asciiToHex(uint8_t d) {//HEX2ASCII
	uint8_t val;
	uint8_t store[2];
	uint8_t i =0;
	store[0] = 0;
	store[1] = 0;
	while (d >= 1){

		val = d % 16;
		d = d/16;
		if (val <= 9) {

			store[i] = val + '0';
		}
		else {
			store[i] = (val%10) + 'A';
		}
		i++;
	}
	
	if (store[1] == 0 && store[0] == 0){
		write('0');
		write('0');
	}
	if (store[1] == 0 && store[0] != 0){
		write('0');
		write(store[0]);
	}
	if (store[0] == 0 && store[1] != 0){
		write(store[1]);
		write('0');
	}
	if (store[0] != 0 && store[1] != 0){
		write(store[1]);
		write(store[0]);
	}
}

uint8_t asciiToHex2(uint8_t d) {
	__xdata uint8_t val = 0;
	switch(d){
		case '0':
		val = 0x0;
		break;
		case '1':
		val = 0x1;
		break;
		case '2':
		val = 0x2;
		break;
		case '3':
		val = 0x3;
		break;
		case '4':
		val = 0x4;
		break;
		case '5':
		val = 0x5;
		break;
		case '6':
		val = 0x6;
		break;
		case '7':
		val = 0x7;
		break;
		case '8':
		val = 0x8;
		break;
		case '9':
		val = 0x9;
		break;
		case 'A':
		val = 0xA;
		break;
		case 'B':
		val = 0xB;
		break;
		case 'C':
		val = 0xC;//SAME AS 0XD
		break;
		case 'D':
		val = 0xD;
		break;
		case 'E':
		val = 0xE;
		break;
		case 'F':
		val = 0xF;
		break;
	}
	return val; //return hex number for memory applications
}

void menu(){
	LCD_clear();
	setTextSize(3);
	__code unsigned char* mainMenu;
	mainMenu= "MENU\n0 Photo      9 UART\n1 Temp\n2 ESP\n3 Dump\n4 RAM Check\n5 Move\n6 Find\n7 Edit\n8 Count";
	LCD_string_write(mainMenu);
	delay(200);
}

void photo(){
	LCD_clear();
	setTextSize(3);
	uint16_t value = 0;
	unsigned char test = 0;
	__code unsigned char* photoPage = "PHOTORESISTOR\n";
	LCD_string_write(photoPage);
	test = ioread8(phot_address);;
	asciiToHex(test);
	LCD_string_write("H");
	delay(300);
}

void temp(){
	unsigned char test = 0;
	LCD_clear();
	setTextSize(3);
	__code unsigned char* tempPage = "TEMPERATURE\n";
	LCD_string_write(tempPage);
	test = ioread8(temp_address);
	asciiToDec(test);//make in degrees
	LCD_string_write(" degrees");
	delay(300);
}
/*
void serial_isr() interrupt 4
{
	unsigned char receivedChar = 0;
	if(RI == 1){
		receivedChar = SBUF;
		SBUF = receiveChar;
		RI = 0;
	}
	//else if(TI == 1)
	//{
	//	TI = 0;
	//}
}
*/

void usart(){
	LCD_clear();
	setTextSize(3);
	__code unsigned char* usartPage = "USART\n";
	LCD_string_write(usartPage);
	delay(300);
	
	//baud rate
	__code unsigned char* choose = "Choose Baud Rate\n";
	LCD_string_write(choose);
	__code unsigned char* rate = "A 1200\nB 2400\nC 4800\nD 9600\nE 19200\n";
	LCD_string_write(rate);
	EA = 1;
	ES = 1;
	while (1){
		keyhit = keyDetect();
		switch(keyhit){
			case 'A':
				SMOD1 = 1;
				TH1 = 0xA9;
				break;
			case 'B':
				SMOD1 = 1;
				TH1 = 0xD5;
				break;
			case 'C':
				SMOD1 = 1;
				TH1 = 0xEA;
				break;
			case 'D':
				SMOD1 = 1;
				TH1 = 0xF5;
				break;
			case 'E':
				SMOD1 = 1;
				TH1 = 0xFB;
				break;
		}
	}
	//# of bits
	LCD_clear();
	setTextSize(3);
	__code unsigned char* usartPage = "USART\n";
	LCD_string_write(usartPage);
	__code unsigned char* bits = "Num of Bits\n";
	LCD_string_write(bits);
	__code unsigned char* rate = "A-8 or B-9\n";
	LCD_string_write(rate);
	while (1){
		keyhit = keyDetect();
		switch(keyhit){
			case 'A':
				SCON = 0x50;
				break;
			case 'B':
				SCON = 0x90;
				break;
		}
	}
	//parity
	/*
	__code unsigned char* parity = "Parity\n";
	LCD_string_write(parity);
	__code unsigned char* type = "A even\nB odd\nC none"; 
	LCD_string_write(type);
	while (1){
		keyhit = keyDetect();
		switch(keyhit){
			case 'A':
				//parity
				break;
			case 'B':
				//parity
				break;
			case 'C':
				//parity
				break;
		}
	}
	*/
}	
void esp(){
	//volatile unsigned char received_byte = 0;
	//volatile unsigned char received_flag = 0
	__xdata unsigned char byte = 0;
	LCD_clear();
	setTextSize(3);
	__code unsigned char* espPage = "ESP\n";
	LCD_string_write(espPage);
	__code unsigned char* choose = "Choose A,B, or C\n";
	LCD_string_write(choose);
	__code unsigned char* mood = "Mood is...\n";
	LCD_string_write(mood);
	//initialize
	SCON = 0x50;
	TMOD = 0x20;
	TH1 = 0xFB;
	TR1 = 1;
	ES = 1;
	EA = 1;
	
	while (byte != 'A' && byte != 'B' && byte != 'C' && byte != 'D'){
		RI = 1;
		byte = SBUF - 0x40;//or 70??
		RI = 0;
		switch(byte){
			case 'A':
				write(byte);
				LCD_string_write("\nYou are feeling\nlike...\nA material gurl");
				break;
			case 'B':
				write(byte);
				LCD_string_write("\nYou are feeling\nlike...\nA menace to\nsociety");
				break;
			case 'C':
				write(byte);
				LCD_string_write("\nYou are feeling\nlike...\nA delicate genius");
				break;
			case 'D':
				write(byte);
				LCD_string_write("\nYou are feeling\nlike...\nAn emo gurl");
				break;
		}
	}
	
	
	delay(300);
	//RX = 0; //output
	//reference Matthew's code
}

uint16_t readKeyboard2Digit(){
	__xdata uint8_t keypressed = 0;
	__xdata uint8_t count = 0;
	__xdata uint8_t num = 0x0;
	__xdata uint8_t num1 = 0x0;

	//STORE HEX VALUE FROM KEYBOARD
	while (count < 2){
		keypressed = keyDetect(); //type 4 characters, get ascii char
		write(keypressed); //write character
		if (count == 0){
			num = asciiToHex2(keypressed);
		}
		else{
			num = num * 0x10;
			num1 = asciiToHex2(keypressed);
			num = num + num1;
			num1 = 0;//reset floating value
		}
		count++;
	}
	delay(500);
	return num;
}

uint16_t readKeyboard(){
	__idata uint8_t keypressed = 0;
	__idata uint8_t count = 0;
	__idata uint16_t num = 0x0;
	__idata uint16_t num1 = 0x0;

	//STORE HEX VALUE FROM KEYBOARD
	while (count < 4){
		keypressed = keyDetect(); //type 4 characters, get ascii char
		write(keypressed); //write character
		if (count == 0){
			num = asciiToHex2(keypressed);
		}
		else{
			num = num * 0x10;
			num1 = asciiToHex2(keypressed);
			num = num + num1;
			num1 = 0;//reset floating value
		}
		count++;
	}
	return num;
}

uint16_t type(){ //may need to change uint8_t or have a cap,CANNOT BE ZERO
	__xdata uint8_t c = 0;
	__xdata uint16_t d = 0;
	LCD_string_write("\nData Type:\nA Byte\nB Word\nC Db Word...");
	while (c != 'A' && c != 'B' && c != 'C'){
		c = keyDetect();
		switch(c){
			case 'A':
				write(c);
				LCD_string_write("\nSize:");
				d = readKeyboard();
				//asciiToHexFourDig(d);//print value
				break;
			case 'B':
				write(c);
				LCD_string_write("\nSize:");//create range for size
				d = readKeyboard();
				d = d * 0x2;
				break;
			case 'C':
				write(c);
				LCD_string_write("\nSize:");
				d = readKeyboard();
				d = d * 0x4;
				break;
		}
	}
	return d;
}
/*
void dump(){
	//VARIABLES
	__xdata uint16_t source = 0x0;
	__xdata uint16_t ogSource = 0x0;
	__xdata uint16_t typeData = 0x0;
	__xdata uint8_t keypressed = 0;
	__xdata uint8_t value = 0x0;
	__xdata uint8_t perPage = 0;
	__xdata uint16_t* ram_address;
	__xdata uint8_t c = 0;
	__xdata uint8_t count = 0x0;
	//SETUP
	LCD_clear();
	setTextSize(3);
	LCD_string_write("DUMP\n");
	LCD_string_write("Src Address:");
	ogSource = readKeyboard();//read in value to kb write("\n" + source);???
	source = ogSource; //start address
	typeData = type();
	perPage = 7;
	//FUNCTION
	LCD_clear();
	setTextSize(3);
	while ((typeData > 0) && (source >= ogSource)){//Do i need FFFF and 0000 bounds??
		LCD_clear();
		setTextSize(3);
		perPage = 7; //reset
		while ((perPage > 0) && typeData > 0){ //(source >= ogSource) //go a few times
			ram_address = (uint16_t __xdata*)(source); //hex address
			//value = ramRead8(ram_address);
			value = *(uint16_t*)ram_address;
			value = *(uint16_t*)ram_address;
			value = *(uint16_t*)ram_address;
			value = *(uint16_t*)ram_address;
			asciiToHexFourDig(source);
			LCD_string_write(" ");
			asciiToHex(value);
			LCD_string_write("H\n");
			source = source + 0x1; //increase address
			typeData = typeData - 0x1;
			perPage--;
			count = count + 0x1;
			}
		__code unsigned char* paging = ">1\n";
		LCD_string_write(paging);
		delay(300);
		c = 0;
		while (c != '1'){
			c = keyDetect();
			switch(c){
				case '1':
					LCD_string_write("Next");
					//let system continue to increment
					break;
			}
		}
	}
}
*/
void ramCheck(){
	__xdata uint8_t c = 0;
	__xdata uint8_t d = 0x0;
	LCD_clear();
	setTextSize(3);
	LCD_string_write("RAM CHECK\n");
	LCD_string_write("RAM test value:");
	d = readKeyboard2Digit();
	testRAM(d);
	d = d^0xFF; //invert number and go again
	testRAM(d);
	LCD_clear();
	setTextSize(3);
	LCD_string_write("RAM CHECK PASSED");
	delay(300);
}

void move(){
	//VARIABLES
	__idata uint16_t source = 0;
	__idata uint16_t destination = 0;
	__xdata uint16_t typeData = 0;
	__xdata uint8_t value = 0;
	__xdata uint8_t* ram_address;
	//SETUP
	LCD_clear();
	setTextSize(3);
	LCD_string_write("MOVE\n");
	LCD_string_write("Src Address:");
	source = readKeyboard();
	LCD_string_write("\nDst Address:");
	destination = readKeyboard();
	//FUNCTION
	typeData = type();
	//LCD_clear();
	//setTextSize(3);
	//LCD_string_write("Move in Progress...\n");
	while (typeData > 0){ //address bounds?
		LCD_clear();
		setTextSize(3);
		LCD_string_write("Move in Progress...\n");
		ram_address = (uint8_t __xdata*)(source); 
		value = *ram_address;//read in source value
		asciiToHex(value);//source value
		
		ram_address = (uint8_t __xdata*)(destination); 
		*ram_address = value; //write to destination
		
		ram_address = (uint8_t __xdata*)(source); 
		value = *ram_address;//read in source value
		LCD_string_write("\nSource:");
		asciiToHexFourDig(source);
		LCD_string_write("\n");
		asciiToHex(value);//print value in source
		
		ram_address = (uint8_t __xdata*)(destination); 
		value = *ram_address;//read in destination value
		LCD_string_write("\nDestination:");
		asciiToHexFourDig(destination);
		LCD_string_write("\n");
		asciiToHex(value);//print value in destination
		typeData--;
		destination++;
		source++;
		
	} 
	delay(300);
	LCD_clear();
	setTextSize(3);
	LCD_string_write("MOVE DONE!");
	delay(300);
}

void find(){
	__xdata uint16_t source = 0;
	__xdata uint16_t bytes = 0;
	__xdata uint8_t c = 0;
	__xdata uint8_t d = 0x0;
	__xdata uint8_t count = 0;
	__xdata uint8_t value = 0;
	__xdata uint8_t* ram_address;
	//SETUP
	LCD_clear();
	setTextSize(3);
	LCD_string_write("FIND\n");
	LCD_string_write("Src Address:");
	source = readKeyboard();
	LCD_string_write("\n# of Bytes:");
	bytes = readKeyboard();
	LCD_string_write("\nFind value:");
	d = readKeyboard2Digit();
	LCD_clear();
	setTextSize(3);
	//use while loop to find value within size 
	while (bytes > 0){ 
		ram_address = (uint8_t __xdata*)(source);
		value = *(uint8_t*)ram_address;//read in
		value = *(uint8_t*)ram_address;
		value = *(uint8_t*)ram_address;
		value = *(uint8_t*)ram_address;	
		bytes--;		
		if (value == d){
			LCD_string_write("Found at:");
			asciiToHexFourDig(source);//print once and go back to hex
			delay(300);
			LCD_clear();
			setTextSize(3);//STUCK
		}
		source++;
	}
}


void edit(){
	__xdata uint16_t source = 0;
	__xdata uint8_t c = 0;
	__xdata uint8_t d = 0x0;
	__xdata uint8_t count = 0;
	__xdata uint8_t read = 0;
	__xdata uint8_t value = 0;
	__xdata uint8_t* ram_address;
	__xdata uint8_t keypressed = 0;
	//SETUP 
	LCD_clear();
	setTextSize(3);
	LCD_string_write("EDIT\n");
	LCD_string_write("Src Address:");
	source = readKeyboard();
	LCD_string_write("\n");
	//FUNCTION
	while (keypressed != '1'){
		LCD_string_write("New input:");
		d = readKeyboard2Digit();
		ram_address = (uint8_t __xdata*)(source); 
		*ram_address = d; //write to source

		LCD_string_write("\nVALUE SAVED");
		//LCD_string_write("\nPURRR QWEEN");
		//PAGER
		LCD_string_write("\n0-continue\n1-exit");
		keypressed = keyDetect();
		if (keypressed == '0'){
			source++;
		}
		if (keypressed == '1'){
			break;
		}	
		LCD_clear();
		setTextSize(3);
		count = 0;
	}
}

void count(){
	__xdata uint16_t source = 0;
	__xdata uint16_t bytes = 0;
	__xdata uint8_t d = 0x0;
	__xdata uint8_t count = 0;
	__xdata uint8_t value = 0;
	__xdata uint8_t* ram_address;
	//SETUP
	LCD_clear();
	setTextSize(3);
	LCD_string_write("COUNT\n");
	LCD_string_write("Src Address:");
	source = readKeyboard();
	LCD_string_write("\n# of Bytes:");
	bytes = readKeyboard();
	LCD_string_write("\nFind value:");
	d = readKeyboard2Digit();
	while (bytes > 0){ 
		ram_address = (uint8_t __xdata*)(source);
		value = *(uint8_t*)ram_address;//read in 
		value = *(uint8_t*)ram_address;//read in 
		value = *(uint8_t*)ram_address;//read in 
		value = *(uint8_t*)ram_address;//read in 
		bytes--;
		if (value == d){
			//__code unsigned char* foundAt = "\nFound at:"; //NEED ASCII
			//LCD_string_write(foundAt);
			//asciiToHex(source);//print once and go back to hex
			count++;
			//delay(300);
			//break;
		}
		source++;
	}
	LCD_string_write("\n# of Finds:");
	asciiToDec(count);//display final value, add 0 and test this function 
	delay(300);
}


void dump2(){
	__xdata uint16_t source = 0;
	__xdata uint8_t value = 0;
	__xdata uint16_t bytes = 0;
	__xdata uint8_t counter = 7;
	__xdata unsigned char button = 0;
	__xdata uint8_t* ram_address;
	LCD_clear();
	setTextSize(3);
	LCD_string_write("DUMP\n");
	LCD_string_write("Src Address:");
	source = readKeyboard();
	//pointer = source;
	bytes = type();
	LCD_clear();
	setTextSize(3);
	while (bytes > 0){
		asciiToHexFourDig(source);
		//IOM = 0;
		ram_address = (uint8_t __xdata*)(source);
		value = *ram_address;
		//IOM = 1;
		LCD_string_write(" ");
		asciiToHex(value);
		LCD_string_write("\n");
		source++;
		counter--;
		bytes --;
		if(counter == 0){
			LCD_string_write (">1");
			button = keyDetect();
			if (button == '1'){
				LCD_string_write("\nNext");
				counter = 7;
				LCD_clear();
				setTextSize(3);
			}
		}
	}
	delay(300);
}
/*
void test(){
	__xdata uint8_t value = 0;
	__xdata uint8_t* ram_address;
	__xdata uint8_t source = 0;
	__xdata uint8_t i = 7;
	LCD_clear();
	setTextSize(3);
	//source = readKeyboard();
	while (i > 0){
		IOM = 0;
		ram_address = (uint8_t __xdata*)(source); //read in your address
		value = *ram_address;
		IOM = 1;
		asciiToHex(value);
		LCD_string_write("\n");
		source++;
		i--;
	}
	
	LCD_string_write("\n");
	IOM = 0;
	ram_address = (uint8_t __xdata*)(0x0000); //read in your address
	value = *ram_address;
	IOM = 1;
	asciiToHex(value);
	
	delay(300);
}
*/
void main (void) {
	CD = 0;
	IOM = 0;

	iowrite8(seg7_address, 0x3f);
	IOM = 0;
	CD = 1;

	TFT_LCD_INIT();
	iowrite8(seg7_address, 0x06);
	LCD_clear();
	setTextSize(5);
	LCD_string_write("JAVIER 2.0\n");
	setTextSize(3);
	LCD_string_write("Courtney Young\n");
	LCD_string_write("ECEN-4330\n");
	delay(300);
	__xdata uint8_t keyhit;
	while(1){
		menu();
		keyhit = keyDetect();
		switch(keyhit){
			case '0':
				photo();
				break;
			case '1':
				temp();
				break;
			case '2':
				esp();
				break;
			case '3':
				dump2();
				break;
			case '4':
				ramCheck();
				break;
			case '5':
				move();
				break;
			case '6':
				find();
				break;
			case '7':
				edit();
				break;
			case '8':
				count();
				break;
			case '9':
				usart();
				break;	
		} 
	}
}

# include <8051.h>
# include <stdint.h>
# include "registers.h"
# include "bmp_image.h"


# define __LCD_ADDRESS__ 0x4000
# define __SEG_7_ADDRESS__ 0x8000
# define __PHOT_ADDRESS__ 0x0000
# define __TEMP_ADDRESS__ 0xC000


# define __START_RAM__ 0x0000
# define __END_RAM__ 0xFFFF
/// LCD specific variables
// width and height of lcd in pixels
# define TFTWIDTH 240
# define TFTHEIGHT 240



// if needed to remeber command/data lines and active/idle signals

# define __ACTIVE__ 0
# define __IDLE__ 1
# define __CMD__ 0
# define __DATA__ 1

# define __KEYPAD_PORT__ P1


// defining important pins for LCD interfacing
// This is how it is defined for
# define IOM P3_4
# define CD P3_5
# define RX P3_0


// definition of colors in 2-bytes
#define BLACK 0x0000
#define GRAY 0xD6BA
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF


//
/// function declaration


void TFT_LCD_INIT(void);  // init function
void delay(int16_t d);  // delay function for d ms


void TFT_LCD_BEGIN(void);  // begin LCD
void writeRegister8(uint8_t a, uint8_t d);
void writeRegister16(uint16_t a, uint16_t d);
void fillScreen(uint16_t color);  // fill screen with the color defined
// set address to bound your operational area
void setAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y);
// reset your LCD
void reset(void);



// draw PIXEL at one pixel
void drawPixel(uint16_t x3,uint16_t y3,uint16_t colour1);
// fill your LCD in operating region
void lcdfill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);


void fillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color);

// draw a character
void drawchar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t size);
//void write(uint8_t c);
// set cursor in certain pixel
void setCursor(uint16_t x, uint16_t y);
// set textcolor
void setTextColor(uint16_t x, uint16_t y);
// set textsize
void setTextSize(uint8_t s);

// set string write
void LCD_string_write(int8_t *str);
// dont really need this function
void drawGrayscaleBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h, uint16_t color) ;

// draw circles
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
// test circles
void testCircles(uint8_t radius, uint16_t color);


uint8_t keyDetect();
void testRAM(uint8_t d);
void freeType();
uint16_t reverse(uint8_t d);
void asciiToDec(uint8_t d);
void asciiToHex(uint8_t d);
// you have to have latch and proper decoding technique
//   *lcd_address = d;	 use this in inline function along with WR signal
// to interface with address decder					
// this is inline funciton definition with just a port


// *** inline functions make your program runs faster
// see that I am writing directly to the port.
// you can use *lcd_address to write the data into databus
// however, you need to change decoding.
// my write probing will not work

#include <ST7735.h>
#include <string.h>
#include "main.h"
#include "gpio.h"
#include "stdint.h"
#include "stdlib.h"

uint8_t _width;       ///< Display width as modified by current rotation
uint8_t _height;      ///< Display height as modified by current rotation
uint8_t cursor_x;     ///< x location to start print()ing text
uint8_t cursor_y;     ///< y location to start print()ing text
uint8_t rotation;     ///< Display rotation (0 thru 3)
uint8_t _colstart;   ///< Some displays need this changed to offset
uint8_t _rowstart;       ///< Some displays need this changed to offset
uint8_t _xstart;
uint8_t _ystart;

  const uint8_t
  init_cmds1[] = {            // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color

#if (defined(ST7735_IS_128X128) || defined(ST7735_IS_160X128))
  init_cmds2[] = {            // Init for 7735R, part 2 (1.44" display)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F },           //     XEND = 127
#endif // ST7735_IS_128X128

#ifdef ST7735_IS_160X80
  init_cmds2[] = {            // Init for 7735S, part 2 (160x80 display)
    3,                        //  3 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x4F,             //     XEND = 79
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F ,            //     XEND = 159
    ST7735_INVON, 0 },        //  3: Invert colors
#endif

  init_cmds3[] = {            // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay

void ST7735_Reset() {
	LCD_RST_OFF;
	HAL_Delay(5);
	LCD_RST_ON;
}

void ST7735_WriteCommand(uint8_t cmd) {
	LCD_CMD_OFF;
// Whay DMA transfer is not accepted here? 	
//	#ifdef ST7735_USE_DMA
//		HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, &cmd, CMD_SIZE);
//		while(hspi2.State == HAL_SPI_STATE_BUSY_TX);
//	#else
	HAL_SPI_Transmit(&ST7735_SPI_PORT, &cmd, CMD_SIZE, HAL_MAX_DELAY);
//		#endif
}

void ST7735_WriteData(uint8_t* buff, size_t buff_size) {
	LCD_CMD_ON;
	#ifdef ST7735_USE_DMA
		HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, buff, buff_size);
		while(hspi2.State == HAL_SPI_STATE_BUSY_TX);
	#else
		HAL_SPI_Transmit(&ST7735_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
	#endif
}

void DisplayInit(const uint8_t *addr) {
	uint8_t numCommands, numArgs;
	uint16_t ms;
	numCommands = *addr++;
	while(numCommands--) {
		uint8_t cmd = *addr++;
		ST7735_WriteCommand(cmd);
		numArgs = *addr++;
		// If high bit set, delay follows args
		ms = numArgs & DELAY;
		numArgs &= ~DELAY;
		if(numArgs) {
			ST7735_WriteData((uint8_t*)addr, numArgs);
			addr += numArgs;
		}
		if(ms) {
			ms = *addr++;
			if(ms == 255) ms = 500;
			HAL_Delay(ms);
		}
	}
}

void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	// column address set
	ST7735_WriteCommand(ST7735_CASET);
	uint8_t data[COORD_SIZE] = {0x00, x0 + _xstart, 0x00, x1 + _xstart};
	ST7735_WriteData(data, COORD_SIZE);
	// row address set
	ST7735_WriteCommand(ST7735_RASET);
	data[1] = y0 + _ystart;
	data[3] = y1 + _ystart;
	ST7735_WriteData(data, COORD_SIZE);
	// write to RAM
	ST7735_WriteCommand(ST7735_RAMWR);
}

void ST7735_Init(uint8_t rotation) {
	ST7735_Select;
	ST7735_Reset();
	DisplayInit(init_cmds1);
	DisplayInit(init_cmds2);
	DisplayInit(init_cmds3);
	#if ST7735_IS_160X80
		_colstart = 24;
		_rowstart = 0;
		/*****  IF Doesn't work, remove the code below (before #elif) *****/
		uint8_t data = 0xC0;
		ST7735_Select;
		ST7735_WriteCommand(ST7735_MADCTL);
		ST7735_WriteData(&data, 1);
		ST7735_Unselect;
	#elif ST7735_IS_128X128
		_colstart = 2;
		_rowstart = 3;
	#else
		_colstart = 0;
		_rowstart = 0;
	#endif
	ST7735_SetRotation (rotation);
	ST7735_Unselect;
}

void ST7735_SetRotation(uint8_t m) {
	uint8_t madctl = 0;
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
  case 0:
		#if ST7735_IS_160X80
			madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR;
		#else
			madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB;
			_height = ST7735_HEIGHT;
			_width = ST7735_WIDTH;
			_xstart = _colstart;
			_ystart = _rowstart;
		#endif
    break;
  case 1:
		#if ST7735_IS_160X80
			madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR;
		#else
			madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
			_width = ST7735_HEIGHT;
			_height = ST7735_WIDTH;
			_ystart = _colstart;
			_xstart = _rowstart;
		#endif
    break;
  case 2:
		#if ST7735_IS_160X80
			madctl = ST7735_MADCTL_BGR;
		#else
			madctl = ST7735_MADCTL_RGB;
			_height = ST7735_HEIGHT;
			_width = ST7735_WIDTH;
			_xstart = _colstart;
			_ystart = _rowstart;
		#endif
    break;
  case 3:
		#if ST7735_IS_160X80
			madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR;
		#else
			madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
			_width = ST7735_HEIGHT;
			_height = ST7735_WIDTH;
			_ystart = _colstart;
			_xstart = _rowstart;
		#endif
    break;
  }
  ST7735_Select;
  ST7735_WriteCommand(ST7735_MADCTL);
  ST7735_WriteData(&madctl, 1);
  ST7735_Unselect;
}

void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
	if((x >= _width) || (y >= _height))
		return;
	uint8_t arrColor[2];
	INT_TO_BYTES(color, arrColor);
	ST7735_Select;
	ST7735_SetAddressWindow(x, y, x+1, y+1);
	ST7735_WriteData(arrColor, COLOR_SIZE);
	ST7735_Unselect;
}

void ST7735_WriteChar(uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
	uint8_t i, j;
	uint32_t b;
	uint8_t arrColor[COLOR_SIZE];
	INT_TO_BYTES(color, arrColor);
	uint8_t bgColor[COLOR_SIZE];
	INT_TO_BYTES(bgcolor, bgColor);	
	ST7735_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);
	for(i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for(j = 0; j < font.width; j++) {
			if((b << j) & 0x8000) {
				ST7735_WriteData(arrColor, COLOR_SIZE);
			} else {
				ST7735_WriteData(bgColor, COLOR_SIZE);
			}
		}
	}
}

void ST7735_WriteString(uint8_t x, uint8_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
	ST7735_Select;
	while(*str) {
		if(x + font.width >= _width) {
			x = 0;
			y += font.height;
			if(y + font.height >= _height) {
				break;
			}
			if(*str == ' ') {
				// skip spaces in the beginning of the new line
				str++;
				continue;
			}
		}
		ST7735_WriteChar(x, y, *str, font, color, bgcolor);
		x += font.width;
		str++;
	}
	ST7735_Unselect;
}

void ST7735_FillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width) w = _width - x;
	if((y + h - 1) >= _height) h = _height - y;
	ST7735_Select;
	ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	uint8_t arrColor[COLOR_SIZE];
	INT_TO_BYTES(color, arrColor);
	// Minimize SPI transfers (large array transfer is not working on the display)
	if(h > w){
		_swap_int8_t(h, w);
	}
	const uint16_t buffSize = COLOR_SIZE * w;
	uint8_t colorBuffer[buffSize];
	// Fill colorBuffer with color data
	for (uint16_t i = 0; i < buffSize; i += COLOR_SIZE) {
		memcpy(colorBuffer + i, arrColor, COLOR_SIZE);
	}
	LCD_CMD_ON;
	while(h--){
		#ifdef ST7735_USE_DMA
			HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, colorBuffer, buffSize);
			while (HAL_SPI_GetState(&ST7735_SPI_PORT) != HAL_SPI_STATE_READY);
		#else
			HAL_SPI_Transmit(&ST7735_SPI_PORT, colorBuffer, buffSize, HAL_MAX_DELAY);
		#endif
	}
	ST7735_Unselect;
}

void ST7735_InvertColors(bool invert) {
	ST7735_Select;
	ST7735_WriteCommand(invert ? ST7735_INVON : ST7735_INVOFF);
	ST7735_Unselect;
}

void fillScreen(uint16_t color) {
    ST7735_FillRectangle(0, 0, _width, _height, color);
}

void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
	int8_t steep = abs(y1 - y0) > abs(x1 - x0);
	// Swap coordinates if the line is steep
	if (steep) {
		_swap_int8_t(x0, y0);
		_swap_int8_t(x1, y1);
	}
	// Ensure x0 is always less than x1
	if (x0 > x1) {
		_swap_int8_t(x0, x1);
		_swap_int8_t(y0, y1);
	}
	// Use int8_t for dx and dy since LCD dimensions are small
	int8_t dx = x1 - x0;
	int8_t dy = abs(y1 - y0);
	int8_t err = dx >> 1; // Bitwise right shift instead of division by 2
	// Determine the direction of y (1 or -1)
	int8_t ystep = (y0 < y1) ? 1 : -1;
	// Iterate over x coordinates
	for (; x0<=x1; x0++) {
		// Draw the pixel, considering the steepness
		if (steep) {
			ST7735_DrawPixel(y0, x0, color);
		} else {
			ST7735_DrawPixel(x0, y0, color);
		}
		// Update error term
		err -= dy;
		// Move to the next y coordinate if the error term is negative
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void  drawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint16_t color) {
	drawLine(x, y, x, y + h - 1, color);
}
void  drawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint16_t color) {
	drawLine(x, y, x + w - 1, y, color);
}

void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
	drawFastHLine(x, y, w, color);
	drawFastHLine(x, y+h-1, w, color);
	drawFastVLine(x, y, h, color);
	drawFastVLine(x+w-1, y, h, color);
}

void drawButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t fillColor, uint16_t txtColor, const char* txt){
	drawRect(x, y, w, h, txtColor);
	ST7735_FillRectangle(x+1, y+1, w-2, h-2, fillColor);
	ST7735_WriteString(x+7, y+6, txt, Font_7x10, txtColor, fillColor);
}
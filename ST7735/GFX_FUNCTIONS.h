#include <stdint.h>
#ifndef INC_GFX_FUNCTIONS_H_
#define INC_GFX_FUNCTIONS_H_


void drawPixel(uint8_t x, uint8_t y, uint16_t color);
void writeLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void drawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint16_t color);
void drawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint16_t color);
void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void drawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);
void drawCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t cornername, uint16_t color);
void fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t corners, int8_t delta, uint16_t color);
void fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void drawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint16_t color);
void fillRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint16_t color);
void drawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void fillTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void fillScreen(uint16_t color);
void drawButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t borderColor, uint16_t fillColor, uint16_t selectColor, const char* txt);

#endif /* INC_GFX_FUNCTIONS_H_ */

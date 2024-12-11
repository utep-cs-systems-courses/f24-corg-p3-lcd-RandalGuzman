/** \file lcddraw.c
 *  \brief Adapted from RobG's EduKit
 */
#include "lcdutils.h"
#include "lcddraw.h"

/** Draw single pixel at x,row 
 *
 *  \param col Column to draw to
 *  \param row Row to draw to
 *  \param colorBGR Color of pixel in BGR
 */
void drawPixel(u_char col, u_char row, u_int colorBGR) 
{
  lcd_setArea(col, row, col, row);
  lcd_writeColor(colorBGR);
}

/** Fill rectangle
 *
 *  \param colMin Column start
 *  \param rowMin Row start
 *  \param width Width of rectangle
 *  \param height height of rectangle
 *  \param colorBGR Color of rectangle in BGR
 */
void fillRectangle(u_char colMin, u_char rowMin, u_char width, u_char height, 
		   u_int colorBGR)
{
  u_char colLimit = colMin + width, rowLimit = rowMin + height;
  lcd_setArea(colMin, rowMin, colLimit - 1, rowLimit - 1);
  u_int total = width * height;
  u_int c = 0;
  while ((c++) < total) {
    lcd_writeColor(colorBGR);
  }
}

void fillCircle(u_char colMin, u_char rowMin, u_char radius, u_int colorBGR) {
    int x, y;
    for (x = -radius; x <= radius; x++) {
        for (y = -radius; y <= radius; y++) {
            if (x * x + y * y <= radius * radius) {  // Check if the point (x, y) is within the circle
                drawPixel(colMin + x, rowMin + y, colorBGR);  // Draw pixel if inside the circle
            }
        }
    }
}

/** Clear screen (fill with color)
 *  
 *  \param colorBGR The color to fill screen
 */
void clearScreen(u_int colorBGR) 
{
  u_char w = screenWidth;
  u_char h = screenHeight;
  fillRectangle(0, 0, screenWidth, screenHeight, colorBGR);
}

/** 11x16 font - this function draws background pixels for 11x16 font
 *  Adapted from RobG's EduKit
 */

void drawChar11x16(u_char rcol, u_char rrow, char c, u_int fgColorBGR, u_int bgColorBGR) 
{
  u_char col = 0;  // Column iterator
  u_char row = 0;  // Row iterator
  u_char bit = 0x01;  // Mask for bit extraction
  u_char oc = c - 0x20;  // Character offset (ASCII 0x20 is space)

  // Set the area to draw an 11x16 character
  lcd_setArea(rcol, rrow, rcol + 10, rrow + 15);  // 11x16 character area

  // Loop over each row of the character
  while (row < 16) {
      while (col < 11) {
          u_int colorBGR = (font_11x16[oc][col] & bit) ? fgColorBGR : bgColorBGR;
          lcd_writeColor(colorBGR);
          col++;
      }
      col = 0;
      bit <<= 1;  // Shift bit for the next row of the character
      row++;
  }
}

/** Draw string at col,row using 11x16 font
 *
 *  \param col Column to start drawing string
 *  \param row Row to start drawing string
 *  \param string The string
 *  \param fgColorBGR Foreground color in BGR
 *  \param bgColorBGR Background color in BGR
 */
void drawString11x16(u_char col, u_char row, char *string,
		u_int fgColorBGR, u_int bgColorBGR)
{
  u_char cols = col;
  while (*string) {
    // Draw each character with a space of 12 pixels (11 for the character, 1 for spacing)
    drawChar11x16(cols, row, *string++, fgColorBGR, bgColorBGR);
    cols += 12;  // 11 pixels for the character + 1 pixel of spacing
  }
}


/** Draw rectangle outline
 *  
 *  \param colMin Column start
 *  \param rowMin Row start 
 *  \param width Width of rectangle
 *  \param height Height of rectangle
 *  \param colorBGR Color of rectangle in BGR
 */
void drawRectOutline(u_char colMin, u_char rowMin, u_char width, u_char height,
		     u_int colorBGR)
{
  /**< top & bot */
  fillRectangle(colMin, rowMin, width, 1, colorBGR);
  fillRectangle(colMin, rowMin + height, width, 1, colorBGR);

  /**< left & right */
  fillRectangle(colMin, rowMin, 1, height, colorBGR);
  fillRectangle(colMin + width, rowMin, 1, height, colorBGR);
}

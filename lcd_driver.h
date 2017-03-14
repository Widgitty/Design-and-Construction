/*includes*/
#include "stm32f4xx.h"
#include "lcd_buffer.h"

/* Defines for user to use in their code*/
#define LCD_WRITE_DATA 0x0000
#define LCD_WRITE_COMMAND 0x8000
#define LCD_LINES_ONE 0
#define LCD_LINES_TWO 1
#define LCD_SCROLLING_ON 1
#define LCD_SCROLLING_OFF 0
#define LCD_CURSOR_ON 1
#define LCD_CURSOR_OFF 0
#define LCD_CBLINK_ON 1
#define LCD_CBLINK_OFF 0
#define LCD_POWER_ON 1
#define LCD_POWER_OFF 0

/* other defines */
#define _LCD_RS_PIN 0x0001
#define _LCD_E_PIN 0x0004
#define _LCD_RW_PIN 0x0002
#define _LCD_SCROLLING_ON_COMMAND 0x4000
#define _LCD_SCROLLING_OFF_COMMAND 0x2000
#define _LCD_SHORT_AMOUNT_OF_TIME 2
#define _LCD_STAGE_1_TIME 11
#define _LCD_STAGE_2_TIME 33
#define _LCD_STAGE_3_TIME 7000
#define _LCD_SCROLLING_COMMAND_DATA_WORD 0x18

/*Function prototypes*/

/* Change the settings. Can turn the LCD on or off, turn the cursor on 
 * off or turn the blinking on or off. 
 *
 * @param power: can be LCD_POWER_ON or LCD_POWER_OFF
 * @param cursor: can be LCD_CURSOR_ON or LCD_CURSOR_OFF
 * @param blink: Turns the cursor blink on or off, can be
 * 		LCD_CBLINK_ON or LCD_CBLINK_OFF
 */
void lcd_change_settings(int power, int cursor, int blink);

/* Clears the LCD */
void lcd_clear_display(void);

/* Initialises the display 
 *
 * @param lines: number of lines to use, can be
 * 		LCD_LINES_ONE or LCD_LINES_TWO
 * @param cursor: can be LCD_CURSOR_ON or LCD_CURSOR_OFF
 * @param blink: Turns the cursor blink on or off, can be
 * 		LCD_CBLINK_ON or LCD_CBLINK_OFF
 * @param buffer_size: Size of write buffer. Recommended size ~128
 */
void lcd_init(int lines, int cursor, int blink, int buffer_size);

/* Moves the cursor to the line and character on the line specified 
 * by the parameters
 */
void lcd_move_cursor(uint16_t line, uint16_t offset);

/* Sets scrolling on or off
 *
 * @param on: can be LCD_SCROLLING_ON or LCD_SCROLLING_OFF
 * @param ms: number of milliseconds between jumps. Lower 
 * 		value will give faster scrolling. Try 300ms
 */
void lcd_scrolling(int on, int ms);

/* Writes a string to the LCD 
 *
 * @param string: string to write to LCD
 * @param line: line to write to (0 or 1)
 * @param offset: character number to write to (starts at 0)
 */

void lcd_write_string(char *string, int line, int offset);

/* Internal functions you don't need to use */
void lcd_setup_GPIO(void);
void lcd_setup_timer(void);
void lcd_write(int cmd, uint16_t byte, uint16_t time_delay);

/*Create buffer*/
extern BUFFER lcd_commandBuffer;

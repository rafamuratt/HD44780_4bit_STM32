
#ifndef LCD_16X2_4BITS_MODE_H    
#define LCD_16X2_4BITS_MODE_H

#define DELAY_MS(ms) delay(ms)
#define DELAY_US(us) delayMicroseconds(us)

#ifdef __cplusplus
extern "C" {
#endif

            typedef enum {
                VAL_UNI,   
                VAL_DEZ,   
                VAL_CEN,   
                VAL_MIL,   
                VAL_DEZM   
            } dispFormat;

            /* -----------------------------------------------------------------------------*/
            /* Functions Prototypes */
            void lcd_begin();
            void lcd_blink(char row, char col);
            void lcd_noBlink();
            void lcd_clear();
            void lcd_send(unsigned char cmd);                                               // send commands (one byte as parameter)
            void lcd_nib(unsigned char nib, char rsel);                                     // send a nibble + status of register select (character/ command)
            void lcd_char(char row, char col, unsigned char chr);                           // write one character at defined position
            void lcd_str(char row, char col, char *str);                                    // write text/ strings
            void lcd_chrCp(unsigned char chr);                                              // write a character at current cursor position
            void lcd_num(char row, char col, unsigned int num, dispFormat range);           // convert up to 5 digit integer and excludes left zeroes. To count more than 65535, use unsigned long (requires a lot of RAM)
            void lcd_numZero(char row, char col, unsigned int num, dispFormat range);       // convert up to 5 digit integer with left zeroes. To count more than 65535, use unsigned long (requires a lot of RAM)

            /* -----------------------------------------------------------------------------*/

            #ifdef __cplusplus
            }

            #endif

#endif

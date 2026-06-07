/* 
LCD 16x2 HD44780 Driver Library for STM32F103C8T6 (BluePill)
4-bit interface, PortB, bare metal HAL
By B.Eng. Rafa Muratt 06.2026
MURAT-TECH CHANNEL: https://www.youtube.com/@Murat-TechChannel-EN
MURAT-TECH HUB: https://murat-tech.eu/


License
This project is licensed under the GNU General Public License v3.0 (GPL-3.0).  
See the full license text at: https://www.gnu.org/licenses/gpl-3.0.html.

If this project is helpful for your application, please consider supporting: https://www.paypal.com/donate/?hosted_button_id=8S8BJ9TT368VN                                           
*/

#include "MT_lcd_16x2_pins_STM32.h"                                              // Pins mapping to the library
#include "MT_lcd_16x2_4bits_STM32.h"                                             // Functions mapping to the library

#define		set_bit(register,bit)	(register |= (1<<bit))                       // shift left "1" the number set in "bit", setting a bit in the given register pos (not affecting the other bits)
#define		clr_bit(register,bit)	(register &= ~(1<<bit))                      // shift left "1" the number set in "bit" and invert to "0", clearing a bit in the given register pos (not affecting the other bits)

/* -----------------------------------------------------------------------------*/
/* Init the LCD (datasheet page 46)*/
void lcd_begin(){                                                               
    static bool hardware_initialized = false;

    if (!hardware_initialized) {
        // Enable Clocks for Port B
        __HAL_RCC_GPIOB_CLK_ENABLE();                                           // "Release" the Port B (power on)
        
        // Enable Clocks for Alternate Function I/O (Required for remapping)
        __HAL_RCC_AFIO_CLK_ENABLE(); 
        
        // Disable JTAG but keep SWD enabled to release PB3 and PB4 during init
        __HAL_AFIO_REMAP_SWJ_NOJTAG();
                                                                                
		// 4= tri state/in, 3= out max 50Mhz, 2= out max 2MHz, 1= out max 10MHz                                              
        GPIOB->CRL = 0x22222444;                                                // PB3 to PB7 as output
        GPIOB->CRH = 0x44444442;                                                // PB8 as output
        hardware_initialized = true;                                            
    }                                          
     
     DELAY_MS(50);                                                             // Power up delay
     
     // Force 8-bit mode 3 times to sync
     lcd_nib(0x30, 0);                                                         // 0011 0000 = init protocol, 0 = send command
     DELAY_MS(5);
     lcd_nib(0x30, 0);
     DELAY_US(200);                                                             // datasheet = 100us
     lcd_nib(0x30, 0);
     DELAY_US(200);
         
         // Switch to 4-bit mode
     lcd_nib(0x20,0);                                                          // 0010 0000 = 4 bits mode
     DELAY_MS(2);                                                               // Time to switch
         
     // In 4-bit mode, lcd_send
     lcd_send(0x28);                                                            // 0010 1000 = 2 lines, 5x8 dots per character
     lcd_send(0x0C);                                                            // Page 24: 0000 1111 = 0x0F = display ON, cursor ON and blink ON; 0x0C = display ON, cursor OFF
     lcd_send(0x06);                                                            // 0000 0110 = entry mode (print a character to the right)
     lcd_clear();
}


/* -----------------------------------------------------------------------------*/
/*   Blink the cursor at given row & column position    */
void lcd_blink(char row, char col){
     if(row == 1) lcd_send(0x80 | (col-1));                                     // Force Address jump (This stops the "auto-increment" from sticking)
     else         lcd_send(0xC0 | (col-1));
     
     lcd_send(0x0D);                                                            // Page 24: 0000 1110 = 0x0E = display ON, cursor ON and blink OFF; 0000 1101 = 0x0D = display ON, cursor OFF and blink ON
     DELAY_US(100);                                                             // 37us is the datasheet minimum, 1ms is safe for all clones
}


/* -----------------------------------------------------------------------------*/
/*   Stop the blink effect    */
void lcd_noBlink(){
    lcd_send(0x0C);                                                             // Display ON, Cursor OFF, Blink OFF
    DELAY_US(50);
}


/* -----------------------------------------------------------------------------*/
/* Clear the display (datasheet page 24)*/
void lcd_clear(){
     lcd_send(0x01);                                                            // 0000 0001 clean and return home automatically
     DELAY_MS(2);                                                               // Mandatory 2ms delay for this specific command
}

/* -----------------------------------------------------------------------------*/
/* Send command */
void lcd_send(unsigned char cmd){
     lcd_nib(cmd & 0xF0, 0);                                                   // send the most significant cmd nibble
     lcd_nib(cmd << 4, 0);                                                     // shift left 4x and send the less significant cmd nibble
     DELAY_US(50);                                                              // Wait for the LCD to process the byte
}


/* -----------------------------------------------------------------------------*/
/* Send the most signigicant nibble */
void lcd_nib(unsigned char nib, char rsel){                                    // send a bit of command 
     if((nib>>4)&0x01)
     	set_bit(PB,D4);                                                         // send bit 4
     else 
        clr_bit(PB,D4);                                                         // clear bit 4
	 
	  if((nib>>5)&0x01)                                                          // ...
     	set_bit(PB,D5);
     else
        clr_bit(PB,D5);
        
     if((nib>>6)&0x01)
     	set_bit(PB,D6);
     else
        clr_bit(PB,D6);
        
     if((nib>>7)&0x01)
     	set_bit(PB,D7);
     else
        clr_bit(PB,D7);
        
     
	  if(rsel)
     	set_bit(PB,RS);
     else
        clr_bit(PB,RS);
	 

    // Pulse enable 
     set_bit(PB,EN);
     DELAY_US(2);                                                               // Hardware latching
    
	  clr_bit(PB,EN);   
     DELAY_US(2);                                                                // Stability delay  
    
    
}


/* -----------------------------------------------------------------------------*/
/*write one character at defined position, datasheet page 24 (Set DDRAM address)*/
void lcd_char(char row, char col, unsigned char chr){                            // send command for column position
     if(row==1){                                                                // row 1?
        lcd_send(0x80|col-1);                                                   // concatenate with "col"; 0x80 standard (row 1, column 1)
                                                                                // example: 0x80 = 10000000
                                                                                //         col 4 = 00000100
                                                                                //           cmd = 10000100 = 0x84 = (row 1, column 4)
                                                                                
        lcd_chrCp(chr);                                                            // write the reiceived character
        }
     else{                                                                      // else row = 2
        lcd_send(0xC0|col-1);                                                   // concatenate with "col"; 0x80 standard (row 1, column 1)
                                                                                //                         0x40 standard (row 2, column 1)
                                                                                // example: 0x80 = 10000000
                                                                                //          0x40 = 01000000 = 0xC0
                                                                                //         col 4 = 00000100
                                                                                //           cmd = 11000100 = 0xC4 = (row 2, column 4)
        lcd_chrCp(chr);                                                            // write the reiceived character
        }
}

/* -----------------------------------------------------------------------------*/
/* write a string at given cursor position */
void lcd_str(char row, char col, char *str){
     register int i;

     for(i=0;str[i] != '\0';i++,col++)                                          // loop until find NULL character
         lcd_char(row,col,str[i]);                                              // print the current character of received string

}

/* -----------------------------------------------------------------------------*/
/* write a character at current cursor position */
void lcd_chrCp(unsigned char chr){                                             // the compiler convert ASCII char to hexadecimal
     lcd_nib(chr & 0xF0, 1);                                                   // send the most significant cmd nibble, parameter = 1 to send data                         
     lcd_nib(chr << 4, 1);                                                     // shift left 4x and send the less significant cmd nibble
     DELAY_US(50);
}


/* -----------------------------------------------------------------------------*/
/* convert up to 5 digit integer and excludes left zeroes */
// To count more than 65535, use unsigned long (requires a lot of RAM)
void lcd_num(char row, char col, unsigned int num, dispFormat range){           // DispFormat: VAL_UNI, VAL_DEZ, VAL_CEN, VAL_MIL, VAL_DEZM
     char dezM, mil, cen, dez, uni, size;
     short noZero = 0;
     
     switch(range){
     
           case VAL_DEZM:
              /* convert up to 5 digit integer and excludes left zeroes */ 
           dezM = (char)(num/10000);
           num %= 10000; 
           mil  = (char)(num/1000);
           num %= 1000;    
           cen  = (char)(num/100);
           num %= 100;
           dez  = (char)(num/10);
           uni  = (char)(num%10);        
           
            if(!dezM && !noZero)                                                // if variable is zero and noZero is = zero
               lcd_char(row,col,' ');                                           // print a blanc space (to delete the left zeroes)
            else{ 
               lcd_char(row,col,dezM+0x30);                                     // else sum '0' (ASCII) to the variable and print at given row and column
               noZero = 1;                                                      // when noZero = 1, all remaining checks will be false in order to fill up the next digits
        }  
           
           if(!mil && !noZero) lcd_chrCp(' ');                                                     
                                                               
           else{
              lcd_chrCp(mil+0x30);                                            
              noZero = 1;                                                             
           } 
               
           if(!cen && !noZero) lcd_chrCp(' ');
               
           else{
              lcd_chrCp(cen+0x30);
              noZero = 1;
              }

           if(!dez && !noZero) lcd_chrCp(' ');
              
           else{
              lcd_chrCp(dez+0x30);
              noZero = 1;
              }
              
           lcd_chrCp(uni+0x30);                                                 // no test about zero because to print a zero here has to be possible
        break;
     
        case VAL_MIL:
              /* convert up to 4 digit integer and excludes left zeroes */  
           mil  = (char)(num/1000);
           num %= 1000;    
           cen  = (char)(num/100);
           num %= 100;
           dez  = (char)(num/10);
           uni  = (char)(num%10);        
           
           if(!mil && !noZero) lcd_char(row,col,' ');
                                                        
           else{
              lcd_char(row,col,mil+0x30);
              noZero = 1;                                                       
           } 
               
           if(!cen && !noZero) lcd_chrCp(' ');
               
           else{
              lcd_chrCp(cen+0x30);
              noZero = 1;
              }

           if(!dez && !noZero) lcd_chrCp(' ');
              
           else{
              lcd_chrCp(dez+0x30);
              noZero = 1;
              }
              
           lcd_chrCp(uni+0x30);                                                   
        break;
     
        case VAL_CEN:
              /* convert up to 3 digit integer and excludes left zeroes */      
           cen  = (char)(num/100);
           num %= 100;
           dez  = (char)(num/10);
           uni  = (char)(num%10);        
               
           if(!cen && !noZero) lcd_char(row,col,' ');
               
           else{
              lcd_char(row,col,cen+0x30);
              noZero = 1;
              }

           if(!dez && !noZero) lcd_chrCp(' ');
              
           else{
              lcd_chrCp(dez+0x30);
              noZero = 1;
              }
              
           lcd_chrCp(uni+0x30);                                                      
        break;
        
        case VAL_DEZ:
              /* convert up to 2 digit integer and excludes left zeroes */      
           dez  = (char)(num/10);
           uni  = (char)(num%10);        
               
           if(!dez && !noZero) lcd_char(row,col,' ');
              
           else{
              lcd_char(row,col,dez+0x30);
              noZero = 1;
              }
              
           lcd_chrCp(uni+0x30);                                                      
        break;
        
        case VAL_UNI:
              lcd_char(row,col,num+0x30);
        break;
        
        default:
              lcd_char(row,col,num+0x30);
     }                         
}


/* -----------------------------------------------------------------------------*/
/* convert up to 5 digit integer including left zeroes */
// To count more than 65535, use unsigned long (requires a lot of RAM)
void lcd_numZero(char row, char col, unsigned int num, dispFormat range){       // DispFormat: VAL_UNI, VAL_DEZ, VAL_CEN, VAL_MIL, VAL_DEZM
     char dezM, mil, cen, dez, uni, size;
     
     switch(range){
     
         case VAL_DEZM:
            /* convert up to 5 digit integer with left zeroes */ 
         dezM = (char)(num/10000);
         num %= 10000; 
         mil  = (char)(num/1000);
         num %= 1000;    
         cen  = (char)(num/100);
         num %= 100;
         dez  = (char)(num/10);
         uni  = (char)(num%10);        
         
         if(!dezM) lcd_char(row,col,'0');                                                                                                     
         else lcd_char(row,col,dezM+0x30);                                      // else sum '0' (ASCII) to the variable and print at given row and column

         if(!mil) lcd_chrCp('0');                                                                                                       
         else lcd_chrCp(mil+0x30);
                                                                    
         if(!cen) lcd_chrCp('0');             
         else lcd_chrCp(cen+0x30);

         if(!dez) lcd_chrCp('0');
         else lcd_chrCp(dez+0x30);
              
         lcd_chrCp(uni+0x30);                                                
         break;
     
         case VAL_MIL:
            /* convert up to 4 digit integer with left zeroes */  
         mil  = (char)(num/1000);
         num %= 1000;    
         cen  = (char)(num/100);
         num %= 100;
         dez  = (char)(num/10);
         uni  = (char)(num%10);        
           
         if(!mil) lcd_char(row,col,'0');
         else lcd_chrCp(mil+0x30);
                                                                    
         if(!cen) lcd_chrCp('0');             
         else lcd_chrCp(cen+0x30);

         if(!dez) lcd_chrCp('0');
         else lcd_chrCp(dez+0x30);
              
         lcd_chrCp(uni+0x30);                                                
         break;
     
         case VAL_CEN:
            /* convert up to 3 digit integer with left zeroes */      
         cen  = (char)(num/100);
         num %= 100;
         dez  = (char)(num/10);
         uni  = (char)(num%10);        
            
         if(!cen) lcd_char(row,col,'0');
         else lcd_chrCp(cen+0x30);

         if(!dez) lcd_chrCp('0');
         else lcd_chrCp(dez+0x30);
              
         lcd_chrCp(uni+0x30);                                                
         break;
        
         case VAL_DEZ:
            /* convert up to 2 digit integer with left zeroes */      
         dez  = (char)(num/10);
         uni  = (char)(num%10);        
            
         if(!dez) lcd_char(row,col,'0');
         else lcd_chrCp(dez+0x30);
              
         lcd_chrCp(uni+0x30);                                                
         break;
        
         case VAL_UNI:
          /* convert up to 1 digit integer with left zeroes */  
              lcd_char(row,col,num+0x30);
         break;
        
         default:
              lcd_char(row,col,num+0x30);
     }                         
}

/* -----------------------------------------------------------------------------*/

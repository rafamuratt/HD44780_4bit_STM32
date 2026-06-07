# HD44780_4bit_STM32

> This library was developed based on concepts studied in C Programming Course (Professor Dr. h.c. Eng. Wagner Rambo), with complete architectural refactoring, redesigned API, and added multi-platform support.

Lightweight C++ library for driving a generic HD44780-compatible 16x2 LCD in 4-bit mode on the STM32F103C8T6 (BluePill), written for the Arduino IDE with the STM32duino core. Direct PortB register manipulation is used instead of `digitalWrite()` calls, keeping the interface fast and suitable for time-sensitive embedded applications. The API is designed to be readable and reusable as a drop-in module for future projects.

---

## 🛠 Hardware Stack

* **MCU:** STM32F103C8T6 (BluePill)
* **Display:** Generic HD44780-compatible 16x2 LCD
* **Mode:** 4-bit parallel
* **Development Environment:** Arduino IDE with STM32duino core
* **Language:** C++

### Pin Mapping (fixed — PortB)

| LCD Pin | STM32 Pin |
|---------|-----------|
| RS      | PB3       |
| EN      | PB4       |
| D4      | PB5       |
| D5      | PB6       |
| D6      | PB7       |
| D7      | PB8       |

> ⚠️ PB3 and PB4 are JTAG pins by default. The library automatically remaps them at init (`SWJ_NOJTAG`), keeping SWD active. Ensure your debugger uses SWD, not JTAG.

---

## 📂 Project Structure

```
/MT_lcd_16x2_4bits_STM32.h      — Public API and dispFormat enum definition
/MT_lcd_16x2_4bits_STM32.cpp    — Full implementation
/MT_lcd_16x2_pins_STM32.h       — Pin mapping and PortB register definition
```

---

## ⚙ Installation (Arduino IDE)

1. Download or clone this repository.
2. Copy the three files (`MT_lcd_16x2_4bits_STM32.h`, `MT_lcd_16x2_4bits_STM32.cpp`, `MT_lcd_16x2_pins_STM32.h`) into your project sketch folder, or place them inside a named folder in your Arduino `libraries/` directory.
3. Include the library in your sketch:

```cpp
#include "MT_lcd_16x2_4bits_STM32.h"
```

4. Call `lcd_begin()` once inside `setup()` before any display operations.

---

## 🚀 API Reference

### Initialisation

**`void lcd_begin()`**
Initialises PortB clocks and pin directions, remaps JTAG to release PB3/PB4, and runs the full HD44780 power-on sequence (force 8-bit × 3 → switch to 4-bit → configure 2 lines, 5×8 dots, display on). Safe to call multiple times — hardware init runs only once via an internal flag.

---

### Display Control

**`void lcd_clear()`**
Clears all characters and returns the cursor to home (row 1, col 1). Includes the mandatory 2ms post-command delay required by the HD44780 datasheet.

**`void lcd_send(unsigned char cmd)`**
Sends a raw one-byte command to the LCD controller. Use for direct HD44780 register access when needed (e.g. custom entry modes or CGRAM addressing).

---

### Cursor

**`void lcd_blink(char row, char col)`**
Moves the cursor to the specified position (1-indexed) and enables the blinking block cursor. Useful for indicating an editable field on the HMI.

**`void lcd_noBlink()`**
Disables the blinking cursor. Display remains on with no visible cursor.

---

### Writing Characters and Strings

**`void lcd_char(char row, char col, unsigned char chr)`**
Writes a single ASCII character at the given row (1–2) and column (1–16) position.

**`void lcd_chrCp(unsigned char chr)`**
Writes a single ASCII character at the current cursor position, without repositioning first. Used internally for sequential multi-character writes.

**`void lcd_str(char row, char col, char *str)`**
Writes a null-terminated string starting at the given row and column. Advances column automatically for each character until `\0` is reached.

---

### Numeric Display

Both functions accept a `dispFormat` enum to define the maximum number of digits to render:

| Enum value  | Max digits | Range   |
|-------------|------------|---------|
| `VAL_UNI`   | 1          | 0–9     |
| `VAL_DEZ`   | 2          | 0–99    |
| `VAL_CEN`   | 3          | 0–999   |
| `VAL_MIL`   | 4          | 0–9999  |
| `VAL_DEZM`  | 5          | 0–65535 |

> Numbers above 65535 require `unsigned long` — note this increases RAM usage.

**`void lcd_num(char row, char col, unsigned int num, dispFormat range)`**
Converts an integer and prints it at the given position, suppressing leading zeroes (replaced by spaces). Suitable for live value readouts where left-padding with spaces is preferred.

**`void lcd_numZero(char row, char col, unsigned int num, dispFormat range)`**
Same as `lcd_num` but preserves leading zeroes. Suitable for fixed-width displays such as counters, timestamps, or formatted sensor readings.

---

## 💡 Usage Example

```cpp
#include "MT_lcd_16x2_4bits_STM32.h"

void setup() {
    lcd_begin();
    lcd_str(1, 1, "Hello World!");
    lcd_num(2, 1, 42, VAL_CEN);       // prints " 42" (leading space, 3 digits)
    lcd_numZero(2, 9, 7, VAL_CEN);    // prints "007" (leading zeroes, 3 digits)
}

void loop() {}
```

---

## 📜 License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0).  
See the full license text at: https://www.gnu.org/licenses/gpl-3.0.html.

---

☕ If this project is helpful for your application, please consider supporting:<br> 
https://www.paypal.com/donate/?hosted_button_id=8S8BJ9TT368VN  

Built by **rafamuratt**: https://murat-tech.eu/  
Murat-Tech Channel: https://www.youtube.com/@Murat-TechChannel-EN

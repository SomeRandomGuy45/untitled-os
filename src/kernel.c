#include "include/enums.h"
#include "drivers/ports.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) 
{
    if (c == '\n')
    {
        terminal_row += 1;
        terminal_column = 0;
        return;
    }
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void terminal_log_data(const char* data, enum LogType logType)
{
    enum vga_color color = VGA_COLOR_GREEN;
    char* dataType = "[SUCCESS] ";
    if (logType == ERROR)
    {
        color = VGA_COLOR_RED;
        dataType = "[ERROR] ";
    } else if (logType == NONE)
    {
        color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        dataType = "[INFO] ";
    }
    terminal_setcolor(color);
    terminal_writestring(dataType);
    terminal_writestring(data);
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void main()
{

	terminal_initialize();

	terminal_log_data("Booted OS: Version 1.0\n", 0);
	terminal_log_data("Waiting for tasks\n", 1);
	terminal_log_data("Trying to get out of VGA\n", 1);

	port_byte_out(0x3d4, 14);
	int position = port_byte_in(0x3d5);
    position = position << 8; /* high byte */

    port_byte_out(0x3d4, 15); /* requesting low byte */
    position += port_byte_in(0x3d5);

    /* VGA 'cells' consist of the character and its control data
     * e.g. 'white on black background', 'red text on white bg', etc */
    int offset_from_vga = position * 2;

	char *vga = (char*) 0xb8000;
    vga[offset_from_vga] = 'X'; 
    vga[offset_from_vga+1] = 0x0f;

    /*
		terminal_initialize();

		terminal_log_data("Booted OS: Version 1.0\n", 0);
		terminal_log_data("Waiting for tasks\n", 1);
		terminal_log_data("Trying to get out of VGA\n", 1);
	*/
}
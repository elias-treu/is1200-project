// Define GPIO addresses
#define GPIO_START ((volatile int *)0x04000000)

#define REGISTER_SELECT (GPIO_START + 1) // 0x04000004
#define READ_WRITE (GPIO_START + 2)      // 0x04000008
#define ENABLE (GPIO_START + 3)          // 0x0400000C
#define DATA (GPIO_START + 4)            // 0x04000010

 
void set_cursor(int position) {}

void write_symbol(char symbol, int position) {}

void write_line(char *text, int line) {}

int main() {}

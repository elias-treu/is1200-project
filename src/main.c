#include "screen.h"

extern void print(const char *);

void handle_interrupt(unsigned cause) { (void)cause; }

int main() {
  lcd_init();

  set_cursor(0, 0);
  write_string("TEST STRING!");

  set_cursor(1, 0);
  write_string("LOWER STRING!");
  print("end of main()\n");
  while (1)
    ;
}

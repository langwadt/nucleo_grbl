
// @100MHz
#define COUNTS_PER_MICROSECOND 33
void _delay_us(int d);
void _delay_ms(int d);

void _delay_us(int d)
{
  unsigned int count = d * COUNTS_PER_MICROSECOND - 2;
  __asm volatile(" mov r0, %[count]  \n\t"
  "1: subs r0, #1            \n\t"
  "   bhi 1b                 \n\t"
  :
  : [count] "r" (count)
  : "r0");
}

void _delay_ms(int d)
{
  while (d--) _delay_us(999);
}

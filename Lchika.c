#include <arm/am335x.h>
#include <hw/inout.h>
#include <stdint.h>
#include <stdio.h>
#include<stdlib.h>
#include <sys/mman.h>
#define LED0 (1 << 21)
#define LED1 (1 << 22)
#define LED2 (1 << 23)
#define LED3 (1 << 24)
#define LEDS(n) ((n & 0xf) << 21)
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("引数は4ケタの2進数のみにしてください\n");
    return 0;
  }
  char *input = argv[1];
  int tmp=atoi(input);
  int bit=0;
  int base=1;
  //2進数を10進数変換
  while(tmp>0){
    bit = bit + ( tmp % 10 ) * base;
    tmp = tmp / 10;
    base = base * 2;
  }



  // 0x4804C000から0x00001000文の領域アクセス権利を取得
  const uintptr_t gpio_base =
      mmap_device_io(AM335X_GPIO_SIZE, AM335X_GPIO1_BASE);
  // in,outを用いてその領域へのアクセスを行うinが読み出し
  const uint32_t oe_value = in32(gpio_base + GPIO_OE);
  out32(gpio_base + GPIO_OE, oe_value & ~(LED0 | LED1 | LED2 | LED3));

  const uint32_t data_value =
      in32(gpio_base + GPIO_DATAOUT) & ~(LED0 | LED1 | LED2 | LED3);
  out32(gpio_base + GPIO_DATAOUT, data_value |LEDS(bit) );
  
  

  munmap_device_io(gpio_base, AM335X_GPIO_SIZE);

  return 0;
}
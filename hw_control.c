#include <libopencm3/stm32/rcc.h>    
#include <libopencm3/stm32/flash.h>  
#include <libopencm3/stm32/gpio.h>   
#include <libopencm3/stm32/usart.h>  
#include <libopencm3/stm32/timer.h>  
#include <libopencm3/cm3/nvic.h>     
#include <libopencm3/cm3/scb.h> 
#include <stdio.h>    

#include "grbl.h"


int timerrunning = 0;

void init_step_timer(void)
{
  
  nvic_set_priority(NVIC_DMA1_STREAM6_IRQ,255);
  nvic_set_priority(NVIC_TIM2_IRQ,0);
  
  rcc_periph_clock_enable(RCC_TIM2);
  nvic_enable_irq(NVIC_TIM2_IRQ);

  timer_reset(TIM2);
  timer_set_prescaler(TIM2, 4); // 50/5 = 10MHz
  timer_set_period(TIM2, 5000);
                                    
  timer_disable_preload(TIM2);                                  
                                    
  timer_set_oc_value(TIM2,TIM_OC1,50);

  timerrunning = 1;  
  
  timer_enable_irq(TIM2, TIM_SR_UIF);
  nvic_enable_irq(NVIC_TIM2_IRQ);
  nvic_enable_irq(NVIC_DMA1_STREAM6_IRQ);
  timer_enable_update_event(TIM2); 
  timer_enable_irq(TIM2, TIM_DIER_CC1IE);
  timer_enable_counter(TIM2);
  
  
}


void set_step_timer_period(int p)
{
  timer_set_period(TIM2, p); 
}

void enable_step_timer_isr(void)
{
  timerrunning = 1;
  //timer_enable_irq(TIM2, TIM_SR_UIF);
}


void disable_step_timer_isr(void)
{
  timerrunning = 0;
  //timer_disable_irq(TIM2, TIM_SR_UIF);
}

void init_step_pins(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);

    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);
    
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
}

void init_spindle()
{
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_TIM3);

  timer_reset(TIM3);

  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0);

  gpio_set_af(GPIOB, GPIO_AF2, GPIO0);

  

  timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, // clock division
                        TIM_CR1_CMS_EDGE,   // Center-aligned mode selection
                        TIM_CR1_DIR_UP);    // TIMx_CR1 DIR: Direction
  
  timer_continuous_mode(TIM3);             // Disables TIM_CR1_OPM (One pulse mode)
  timer_set_period(TIM3, 20000-1);                    // Sets TIMx_ARR
  timer_set_prescaler(TIM3, 50-1);               // Adjusts speed of timer
  timer_enable_preload(TIM3);                        // Set ARPE bit in TIMx_CR1
   
  timer_enable_update_event(TIM3); 
  // Channel-specific settings
  timer_set_oc_value(TIM3, TIM_OC3, 1000);             // sets TIMx_CCRx
  timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);   // Sets PWM Mode 1
  timer_enable_oc_preload(TIM3, TIM_OC3);           // Sets OCxPE in TIMx_CCMRx
  timer_set_oc_polarity_high(TIM3, TIM_OC3);        // set desired polarity in TIMx_CCER
  timer_enable_oc_output(TIM3, TIM_OC3);             // set CCxE bit in TIMx_CCER  (enable output)

  timer_enable_counter(TIM3);
  timer_generate_event(TIM3, TIM_EGR_UG);


}



uint32_t spibytes(uint8_t data0,uint8_t data1,uint8_t data2);
void init_steppers(void)
{
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);


  gpio_clear(GPIOA,GPIO9);
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);  // reset

  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT,  GPIO_PUPD_PULLUP, GPIO10);  // stepper flag 

  gpio_set(GPIOB,GPIO6);
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);  // cs
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);  // mosi
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT,  GPIO_PUPD_PULLDOWN, GPIO6);  // miso
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);  // sck

  gpio_set(GPIOA,GPIO9);


  spibytes(0xa8,0xa8,0xa8);  // disable

  spibytes(0xd0,0xd0,0xd0);  // config
  spibytes(0x00,0x00,0x00);
  spibytes(0x00,0x00,0x00);

  spibytes(0x18,0x18,0x18);
  spibytes(0x2c,0x2c,0x2c);
  spibytes(0x88,0x88,0x88);

  spibytes(0x16,0x16,0x16);
  spibytes(0x88|4,0x88|4,0x88|4);  // 1/16 step

  spibytes(0x09,0x09,0x9);
  spibytes(floor(500/31),floor(1500/31),floor(2000/31));      // 31mA per

  spibytes(0x13,0x13,0x13);  // overcurrent ~3.5A
  spibytes(0xf,0xf,0xf);


  spibytes(0xb8,0xb8,0xb8);  // enable Z,X,Y


  init_spindle();


}


void printstepperstatus(void)
{
    char buf[32];
    unsigned int stat,stat1;
    
    spibytes(0xd0,0xd0,0xd0);
    stat = spibytes(0x00,0x00,0x00);
    stat1= spibytes(0x00,0x00,0x00);
        
    sprintf(buf,"0x%2.2x%2.2x 0x%2.2x%2.2x 0x%2.2x%2.2x\n\r",(stat>>16)&0xff,(stat1>>16)&0xff,(stat>>8)&0xff,(stat1>>8)&0xff,(stat>>0)&0xff,(stat1>>0)&0xff);
    printPgmString(buf); 

}


void set_step_pins(uint8_t pins)
{
  if(pins & (1<<Y_STEP_BIT)) gpio_set(GPIOC, GPIO7);  else  gpio_clear(GPIOC, GPIO7);
  if(pins & (1<<X_STEP_BIT)) gpio_set(GPIOB, GPIO3);  else  gpio_clear(GPIOB, GPIO3);
  if(pins & (1<<Z_STEP_BIT)) gpio_set(GPIOB, GPIO10); else  gpio_clear(GPIOB, GPIO10);
}

void set_dir_pins(uint8_t pins)
{
  if(pins & (1<<Z_DIRECTION_BIT)) gpio_clear(GPIOB, GPIO4); else  gpio_set(GPIOB, GPIO4);
  if(pins & (1<<X_DIRECTION_BIT)) gpio_set(GPIOB, GPIO5);   else  gpio_clear(GPIOB, GPIO5);
  if(pins & (1<<Y_DIRECTION_BIT)) gpio_set(GPIOA, GPIO8);   else  gpio_clear(GPIOA, GPIO8);
}

int limits_enabled = 0;

uint8_t oldlimits ;
uint8_t newlimits ;

void limits_init(void)
{
  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO0);  // X
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO1);  // Y
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO4);  // Z    

  newlimits = limits_get_state();
  oldlimits = oldlimits;
  limits_enabled = 1; 
}


uint8_t limits_get_state(void)
{
  uint8_t limitstate =0;
  
  if(gpio_get(GPIOA,GPIO1)==0)     limitstate |= 1<<1; // X
  if(gpio_get(GPIOA,GPIO0)==0)     limitstate |= 1<<0; // Y
  if(gpio_get(GPIOA,GPIO4)==0)     limitstate |= 1<<2; // Z

  return limitstate;
}

void limits_disable(void)
{
  limits_enabled = 0;
}
///////////////////////////////////////////////////////////////////////
extern void main_step_isr(void);
extern void limit_isr(void);

void dma1_stream6_isr(void)
{
  nvic_clear_pending_irq(NVIC_DMA1_STREAM6_IRQ);
  
    gpio_set(GPIOC,GPIO0);

    if(timerrunning) main_step_isr();
    
    if(limits_enabled)
    {
      newlimits = limits_get_state();
      if(( oldlimits != newlimits) && (newlimits & 0x7))
        limit_isr();

      oldlimits = newlimits;
    }

    
    if(gpio_get(GPIOA,GPIO10)==0)
    {
      printstepperstatus();

      bit_true_atomic(sys.rt_exec_alarm, (EXEC_ALARM_SOFT_LIMIT|EXEC_CRITICAL_EVENT)); // Indicate soft limit critical event
      protocol_execute_realtime(); // Execute to enter critical event loop and system abort
    }        

    gpio_clear(GPIOC,GPIO0);
    
}


void tim2_isr(void)
{
  if (timer_get_flag(TIM2, TIM_SR_UIF))
  {
    timer_clear_flag(TIM2, TIM_SR_UIF);

    nvic_set_pending_irq(NVIC_DMA1_STREAM6_IRQ);
   
  }
  
  if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
    timer_clear_flag(TIM2, TIM_SR_CC1IF);
    
     set_step_pins(0);
    
  }
  
}

#define SPI1SCK  GPIOA,GPIO5
#define SPI1NSS  GPIOB,GPIO6
#define SPI1MOSI GPIOA,GPIO7
#define SPI1MISO GPIOA,GPIO6


uint32_t spibytes(uint8_t data0,uint8_t data1,uint8_t data2)
{
	uint32_t res=0;
	int i;

	gpio_clear(SPI1SCK);
	gpio_clear(SPI1NSS);

	for(i=0;i<8;i++)
	{
		gpio_clear(SPI1SCK);
		if(data0&(0x80>>i)) gpio_set(SPI1MOSI); else gpio_clear(SPI1MOSI);
		gpio_set(SPI1SCK);
		res = (res<<1) | (gpio_get(SPI1MISO)==0 ? 0:1 );
	}

	for(i=0;i<8;i++)
	{
		gpio_clear(SPI1SCK);
		if(data1&(0x80>>i)) gpio_set(SPI1MOSI); else gpio_clear(SPI1MOSI);
		gpio_set(SPI1SCK);
		res = (res<<1) | (gpio_get(SPI1MISO)==0 ? 0:1 );
	}
	for(i=0;i<8;i++)
	{
		gpio_clear(SPI1SCK);
		if(data2&(0x80>>i)) gpio_set(SPI1MOSI); else gpio_clear(SPI1MOSI);
		gpio_set(SPI1SCK);
		res = (res<<1) | (gpio_get(SPI1MISO)==0 ? 0:1 );
	}

	gpio_set(SPI1NSS);

	return res;

}

uint8_t probe_invert_mask;

// Probe pin initialization routine.
void probe_init() 
{
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO1);  // X
    
  probe_configure_invert_mask(false); // Initialize invert mask. Not required. Updated when in-use.
}

// Called by probe_init() and the mc_probe() routines. Sets up the probe pin invert mask to 
// appropriately set the pin logic according to setting for normal-high/normal-low operation 
// and the probing cycle modes for toward-workpiece/away-from-workpiece. 
void probe_configure_invert_mask(uint8_t is_probe_away)
{
  if(is_probe_away==true) 
    probe_invert_mask = 1; 
  else
    probe_invert_mask = 0; 
}


// Returns the probe pin state. Triggered = true. Called by gcode parser and probe state monitor.
uint8_t probe_get_state() 
{ 
  if(gpio_get(GPIOC,GPIO1)==probe_invert_mask) 
    return true; 
  else  
    return false;
}

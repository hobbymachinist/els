#include "clock.h"

//==============================================================================
// Clock and peripherals setup
//==============================================================================

const struct rcc_clock_scale rcc_hse_config = {
  .pllm = 8,
  .plln = 360,
  .pllp = 2,
  .pllq = 4,
  .pllr = 0,
  .pll_source = RCC_CFGR_PLLSRC_HSE_CLK,
  .hpre = RCC_CFGR_HPRE_NODIV,
  .ppre1 = RCC_CFGR_PPRE_DIV4,
  .ppre2 = RCC_CFGR_PPRE_DIV2,
  .voltage_scale = PWR_SCALE1,
  .flash_config = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_LATENCY_5WS,
  .ahb_frequency  = 180000000,
  .apb1_frequency =  45000000,
  .apb2_frequency =  90000000,
};

#define RCC_HSE_CONFIG &rcc_hse_config

//------------------------------------------------------------------------------
// Set STM32 using 8MHz HSE.
//------------------------------------------------------------------------------
void els_clock_setup(void) {
  rcc_clock_setup_pll(RCC_HSE_CONFIG);

  // Enable clocks for GPIOA, GPIOB, GPIOC, USART1, SYSCFG (required for port mapping).
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_USART1);
  rcc_periph_clock_enable(RCC_SYSCFG);
}

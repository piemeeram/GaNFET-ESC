/************************************************ Library ************************************************/

#include "sysinit.h"

/********************************************* Used functions ********************************************/

void InitRCC (void){
	uint8_t ticks = 0;													//(1us = 168 ticks @ 168MHz)

	RCC		-> CFGR			|= RCC_CFGR_HPRE_DIV2;						// AHB divided by 2 before changing clock

	PWR		-> CR1			|= PWR_CR1_VOS_0;							// Ensure range 1
	PWR		-> CR5			&= ~PWR_CR5_R1MODE;							// Range 1 from normal to boost mode

	RCC		->CR			|= ((uint32_t)RCC_CR_HSEON); 				// Enable HSE
	while (!(RCC->CR & RCC_CR_HSERDY));									// Ready to start HSE

	RCC		-> CR			|= RCC_CR_CSSON;

	FLASH	-> ACR			|= FLASH_ACR_PRFTEN;
	FLASH	-> ACR			|= FLASH_ACR_LATENCY_4WS;					// See table 9

	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLPEN;						// clear PLLPEN bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLP;						// clear PLLP bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLQEN;						// clear PLLQEN bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLQ;						// clear PLLQ bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLREN;						// clear PLLREN bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLR;						// clear PLLR bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLN;						// clear PLLN bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLM_0;						// clear PLLM bits
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLSRC;               		// clear PLLSRC bits

	RCC		-> PLLCFGR		|= RCC_PLLCFGR_PLLSRC_HSE; 					// source HSE = 8MHz
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLM_0; 					// PLLM = HSE/1 = 8 MHz
	RCC		-> PLLCFGR		|= 0x29UL << RCC_PLLCFGR_PLLN_Pos; 			// PLLN = PLLM*42 = 336 MHz

	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLR; 						// PLLR = PLLN/2 = 168 MHz
	RCC		-> PLLCFGR		|= RCC_PLLCFGR_PLLREN;

	RCC		-> PLLCFGR		|= RCC_PLLCFGR_PLLSRC; 						// PLLQ = PLLN/8 = 42 MHz (for FD CAN)
	RCC		-> PLLCFGR		|= RCC_PLLCFGR_PLLQEN;

	/*
	 * PLLP is not used now, but probably will be used for ADC
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLP;
	RCC		-> PLLCFGR		&= ~RCC_PLLCFGR_PLLPEN;
	*/

	RCC		-> CR			|= RCC_CR_PLLON;                      		// enable PLL
	while((RCC->CR & RCC_CR_PLLRDY) == 0) {}     						// wait till PLL is ready

	RCC		-> CFGR			&= ~RCC_CFGR_SW;                   			// clear SW bits
	RCC		-> CFGR			|= RCC_CFGR_SW_PLL;               			// select source SYSCLK = PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) {} 				// wait till PLL is used

	//waiting for at least 1us
	while (!(ticks = 200))
	{
		ticks++;
	}

	ticks = 0;

	RCC		-> CFGR			|= RCC_CFGR_HPRE_DIV1;						// AHB = SYSCLK/1
	RCC		-> CFGR			|= RCC_CFGR_PPRE1_DIV1;						// APB1 = HCLK/1
	RCC		-> CFGR			|= RCC_CFGR_PPRE2_DIV1;						// APB2 = HCLK/1

}

void NMI_Handler(void)
{
	 if (RCC -> CIFR & RCC_CIFR_CSSF)
	 {
		 RCC	->	CICR		|=RCC_CICR_CSSC;						//Clock security system interrupt clear
	 }
}

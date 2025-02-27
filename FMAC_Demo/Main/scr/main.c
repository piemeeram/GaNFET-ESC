#include "main.h"

//#if !defined(__SOFT_FP__) && defined(__ARM_FP)
//  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
//#endif

#define	MCU_CLK		168000000
#define	SAMPLE_RATE	48000
#define FREQ1		16000
#define FREQ2		8000
#define RES			32

#define GAIN		2

enum SettingsFMAC{
	COEFF_START_ADDRESS				= 0x00,
	COEFF_A_LENGTH					= 0x03,
	COEFF_B_LENGTH					= 0x03,
	COEFF_GAIN						= GAIN,
	INPUT_START_ADDRESS				= 0x00,
	INPUT_LENGTH					= 0x08,
	INPUT_BUFFER_FULL_THRESHOLD		= 0x00,
	OUTPUT_START_ADDRESS			= 0x00,
	OUTPUT_LENGTH					= 0x04,
	OUTPUT_BUFFER_FULL_THRESHOLD	= 0x00,
};


int16_t coeff[] = {70, 141, 70, 17973, -32627, 14936}; // calculated for fs = 31250

uint16_t input[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint16_t output[4] = {0, 0, 0, 0};

static int16_t sine16[RES*2];
static int16_t data, data_fmac;

int main(void)
{
	static uint16_t amplitude[RES];
	static int16_t angle[RES];
	static int16_t sineq15[RES];

	size_t i = 0;

	InitRCC();
	InitLedGPIO();
	InitUSART1();
	InitDMAusart1();
	InitVrefbuf();
	InitDAC2(MCU_CLK);
	InitDmaDac2();

	InitCORDIC();

	for (i = 0; i < RES; i++)
	{
		amplitude[i] = 0x3FF;
		angle[i] = (0xFFFF/RES)*i;
	}

	QuickFunctionQ15inQ15out(angle, amplitude, sineq15, 0, RES, 0, SINE,6);

	for (i = 0; i < 2*RES; i++)
	{
		sine16[i] = sineq15[i%RES] + sineq15[i/2];
	}
	i = 0;

	InitFmac();
	WriteFmacIirCoefficients(coeff, COEFF_START_ADDRESS, COEFF_A_LENGTH, COEFF_B_LENGTH);
	SelectFmacPreloadFmacY(OUTPUT_LENGTH, OUTPUT_LENGTH, OUTPUT_BUFFER_FULL_THRESHOLD);

	size_t j = 0;

	while(FMAC -> PARAM & FMAC_PARAM_START)
	{
		for (j= 0; j <= sizeof(output); j++)
		{
			WriteFmacData (output[j]);
		}
	}

	SelectFmacIir(INPUT_START_ADDRESS, INPUT_LENGTH, INPUT_BUFFER_FULL_THRESHOLD,
					COEFF_START_ADDRESS, COEFF_A_LENGTH, COEFF_B_LENGTH,
					OUTPUT_START_ADDRESS, OUTPUT_LENGTH, OUTPUT_BUFFER_FULL_THRESHOLD, COEFF_GAIN);
	StopFmacFunction();
	FMAC	-> CR			|= FMAC_CR_RIEN;
	NVIC_EnableIRQ(FMAC_IRQn);
	NVIC_SetPriority(FMAC_IRQn, 5);
	ExecuteFmacFunction();

	xTaskCreate(vTaskLED, "LED", 2048, 0 , 1, &TaskLEDHandle);
	//xTaskCreate(vTaskSendUSART, "SendUSART", 2048, 0 , 1, &TaskSendUSARTHandle);

	vTaskStartScheduler();
	//InitTIM6();
	//InitTIM7();

    while(1)
    {

    }
}

//Sine freq = ((f_osc)/(psc*arr))/(2*res)
void InitTIM6(void)
{
	RCC		-> APB1ENR1	|= RCC_APB1ENR1_TIM6EN;
	TIM6	-> PSC		= 28-1;
	TIM6	-> ARR		= 12-1;
	TIM6	-> DIER		|= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	TIM6	-> CR1		|= TIM_CR1_CEN;
}

//samp freq = ((f_osc)/(psc*arr))
void InitTIM7(void)
{
	RCC		-> APB1ENR1	|= RCC_APB1ENR1_TIM7EN;
	TIM7	-> PSC		= 14-1;
	TIM7	-> ARR		= 12-1;
	TIM7	-> DIER		|= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM7_DAC_IRQn);
	TIM7	-> CR1		|= TIM_CR1_CEN;
}

void vTaskLED (void *pvParameters){

	while(1)
	{
		BlinkLED (GREEN, 100);
	}
}


void vTaskSendUSART(void *pvParameters){

	while(1)
	{
		vTaskDelay(100);
		ToggleLED(YELLOW);
	}
}


void TIM6_DACUNDER_IRQHandler(void)
{
	static size_t i;

	if (i>=(2*RES))
	{
		i = 0;
	}

	if (TIM6 -> SR & TIM_SR_UIF)
	{
		TIM6 -> SR &= ~TIM_SR_UIF;
		data = sine16[i++];
		WriteDataDmaDac2(data);

		ToggleLED(YELLOW);
	}
}

void TIM7_IRQHandler(void)
{
	if (TIM7 -> SR & TIM_SR_UIF)
	{
		TIM7 -> SR &= ~TIM_SR_UIF;

		WriteFmacData(data);
		ToggleLED(BLUE);
	}
}

void FMAC_IRQHandler(void)
{
	if (FMAC -> SR & FMAC_SR_YEMPTY)
	{
		FMAC -> SR &= ~FMAC_SR_YEMPTY;
		//data_fmac = (FMAC -> RDATA);
		//WriteDAC2(data_fmac);
		//ToggleLED(WHITE);
	}
}

void DMA1_CH2_IRQHandler(void)
{
		ToggleLED(WHITE);
		DMA1				-> IFCR		|= DMA_IFCR_CGIF2;
}


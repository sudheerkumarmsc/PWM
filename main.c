#include "stm8s.h"
#include "main.h"
#include "stm8s_tim2.h"

void GPIO_setup(void);
static void TIM2_Config(void);
void TIM4_Config(void);
void TimingDelay_Decrement(void);
void delay(unsigned int dl);



void TIM2_TimeBaseInit( TIM2_Prescaler_TypeDef TIM2_Prescaler,
                        uint16_t TIM2_Period);
												
void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
												

/*void TIM2_OC2Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
*/


void TIM2_OC3Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
									
void TIM2_OC1PreloadConfig(FunctionalState NewState);
void TIM2_OC3PreloadConfig(FunctionalState NewState);
void TIM2_ARRPreloadConfig(FunctionalState NewState);
void TIM2_Cmd(FunctionalState NewState);



uint16_t CCR1_Val = 150; 		//750 for 75% duty cycle		//500 -> 50% duty cycle
uint16_t CCR2_Val = 150;		//250 -> 25 % duty cycle
uint16_t CCR3_Val = 150;		//125 -> 12.5 % duty cycle

int iTrigger_Flag=0;
unsigned int Time_in_min=0;

int Two_Msec=0,One_Sec=0,One_Min=0;

void main(void)
{

	GPIO_setup();
	TIM2_Config();
	TIM4_Config();
	LED1_ON();
	//GPIO_WriteLow(GPIOD, GPIO_PIN_0);
	/* For 75% duty cycle use 10,10,55,10,10,5 
		 For 60% duty cycle use 10,10,40,10,10,20 
		 For 50% duty cycle use 10,10,30,10,10,30*/
		 
  while(1)
	{
		if(GPIO_ReadInputPin(SW_PORT, SW_PIN) == 0)		// If the trigger switch is pressed
		{
			Time_in_min=10;															// Load the Timer variable with required time minutes
			LED1_OFF();
			LED2_ON();
			TIM2_Cmd(ENABLE);
		}
		
		/*if(Time_in_min>0)
		{
			GPIO_WriteHigh(G1_PORT, G1_PIN);		//G1
			delay(10);
			GPIO_WriteHigh(G2_PORT, G2_PIN);		//G2
			delay(10);
			GPIO_WriteHigh(G3_PORT, G3_PIN);		//G3
			delay(30);
			GPIO_WriteLow(G1_PORT, G1_PIN);
			delay(10);
			GPIO_WriteLow(G2_PORT, G2_PIN);
			delay(10);
			GPIO_WriteLow(G3_PORT, G3_PIN);
			delay(30);
		}*/
		
	};

}

void GPIO_setup(void)
{
       GPIO_DeInit(GPIOB);
			 GPIO_DeInit(GPIOF);
			 GPIO_DeInit(GPIOD);
			 
       //GPIO_Init(G1_PORT, G1_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);		//G1
			 //GPIO_Init(G2_PORT, G2_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);		//G2
			 /* The default functionality is SPI Clock so Make it as output for the PWM mode, check the below link for it*/
			 //http://embedded-lab.com/blog/starting-stm8-microcontrollers/20/
			 GPIO_Init(G3_PORT, G3_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);		//G3  
			 GPIO_Init(LED_PORT, LED1_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);		//LED1
			 GPIO_Init(LED_PORT, LED2_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);		//LED2
			 
			 
			 GPIO_Init(SW_PORT, SW_PIN, GPIO_MODE_IN_PU_NO_IT);
}


void TIM4_Config(void)
{
  /* TIM4 configuration:
   - TIM4CLK is set to 2 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 2 MHz / 32 = 62 500 Hz
  - With 62 500 Hz we can generate time base:
      max time base is 4.096 ms if TIM4_PERIOD = 255 --> (255 + 1) / 62500 = 4.096 ms
      min time base is 0.032 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 62500 = 0.032 ms
  - In this example we need to generate a time base equal to 5 ms
   so TIM4_PERIOD = (0.002 * 62500 - 1) = 124 */

  /* Time base configuration */
  TIM4_TimeBaseInit(TIM4_PRESCALER_32, TIM4_PERIOD);
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  /* Enable update interrupt */
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  
  /* enable interrupts */
  enableInterrupts();

  /* Enable TIM4 */
  TIM4_Cmd(ENABLE);
}


void TimingDelay_Decrement(void)
{
	/*
				Timer Resolution is 2msec
				for 1sec -> 500 counts 
				for 1minute -> 500*60 ->30000
				for 10 minutes-> 30000*10 -> 300000
	*/
	Two_Msec++;
	if(Two_Msec>=500)
	{
		Two_Msec=0;
		One_Sec++;
	}
	
	if(One_Sec>=60)
	{
		One_Sec=0;
		One_Min++;
	}
	
	if(One_Min)
	{
		One_Min=0;
		if(Time_in_min>0)
		{
			Time_in_min--;
			if(!Time_in_min)
			{
				LED1_ON();
				LED2_OFF();
				TIM2_Cmd(DISABLE);
			}
		}
	}
	
}
void delay(unsigned int dl)
{
	int i,j;
	for(i = 0; i<dl ; i++);
}


void TIM2_TimeBaseInit( TIM2_Prescaler_TypeDef TIM2_Prescaler,
                        uint16_t TIM2_Period)
{
    /* Set the Prescaler value */
    TIM2->PSCR = (uint8_t)(TIM2_Prescaler);
    /* Set the Autoreload value */
    TIM2->ARRH = (uint8_t)(TIM2_Period >> 8);
    TIM2->ARRL = (uint8_t)(TIM2_Period);
}

void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
{
    /* Check the parameters */
    assert_param(IS_TIM2_OC_MODE_OK(TIM2_OCMode));
    assert_param(IS_TIM2_OUTPUT_STATE_OK(TIM2_OutputState));
    assert_param(IS_TIM2_OC_POLARITY_OK(TIM2_OCPolarity));

    /* Disable the Channel 1: Reset the CCE Bit, Set the Output State , the Output Polarity */
    TIM2->CCER1 &= (uint8_t)(~( TIM2_CCER1_CC1E | TIM2_CCER1_CC1P));
    /* Set the Output State &  Set the Output Polarity  */
    TIM2->CCER1 |= (uint8_t)((uint8_t)(TIM2_OutputState & TIM2_CCER1_CC1E ) | 
                             (uint8_t)(TIM2_OCPolarity & TIM2_CCER1_CC1P));

    /* Reset the Output Compare Bits  & Set the Ouput Compare Mode */
    TIM2->CCMR1 = (uint8_t)((uint8_t)(TIM2->CCMR1 & (uint8_t)(~TIM2_CCMR_OCM)) |
                            (uint8_t)TIM2_OCMode);

    /* Set the Pulse value */
    TIM2->CCR1H = (uint8_t)(TIM2_Pulse >> 8);
    TIM2->CCR1L = (uint8_t)(TIM2_Pulse);
}

void TIM2_OC1PreloadConfig(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONALSTATE_OK(NewState));

    /* Set or Reset the OC1PE Bit */
    if (NewState != DISABLE)
    {
        TIM2->CCMR1 |= (uint8_t)TIM2_CCMR_OCxPE;
    }
    else
    {
        TIM2->CCMR1 &= (uint8_t)(~TIM2_CCMR_OCxPE);
    }
}

void TIM2_OC3Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
{
    /* Check the parameters */
    assert_param(IS_TIM2_OC_MODE_OK(TIM2_OCMode));
    assert_param(IS_TIM2_OUTPUT_STATE_OK(TIM2_OutputState));
    assert_param(IS_TIM2_OC_POLARITY_OK(TIM2_OCPolarity));
    /* Disable the Channel 1: Reset the CCE Bit, Set the Output State, the Output Polarity */
    TIM2->CCER2 &= (uint8_t)(~( TIM2_CCER2_CC3E  | TIM2_CCER2_CC3P));
    /* Set the Output State & Set the Output Polarity */
    TIM2->CCER2 |= (uint8_t)((uint8_t)(TIM2_OutputState & TIM2_CCER2_CC3E) |  
                             (uint8_t)(TIM2_OCPolarity & TIM2_CCER2_CC3P));

    /* Reset the Output Compare Bits & Set the Output Compare Mode */
    TIM2->CCMR3 = (uint8_t)((uint8_t)(TIM2->CCMR3 & (uint8_t)(~TIM2_CCMR_OCM)) |
                            (uint8_t)TIM2_OCMode);

    /* Set the Pulse value */
    TIM2->CCR3H = (uint8_t)(TIM2_Pulse >> 8);
    TIM2->CCR3L = (uint8_t)(TIM2_Pulse);

}

void TIM2_OC3PreloadConfig(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONALSTATE_OK(NewState));

    /* Set or Reset the OC3PE Bit */
    if (NewState != DISABLE)
    {
        TIM2->CCMR3 |= (uint8_t)TIM2_CCMR_OCxPE;
    }
    else
    {
        TIM2->CCMR3 &= (uint8_t)(~TIM2_CCMR_OCxPE);
    }
}

void TIM2_ARRPreloadConfig(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONALSTATE_OK(NewState));

    /* Set or Reset the ARPE Bit */
    if (NewState != DISABLE)
    {
        TIM2->CR1 |= (uint8_t)TIM2_CR1_ARPE;
    }
    else
    {
        TIM2->CR1 &= (uint8_t)(~TIM2_CR1_ARPE);
    }
}

void TIM2_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONALSTATE_OK(NewState));

    /* set or Reset the CEN Bit */
    if (NewState != DISABLE)
    {
        TIM2->CR1 |= (uint8_t)TIM2_CR1_CEN;
    }
    else
    {
        TIM2->CR1 &= (uint8_t)(~TIM2_CR1_CEN);
    }
}



static void TIM2_Config(void)
{
	//TIM2_TimeBaseInit
  /* Time base configuration */
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, 199);		//999->2KHz, 199 -> 10KHz

  /* PWM1 Mode configuration: Channel1 */ 
  TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE,CCR1_Val, TIM2_OCPOLARITY_HIGH);
  TIM2_OC1PreloadConfig(ENABLE);

  /* PWM1 Mode configuration: Channel2 */ 
  //TIM2_OC2Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE,CCR2_Val, TIM2_OCPOLARITY_HIGH);
  //TIM2_OC2PreloadConfig(ENABLE);

  /* PWM1 Mode configuration: Channel3 */         
  TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE,CCR3_Val, TIM2_OCPOLARITY_HIGH);
  TIM2_OC3PreloadConfig(ENABLE);

  TIM2_ARRPreloadConfig(ENABLE);

  /* TIM2 enable counter */
  TIM2_Cmd(ENABLE);
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval
  * None
  */
void assert_failed(u8* file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

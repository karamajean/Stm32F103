#include "stm32f10x.h"
#include "stdio.h"
#include "stdlib.h"

#define VP_ON         GPIO_SetBits( GPIOC, GPIO_Pin_9 );
#define VP_OFF        GPIO_ResetBits( GPIOC, GPIO_Pin_9 );

volatile uint16_t timeCount = 0;
volatile uint16_t tempPluseCount = 0;

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(USART2, (uint8_t) ch);

    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}

    return ch;
}


void delay_1ms(unsigned int nCount)
{
    int i,j;
    for(i=0; i<nCount; i++)
    {
        for(j=0; j<0x2000; j++); //1ms
    }
}


void USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /*GPIOA Periph clock enable*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /*USART2 Periph clock enable*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure USART2 Tx (PA.2) as alternate function open-drain */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USART2 */
    USART_Init(USART2, &USART_InitStructure);

    /* Enable USART2 Receive and Transmit interrupts */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

    /* Enable the USART2 */
    USART_Cmd(USART2, ENABLE);
}


//VN Input PB0
void VN_EXTI0_PB0_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure PA.00 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Connect EXTI0 Line to PA.00 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);

    /* Configure EXTI0 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0f;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void LMT01_DEV_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    //VP PC9 Output
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
    GPIO_ResetBits( GPIOC, GPIO_Pin_9 );
}

int8_t LMT01_Get_Temp(void)
{
    int8_t temp_value = 0;

    temp_value = (( tempPluseCount / 16 ) - 50 );

    return temp_value;
}


void temp_show(void)
{
    int8_t temp;

    tempPluseCount = 0;
    VP_ON
    delay_1ms(94);
    VP_OFF
    temp = LMT01_Get_Temp();
    tempPluseCount = 0;

    printf("[TS]%d \n\r",temp);
}


int main(void)
{
   
    USART_Config();

    LMT01_DEV_Init();       
    VN_EXTI0_PB0_Config();

    printf(" Lmt01 Example \n");

    //lmt01 temperature test
    while( 1  )
    {
        temp_show();

    }


}

# STM32 AT-Command LL Library

* http://www.github.com/NimaLTD   
* https://www.instagram.com/github.nimaltd/   
* https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw   

This is the AT-Command STM32 LL Library  

How to use this Library:
* Select "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" on project settings.   
* Enable USART and enable interrupt.   
* Enable TX DMA is optional.
* Enable FREERTOS is optional.
* Add atc_callback() into USART IRQ callback.
* If Enabled TX DMA, Add atc_callback_txDMA() into DMA callback.
* Include Header and source into your project.   
* Config "ATCConfig.h".   
* Call atc_init( .. .. .. ).   

```
#include "atc.h"

char answerString[128];
uint8_t foundAnswer; 
int main()
{
  atc_init(ATC_Element_0, USART3, 256, 50, "WIFI", DMA1, LL_DMA_STREAM_3);
  foundAnswer = atc_sendAtCommand(ATC_Element_0, "AT\r\n", 3000, answerString, 128, 2, "OK\r\n", "ERROR\r\n");
  
  while(1)
  {
    atc_process();
  }
}
```
in stm32f4xx_it.c
```
void USART3_IRQHandler(void)
{
  atc_callback(ATC_Element_0);
}

// if use tx dma
void DMA1_Stream3_IRQHandler(void)
{
  atc_callback_txDMA(ATC_Element_0);
}


```









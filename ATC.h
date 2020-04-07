#ifndef _ATC_H_
#define _ATC_H_


/*
  Author:     Nima Askari
  WebSite:    http://www.github.com/NimaLTD
  Instagram:  http://instagram.com/github.NimaLTD
  Youtube:    https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw
  
  Version:    2.0.2
  
  Reversion History:
  
  (2.0.0)
  fix bugs.
  
  (2.0.0)
  First release.
*/ 

#include "main.h"
#include "atcconfig.h"
#include <stdbool.h>
#include <stdarg.h>

#ifdef  __cplusplus
extern "C"
{
#endif
  
#if ATC_CMSIS_OS == 0
#define atc_delay(x)  HAL_Delay(x)
#else
#include "cmsis_os.h"
#define atc_delay(x)  osDelay(x)
#endif

typedef struct
{
  USART_TypeDef*  usart;
  char            name[8];
  uint32_t        rxTime;
  uint8_t*        rxBuffer;
  uint8_t         rxTmp;
  uint16_t        rxIndex;
  uint16_t        rxSize;  
  uint16_t        rxTimeout; 
  uint8_t         txBusy; 
  int8_t          foundAnswer;  
  uint16_t        foundAnswerSize; 
  char*           foundAnswerString;
  char*           answerSearchingString[ATC_MAX_ANSWER_SEARCHING_STRING];
  char*           alwaysSearchingString[ATC_MAX_ALWAYS_SEARCHING_STRING]; 
  #if (ATC_TXDMA == 1)
  DMA_TypeDef*    txDMA;
  uint32_t        txDMA_Stream;
  uint8_t         txDMAdone;
  #endif  
  
}ATC_t;

typedef enum
{
  ATC_Element_Error = -1,
  ATC_Element_0,
  ATC_Element_1,
  ATC_Element_2,
  ATC_Element_3,
  ATC_Element_4,  
  
}ATC_Element_t;

//###################################################################################
void            atc_process(void);  // put in main loop. 
void            atc_callback(ATC_Element_t ATC_Element);  //  put in usart callback
#if (ATC_TXDMA == 1)
void            atc_callback_txDMA(ATC_Element_t ATC_Element);  //  put in usart txDMA callback
bool            atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name, DMA_TypeDef *DMA, uint32_t LL_DMA_STREAM_);
#else
bool            atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name);
#endif
uint8_t         atc_sendAtCommand(ATC_Element_t ATC_Element, char *atCommand, uint32_t wait_ms, char *answerString, uint16_t answerSize, uint8_t searchingItems,...);
bool            atc_addAutoSearchString(ATC_Element_t ATC_Element, char *str);

uint8_t         atc_sendString(ATC_Element_t ATC_Element, char *str, uint32_t timeout);
uint8_t         atc_sendData(ATC_Element_t ATC_Element, uint8_t *data, uint16_t size, uint32_t timeout);
ATC_Element_t   atc_getElementByName(char *name);
uint16_t        atc_getSize(ATC_Element_t ATC_Element);

void            atc_userFoundString(ATC_Element_t ATC_Element, const char* foundStr, char* startPtr); 
//###################################################################################

#ifdef  __cplusplus
}
#endif

#endif

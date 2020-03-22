
#include "ATC.h"
#include <string.h>
#include <stdlib.h>

uint8_t     atc_cnt;
ATC_t       atc_elements[ATC_MAX_ELEMENTS];   

//#####################################################################################################
__weak void  atc_userFoundString(ATC_Element_t ATC_Element, const char* foundStr, char* startPtr)
{
     
}  
//#####################################################################################################
void atc_callback(ATC_Element_t ATC_Element)
{
  if(LL_USART_IsActiveFlag_RXNE(atc_elements[ATC_Element].usart) && LL_USART_IsEnabledIT_RXNE(atc_elements[ATC_Element].usart))
  {
    if(atc_elements[ATC_Element].rxIndex < atc_elements[ATC_Element].rxSize - 1)
    {
      atc_elements[ATC_Element].rxBuffer[atc_elements[ATC_Element].rxIndex] = LL_USART_ReceiveData8(atc_elements[ATC_Element].usart);      
      atc_elements[ATC_Element].rxIndex++;
    }
    else
      LL_USART_ReceiveData8(atc_elements[ATC_Element].usart);      
  }
  atc_elements[ATC_Element].rxTime = HAL_GetTick();
}
//#####################################################################################################
#if (ATC_TXDMA == 1)
void  atc_callback_txDMA(ATC_Element_t ATC_Element)
{
  switch(atc_elements[ATC_Element].txDMA_Stream)
  {
    case LL_DMA_STREAM_0:
      if(LL_DMA_IsActiveFlag_TC0(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC0(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
    case LL_DMA_STREAM_1:
      if(LL_DMA_IsActiveFlag_TC1(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC1(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
    case LL_DMA_STREAM_2:
      if(LL_DMA_IsActiveFlag_TC2(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC2(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;  
    case LL_DMA_STREAM_3:
      if(LL_DMA_IsActiveFlag_TC3(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC3(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
    case LL_DMA_STREAM_4:
      if(LL_DMA_IsActiveFlag_TC4(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC4(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
    case LL_DMA_STREAM_5:
      if(LL_DMA_IsActiveFlag_TC5(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC5(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
    case LL_DMA_STREAM_6:
      if(LL_DMA_IsActiveFlag_TC6(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC6(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
    case LL_DMA_STREAM_7:
      if(LL_DMA_IsActiveFlag_TC7(atc_elements[ATC_Element].txDMA))
      {
        LL_DMA_ClearFlag_TC7(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }      
    break;
  }
}
#endif
//#####################################################################################################
void atc_process(void)
{
  for(uint8_t el = 0; el < atc_cnt ; el++)
  {    
    if((atc_elements[el].rxIndex > 0) && (HAL_GetTick() - atc_elements[el].rxTime > atc_elements[el].rxTimeout))
    {
      //  +++ answer AtCommand
      for(uint8_t ans = 0 ; ans < ATC_MAX_ANSWER_SEARCHING_STRING ; ans++)
      {
        if(atc_elements[el].answerSearchingString[ans] == NULL)
          break;
        char *ansStr = strstr((char*)atc_elements[el].rxBuffer, atc_elements[el].answerSearchingString[ans]);         
        if( ansStr != NULL)
        {
          if(atc_elements[el].foundAnswerString != NULL)
          {
            #if (ATC_CMSIS_OS == 0)            
            atc_elements[el].foundAnswerString = malloc(strlen(ansStr));
            #endif
            #if (ATC_CMSIS_OS == 1)
            atc_elements[el].foundAnswerString = pvPortMalloc(strlen(ansStr));
            #endif
            if(atc_elements[el].foundAnswerString != NULL)
              strcpy(atc_elements[el].foundAnswerString, ansStr);
          }
          atc_elements[el].foundAnswer = ans; 
          break;          
        }        
      }
      //  --- answer AtCommand      
      //  +++ auto searching  
      for(uint8_t au = 0 ; au < ATC_MAX_ALWAYS_SEARCHING_STRING ; au++)
      {
        if(atc_elements[el].alwaysSearchingString[au] == NULL)
          break;
        char *autoStr = strstr((char*)atc_elements[el].rxBuffer, atc_elements[el].alwaysSearchingString[au]);
        if(autoStr != NULL)
        {
          atc_userFoundString((ATC_Element_t)el, atc_elements[el].alwaysSearchingString[au], autoStr);
        }
      }
      //  --- auto searching
      atc_elements[el].rxIndex = 0;
      memset(atc_elements[el].rxBuffer, 0, atc_elements[el].rxSize);
    }
  }  
}
//#####################################################################################################
uint8_t atc_sendString(ATC_Element_t ATC_Element, char *str, uint32_t timeout)
{
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return 0;
  while(atc_elements[ATC_Element].txBusy == 1)
    atc_delay(1);
  atc_elements[ATC_Element].txBusy = 1;
  uint32_t startTime = HAL_GetTick();
  #if (ATC_TXDMA == 0)
  for(uint16_t i = 0 ; i<strlen(str) ; i++)
  {
    while (!LL_USART_IsActiveFlag_TXE(atc_elements[ATC_Element].usart))
    {
      if(HAL_GetTick() - startTime > timeout)
      {
        atc_elements[ATC_Element].txBusy = 0;
        return 0;
      }   
    }
    LL_USART_TransmitData8(atc_elements[ATC_Element].usart, str[i]);   
  }
  while (!LL_USART_IsActiveFlag_TC(atc_elements[ATC_Element].usart))
  {
    if(HAL_GetTick() - startTime > timeout)
    {
      atc_elements[ATC_Element].txBusy = 0;
      return 0;
    }   
  }
  #endif
  #if (ATC_TXDMA == 1)
  atc_elements[ATC_Element].txDMAdone = 0;
  LL_DMA_ConfigAddresses(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream,\
    (uint32_t)str, LL_USART_DMA_GetRegAddr(atc_elements[ATC_Element].usart),\
    LL_DMA_GetDataTransferDirection(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream));
  LL_DMA_SetDataLength(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream, strlen(str));
  LL_DMA_EnableStream(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream);
  while(atc_elements[ATC_Element].txDMAdone == 0)
  {
    if(HAL_GetTick() - startTime > timeout)
    {
      LL_DMA_DisableStream(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream);
      atc_elements[ATC_Element].txBusy = 0;
      return 0;
    }
    atc_delay(1);
  }
  #endif
  atc_elements[ATC_Element].txBusy = 0;
  return 1;
}
//#####################################################################################################
uint8_t atc_sendData(ATC_Element_t ATC_Element, uint8_t *data, uint16_t size, uint32_t timeout)
{
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return 0;
  while(atc_elements[ATC_Element].txBusy == 1)
    atc_delay(1);
  atc_elements[ATC_Element].txBusy = 1;
  #if (ATC_TXDMA == 0)
  for (uint16_t i = 0; i<size; i++)
  {
    while (!LL_USART_IsActiveFlag_TXE(atc_elements[ATC_Element].usart))
    {
      if(HAL_GetTick() - startTime > timeout)
      {
        atc_elements[ATC_Element].txBusy = 0;
        return 0;
      }   
    }
    LL_USART_TransmitData8(atc_elements[ATC_Element].usart, data[i]);
  }
  while (!LL_USART_IsActiveFlag_TC(atc_elements[ATC_Element].usart))
  {
    if(HAL_GetTick() - startTime > timeout)
    {
      atc_elements[ATC_Element].txBusy = 0;
      return 0;
    }  
  }
  #endif
  #if (ATC_TXDMA == 1)
  atc_elements[ATC_Element].txDMAdone = 0;
  LL_DMA_ConfigAddresses(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream,\
    (uint32_t)data, LL_USART_DMA_GetRegAddr(atc_elements[ATC_Element].usart),\
    LL_DMA_GetDataTransferDirection(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream));
  LL_DMA_SetDataLength(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream, size);
  while(atc_elements[ATC_Element].txDMAdone == 0)
    atc_delay(1);
  #endif
  atc_elements[ATC_Element].txBusy = 0;
  return 1;
}
//#####################################################################################################
uint8_t atc_sendAtCommand(ATC_Element_t ATC_Element, char *atCommand, uint32_t wait_ms,char *answerString,uint16_t answerSize, uint8_t searchingItems,...)
{
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return 0;
  atc_elements[ATC_Element].foundAnswerString = answerString;
  va_list tag;
  va_start (tag,searchingItems);
  for(uint8_t i=0; i<searchingItems ; i++)
  {
    char *str = va_arg (tag, char*);
    #if (ATC_CMSIS_OS == 0)
    atc_elements[ATC_Element].answerSearchingString[i] = malloc(strlen(str));
    #endif
    #if (ATC_CMSIS_OS == 1)
    atc_elements[ATC_Element].answerSearchingString[i] = pvPortMalloc(strlen(str));
    #endif
    if(atc_elements[ATC_Element].answerSearchingString[i] != NULL)
      strcpy(atc_elements[ATC_Element].answerSearchingString[i], str);
  }
  va_end (tag);
  atc_elements[ATC_Element].foundAnswer = -1;
  uint8_t retValue = 0;
  uint32_t startTime = HAL_GetTick();
  if( atc_sendString(ATC_Element, atCommand, 200) == 0)
    goto SEND_FAILD;      
  while(1)
  {
    atc_delay(1);
    atc_process();  
    if((atc_elements[ATC_Element].foundAnswer != -1) && (answerString != NULL))
      strncpy(answerString, atc_elements[ATC_Element].foundAnswerString, answerSize);
    if(atc_elements[ATC_Element].foundAnswer >= 0)
    {
      retValue = atc_elements[ATC_Element].foundAnswer + 1;
      break;
    }
    if(HAL_GetTick() - startTime > wait_ms)
    {
      if(searchingItems == 0)
        retValue = 1;  
      break;        
    }
  }
  SEND_FAILD:
  #if (ATC_CMSIS_OS == 0)
  for(uint8_t i = 0 ; i < ATC_MAX_ANSWER_SEARCHING_STRING ; i++)
  {
     free(atc_elements[ATC_Element].answerSearchingString[i]);
     atc_elements[ATC_Element].answerSearchingString[i] = NULL;  
  }
  free(atc_elements[ATC_Element].foundAnswerString);
  atc_elements[ATC_Element].foundAnswerString = NULL;
  #endif
  #if (ATC_CMSIS_OS == 1)
  for(uint8_t i = 0 ; i < ATC_MAX_ANSWER_SEARCHING_STRING ; i++)
  {
     vPortFree(atc_elements[ATC_Element].answerSearchingString[i]);
     atc_elements[ATC_Element].answerSearchingString[i] = NULL;
  }
  vPortFree(atc_elements[ATC_Element].foundAnswerString);
  atc_elements[ATC_Element].foundAnswerString = NULL;   
  #endif  
  return retValue;
}  
//#####################################################################################################
ATC_Element_t atc_getElementByName(char *name)
{
  for(uint8_t i = 0 ; i < ATC_MAX_ELEMENTS ; i++)
  {
    if(strcmp(atc_elements[i].name, name) == 0)
      return (ATC_Element_t)i;            
  }
  return ATC_Element_Error;  
}
//#####################################################################################################
bool atc_addAutoSearchString(ATC_Element_t ATC_Element, char *str)
{
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return false;
  for(uint8_t s = 0; s < ATC_MAX_ALWAYS_SEARCHING_STRING ; s++)
  {  
    if(atc_elements[ATC_Element].alwaysSearchingString[s] == NULL)
    {
      #if (ATC_CMSIS_OS == 0)
      atc_elements[ATC_Element].alwaysSearchingString[s] = malloc(strlen(str));            
      #endif
      #if (ATC_CMSIS_OS == 1)
      atc_elements[ATC_Element].alwaysSearchingString[s] = pvPortMalloc(strlen(str));            
      #endif
      if(atc_elements[ATC_Element].alwaysSearchingString[s] != NULL)
      {
        strcpy(atc_elements[ATC_Element].alwaysSearchingString[s], str);
        return true;
      }
      else      
        break;
    }   
  }
  return false;
}
//#####################################################################################################
uint16_t atc_getSize(ATC_Element_t ATC_Element)
{
  return sizeof(atc_elements[ATC_Element]);
}
//#####################################################################################################
#if (ATC_TXDMA == 0)
bool atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name)
#endif  
#if (ATC_TXDMA == 1)
bool atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name, DMA_TypeDef *DMA, uint32_t LL_DMA_STREAM_)
#endif
{
  if(ATC_Element != atc_cnt)
    return false;
  memset(&atc_elements[ATC_Element], 0, sizeof(ATC_t));
  atc_elements[ATC_Element].usart = usart;  
  atc_elements[ATC_Element].rxSize = rxSize;
  atc_elements[ATC_Element].rxTimeout = rxTimeout;
  strncpy(atc_elements[ATC_Element].name, name, 8);
  #if (ATC_TXDMA == 1)
  
  atc_elements[ATC_Element].txDMA = DMA;
  atc_elements[ATC_Element].txDMA_Stream = LL_DMA_STREAM_;
  LL_DMA_EnableIT_TC(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream);
  LL_USART_EnableDMAReq_TX(atc_elements[ATC_Element].usart);
  #endif
  #if (ATC_CMSIS_OS == 0)
  atc_elements[ATC_Element].rxBuffer = malloc(rxSize);
  #endif
  #if (ATC_CMSIS_OS == 1)
  atc_elements[ATC_Element].rxBuffer = pvPortMalloc(rxSize);
  #endif
  if(atc_elements[ATC_Element].rxBuffer != NULL)
  {
    atc_cnt++;
    LL_USART_EnableIT_RXNE(atc_elements[ATC_Element].usart);
    return true;
  }
  return false;      
}
//#####################################################################################################

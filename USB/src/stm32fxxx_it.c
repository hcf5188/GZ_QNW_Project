/**
  ******************************************************************************
  * @file    stm32fxxx_it.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides all exceptions handler and peripherals interrupt
  *          service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 
#include "ucos_ii.h"
/* Includes ------------------------------------------------------------------*/
#include "stm32fxxx_it.h"
#include "usb_core.h"
#include "usbd_core.h"
#include "usbd_hid_core.h"
//#include "lcd_log.h"
#include "usb_conf.h"
#include "core_cm3.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define CURSOR_STEP     10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t remote_wakeup =0;
/* Private function prototypes -----------------------------------------------*/
extern USB_OTG_CORE_HANDLE           USB_OTG_dev;

extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);


/******************************************************************************/
/*             Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/




/**
* @brief  This function handles Memory Manage exception.
* @param  None
* @retval None
*/

/**
* @brief  This function handles Bus Fault exception.
* @param  None
* @retval None
*/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}




/**
* @brief  This function handles SVCall exception.
* @param  None
* @retval None
*/
void SVC_Handler(void)
{
}

/**
* @brief  This function handles Debug Monitor exception.
* @param  None
* @retval None
*/
void DebugMon_Handler(void)
{
}



/**
* @brief  This function handles SysTick Handler.
* @param  None
* @retval None
*/
uint8_t tim;
uint8_t usbsdate[64]={0x25,0x36,0x45,0x94,0x45};
//void SysTick_Handler(void)
//{
//  
//  uint8_t *buf;

//  tim++;   // usb_usr.c中USBD_USR_Init设定20ms进入一次

//  if(tim>=50)
//  {
//  	  tim=0;
//	  usbsdate[0]++;
//	  buf = usbsdate;//USBD_HID_GetPos();	// 获取要向PC发送的数据
//	  if((buf[1] != 0) ||(buf[2] != 0))
//	  {
//	    USBD_HID_SendReport (&USB_OTG_dev, 
//	                         buf,
//	                         HID_INOUT_BYTES);
//	  } 
//  }
//  

//}



/**
* @brief  USB Device woke up KEY
* @param  None
* @retval None
*/ 
void EXTI9_5_IRQHandler(void)
{
	OSIntEnter();							//进入中断	 
  if (EXTI_GetITStatus(EXTI_Line9) != RESET)
  {
    
    if (USB_OTG_dev.dev.DevRemoteWakeup)
    {
      SystemInit();
      
      USB_OTG_ActiveRemoteWakeup(&USB_OTG_dev);
      USB_OTG_dev.dev.device_status = USB_OTG_dev.dev.device_old_status;
      remote_wakeup =1;
      //LCD_UsrLog("> USB Device woke up.\n");
    }
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(EXTI_Line9);
  }
  OSIntExit();   
}

/**
* @brief  This function handles EXTI15_10_IRQ Handler.
* @param  None
* @retval None
*/
void OTG_FS_WKUP_IRQHandler(void)
{
OSIntEnter();
  if(USB_OTG_dev.cfg.low_power)
  {
    /* Reset SLEEPDEEP and SLEEPONEXIT bits */
//    SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
       
    SystemInit();
  
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
  OSIntExit(); 
}


/**
* @brief  This function handles OTG_HS Handler.
* @param  None
* @retval None
*/
void OTG_FS_IRQHandler(void)
{
	OSIntEnter();
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
  OSIntExit(); 
}



/******************************************************************************/
/*                 STM32Fxxx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32fxxx.s).                                               */
/******************************************************************************/

/**
* @brief  This function handles PPP interrupt request.
* @param  None
* @retval None
*/
/*void PPP_IRQHandler(void)
{
}*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

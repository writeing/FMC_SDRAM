/**
  ******************************************************************************
  * @file    FMC/FMC_SDRAM/Inc/main.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    25-June-2015
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery.h"
#include "stm32f7xx_lp_modes.h"

#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0130

#define UART_BUFFER_SIZE 	(1024*2)

/* Exported types ------------------------------------------------------------*/
//typedef enum {PASSED = 0, FAILED = !PASSED} TestStatus_t;
/* Exported constants --------------------------------------------------------*/
#define SDRAM_BANK_ADDR                 ((uint32_t)0xC0000000)

/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8  */
#define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_16

#define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2
/* #define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3 */

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 32 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08008000) /* Base address of Sector 1, 32 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08010000) /* Base address of Sector 2, 32 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08018000) /* Base address of Sector 3, 32 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08020000) /* Base address of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08040000) /* Base address of Sector 5, 256 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08070000) /* Base address of Sector 6, 256 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x080A0000) /* Base address of Sector 7, 256 Kbytes */

/* Definition for UART7 clock resources */
//ublox
#define UART7_CLK_ENABLE()              __HAL_RCC_UART7_CLK_ENABLE()
#define DMA7_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define UART7_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()
#define UART7_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()

#define UART7_FORCE_RESET()             __HAL_RCC_UART7_FORCE_RESET()
#define UART7_RELEASE_RESET()           __HAL_RCC_UART7_RELEASE_RESET()

/* Definition for UART7 Pins */
#define UART7_TX_PIN                    GPIO_PIN_7
#define UART7_TX_GPIO_PORT              GPIOF
#define UART7_TX_AF                     GPIO_AF8_UART7
#define UART7_RX_PIN                    GPIO_PIN_6
#define UART7_RX_GPIO_PORT              GPIOF
#define UART7_RX_AF                     GPIO_AF8_UART7

/* Definition for UART7's DMA */
#define UART7_TX_DMA_CHANNEL             DMA_CHANNEL_5
#define UART7_RX_DMA_CHANNEL             DMA_CHANNEL_5

#define UART7_TX_DMA_STREAM              DMA1_Stream1
#define UART7_RX_DMA_STREAM              DMA1_Stream3

/* Definition for UART7's NVIC */
#define UART7_DMA_TX_IRQn                DMA1_Stream1_IRQn
#define UART7_DMA_RX_IRQn                DMA1_Stream3_IRQn
#define UART7_DMA_TX_IRQHandler          DMA1_Stream1_IRQHandler
#define UART7_DMA_RX_IRQHandler          DMA1_Stream3_IRQHandler


/* Definition for USARTx clock resources */
// 对外串口2
#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART1
#define USARTx_RX_PIN                    GPIO_PIN_10
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART1

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL             DMA_CHANNEL_4
#define USARTx_RX_DMA_CHANNEL             DMA_CHANNEL_4

#define USARTx_TX_DMA_STREAM              DMA2_Stream7
#define USARTx_RX_DMA_STREAM              DMA2_Stream5

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn                DMA2_Stream7_IRQn
#define USARTx_DMA_RX_IRQn                DMA2_Stream5_IRQn
#define USARTx_DMA_TX_IRQHandler          DMA2_Stream7_IRQHandler
#define USARTx_DMA_RX_IRQHandler          DMA2_Stream5_IRQHandler

/* Definition for USARTx's NVIC IRQ and IRQ Handlers */
#define USARTx_IRQn                      USART1_IRQn
#define USARTx_IRQHandler                USART1_IRQHandler

/* Definition for USART2 clock resources */
//#define USARTx                           USART2
//外部USB
#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()
#define DMA2_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define USART2_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USART2_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USART2 Pins */
#define USART2_TX_PIN                    GPIO_PIN_5
#define USART2_TX_GPIO_PORT              GPIOD
#define USART2_TX_AF                     GPIO_AF7_USART2
#define USART2_RX_PIN                    GPIO_PIN_6
#define USART2_RX_GPIO_PORT              GPIOD
#define USART2_RX_AF                     GPIO_AF7_USART2

/* Definition for USART2's DMA */
#define USART2_TX_DMA_CHANNEL             DMA_CHANNEL_4
#define USART2_RX_DMA_CHANNEL             DMA_CHANNEL_4

#define USART2_TX_DMA_STREAM              DMA1_Stream6
#define USART2_RX_DMA_STREAM              DMA1_Stream5

/* Definition for USART2's NVIC */
#define USART2_DMA_TX_IRQn                DMA1_Stream6_IRQn
#define USART2_DMA_RX_IRQn                DMA1_Stream5_IRQn
#define USART2_DMA_TX_IRQHandler          DMA1_Stream6_IRQHandler
#define USART2_DMA_RX_IRQHandler          DMA1_Stream5_IRQHandler



/* Definition for USART3 clock resources */
//#define USARTx                           USART3
// UBLOX 串口1
#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE()
#define DMA3_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define USART3_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define USART3_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()

/* Definition for USART3 Pins */
#define USART3_TX_PIN                    GPIO_PIN_10
#define USART3_TX_GPIO_PORT              GPIOC
#define USART3_TX_AF                     GPIO_AF7_USART3
#define USART3_RX_PIN                    GPIO_PIN_11
#define USART3_RX_GPIO_PORT              GPIOC
#define USART3_RX_AF                     GPIO_AF7_USART3

/* Definition for USART3's DMA */
#define USART3_TX_DMA_CHANNEL             DMA_CHANNEL_4
#define USART3_RX_DMA_CHANNEL             DMA_CHANNEL_4

#define USART3_TX_DMA_STREAM              DMA1_Stream3
#define USART3_RX_DMA_STREAM              DMA1_Stream1

/* Definition for USART3's NVIC */
#define USART3_DMA_TX_IRQn                DMA1_Stream3_IRQn
#define USART3_DMA_RX_IRQn                DMA1_Stream1_IRQn
#define USART3_DMA_TX_IRQHandler          DMA1_Stream3_IRQHandler
#define USART3_DMA_RX_IRQHandler          DMA1_Stream1_IRQHandler

/* Definition for USART6 clock resources */
//#define USARTx                           USART6
#define USART6_CLK_ENABLE()              __HAL_RCC_USART6_CLK_ENABLE()
#define DMA6_CLK_ENABLE()                __HAL_RCC_DMA2_CLK_ENABLE()
#define USART6_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USART6_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define USART6_FORCE_RESET()             __HAL_RCC_USART6_FORCE_RESET()
#define USART6_RELEASE_RESET()           __HAL_RCC_USART6_RELEASE_RESET()

/* Definition for USART6 Pins */
#define USART6_TX_PIN                    GPIO_PIN_6
#define USART6_TX_GPIO_PORT              GPIOC
#define USART6_TX_AF                     GPIO_AF8_USART6
#define USART6_RX_PIN                    GPIO_PIN_7
#define USART6_RX_GPIO_PORT              GPIOC
#define USART6_RX_AF                     GPIO_AF8_USART6


/* Definition for USART3's DMA */
#define USART6_TX_DMA_CHANNEL             DMA_CHANNEL_5
#define USART6_RX_DMA_CHANNEL             DMA_CHANNEL_5

#define USART6_TX_DMA_STREAM              DMA2_Stream6
#define USART6_RX_DMA_STREAM              DMA2_Stream2

/* Definition for USART3's NVIC */
#define USART6_DMA_TX_IRQn                DMA2_Stream6_IRQn
#define USART6_DMA_RX_IRQn                DMA2_Stream2_IRQn
#define USART6_DMA_TX_IRQHandler          DMA2_Stream6_IRQHandler
#define USART6_DMA_RX_IRQHandler          DMA2_Stream2_IRQHandler


typedef unsigned char U8;
typedef char          S8;
typedef short S16;
typedef unsigned short U16 ;
typedef unsigned int U32;
typedef int S32;
typedef char                    kal_char;
/* portable wide character for unicode character set */
typedef unsigned short          kal_wchar;

/* portable 8-bit unsigned integer */
typedef unsigned char           kal_uint8;
/* portable 8-bit signed integer */
typedef char             kal_int8;
/* portable 16-bit unsigned integer */
typedef unsigned short int      kal_uint16;
/* portable 16-bit signed integer */
typedef short int        kal_int16;
/* portable 32-bit unsigned integer */
typedef unsigned int            kal_uint32;
/* portable 32-bit signed integer */
typedef int              kal_int32;


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SDRAM_ADDRESS   ((uint32_t) 0xC0000000)
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define RXBUFFERSIZE                      50
/* Size of Trasmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
#define BUFFER_SIZE         ((uint32_t)0x0040000)
#define WRITE_READ_ADDR     ((uint32_t)0x0800)

/* UART handler declared in "main.c" file */
extern UART_HandleTypeDef Uart1Handle;
extern UART_HandleTypeDef Uart7Handle;
extern UART_HandleTypeDef Uart3Handle;
extern UART_HandleTypeDef Uart6Handle;

extern __IO ITStatus UartReady;


 void SystemClock_Config(void);
 void BSP_SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
 void MPU_Config(void);
 void CPU_CACHE_Enable(void);
 void SdramInit(void);
 void CheckBasePosValid(void);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

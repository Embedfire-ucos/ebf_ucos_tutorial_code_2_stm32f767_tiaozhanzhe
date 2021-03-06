/*
  ******************************************************************************
  * @file    bsp.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief   系统初始化相关
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F767 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
*/


/*
*********************************************************************************************************
*                                             包含的文件
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  "os.h"
#include  "bsp.h"
#include "./led/bsp_led.h" 
#include ".\key\bsp_key.h" 
#include "usart/bsp_debug_usart.h"
#include  "stm32f7xx_hal.h"
#include  "stm32f7xx_hal.h"

/*
*********************************************************************************************************
* 函数名 : BSP_Init
* 描述   : 所有的硬件设备都应该放在这个函数里边初始化
* 形参   : 无
* 返回值 : 无
*********************************************************************************************************
*/
void  BSP_Init (void)
{
    BSP_OSTickInit();                  //初始化 OS 时钟源
    LED_GPIO_Config();                 //初始化LED
    DEBUG_USART_Config();              //初始化 USART1
    Key_GPIO_Config();                //初始化按键
}
/*
*********************************************************************************************************                               
* 函数名 : BSP_SystemClkCfg
* 描述   : 系统时钟初始化
* 形参   : 无
* 返回值 : 无
*********************************************************************************************************
*/

void  BSP_SystemClkCfg (void)
{
    RCC_OscInitTypeDef  RCC_OscInit;
    RCC_ClkInitTypeDef  RCC_ClkInit;
    HAL_StatusTypeDef   hal_status;
                                                      
    RCC_OscInit.OscillatorType = RCC_OSCILLATORTYPE_HSE;            
    RCC_OscInit.HSEState       = RCC_HSE_ON;
    RCC_OscInit.HSIState       = RCC_HSI_OFF;
    RCC_OscInit.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInit.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInit.PLL.PLLM       = 25u;                              //外部晶振频率 HSE freq     = 25MHz
    RCC_OscInit.PLL.PLLN       = 432u;                             //倍频时钟频率 VCO out-freq = HSE * (PLLN / PLLM)     = 432MHz
    RCC_OscInit.PLL.PLLP       = RCC_PLLP_DIV2;                    //系统时钟频率 PLLCLK       = (VCO out-freq) / PLLP   = 216MHz
    RCC_OscInit.PLL.PLLQ       = 9;                                //外设时钟频率 PLL_Q out freq = (VCO out-freq) / PLLQ = 48MHz

    hal_status = HAL_RCC_OscConfig(&RCC_OscInit);
    if (hal_status != HAL_OK) {
        while (DEF_TRUE) {                                         //如果出错则停止
            ;
        }
    }

    hal_status = HAL_PWREx_EnableOverDrive();                      //等待时钟频率稳定为 216 Mhz
    if (hal_status != HAL_OK) {
        while (DEF_TRUE) {                                         //如果出错则停止
            ;
        }
    }

    RCC_ClkInit.ClockType      = RCC_CLOCKTYPE_SYSCLK |
                                 RCC_CLOCKTYPE_HCLK   |
                                 RCC_CLOCKTYPE_PCLK1  |
                                 RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInit.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInit.AHBCLKDivider  = RCC_SYSCLK_DIV1;                   //AHB时钟频率  HCLK= AHBCLK    = PLLCLK / AHBPRES(1) = 216MHz
    RCC_ClkInit.APB1CLKDivider = RCC_HCLK_DIV4;                     //APB1外设总线时钟频率 APB1CLK = AHBCLK  / APB1DIV(4)=  54MHz
    RCC_ClkInit.APB2CLKDivider = RCC_HCLK_DIV2;                     //APB2外设总线时钟频率 APB2CLK = AHBCLK  / APB2DIV(2)= 108MHz

    hal_status = HAL_RCC_ClockConfig(&RCC_ClkInit, FLASH_LATENCY_7);
    if (hal_status != HAL_OK) {
        while (DEF_TRUE) {                                          //如果出错则停止
            ;
        }
    }
}


/*
*********************************************************************************************************                               
* 函数名 : BSP_ClkFreqGet
* 描述   : 这个函数用来检索系统时钟频率
* 形参   : clk_id    系统时钟标识符
*                    BSP_CLK_ID_SYSCLK     系统时钟频率。
*                    BSP_CLK_ID_HCLK       CPU时钟频率。
*                    BSP_CLK_ID_PCLK1      APB1总线时钟频率。
*                    BSP_CLK_ID_PCLK2      APB2总线时钟频率。
* 返回值 : 无
*********************************************************************************************************
*/
CPU_INT32U  BSP_ClkFreqGet (BSP_CLK_ID  clk_id)
{
    CPU_INT32U  clk_freq;


    switch (clk_id) {
        case BSP_CLK_ID_SYSCLK:
             clk_freq = HAL_RCC_GetSysClockFreq();
             break;


        case BSP_CLK_ID_HCLK:
             clk_freq = HAL_RCC_GetHCLKFreq();
             break;


        case BSP_CLK_ID_PCLK1:
             clk_freq = HAL_RCC_GetPCLK1Freq();
             break;


        case BSP_CLK_ID_PCLK2:
             clk_freq = HAL_RCC_GetPCLK2Freq();
             break;


        default:
             clk_freq = 1u;                                     //没有有效时钟频率
             break;
    }

    return (clk_freq);
}
/*
*********************************************************************************************************                               
* 函数名 : BSP_OSTickInit
* 描述   : 初始化 OS 嘀嗒时钟中断
* 形参   : 无
* 返回值 : 无
*********************************************************************************************************
*/
void  BSP_OSTickInit (void)
{
    CPU_INT32U  cpu_clk_freq;


    cpu_clk_freq = BSP_ClkFreqGet(BSP_CLK_ID_HCLK);             //获取CPU时钟，时间戳是以该时钟计数

    OS_CPU_SysTickInitFreq(cpu_clk_freq);                       //初始化uC/OS 周期时钟源 (SysTick)
}

/*
*********************************************************************************************************                               
* 函数名 : HAL_InitTick
* 描述   : 覆盖STM32F7xx HAL 库中的HAL_InitTick函数，因为Micrium实时系统有自己的Systick 初始化，
*          必须在多任务启动后才初始化滴答时钟
* 形参   : TickPriority     滴答中断优先级
* 返回值 : 无
*********************************************************************************************************
*/
HAL_StatusTypeDef  HAL_InitTick (uint32_t  TickPriority)
{
    HAL_NVIC_SetPriorityGrouping(4);

    return (HAL_OK);
}

/*
*********************************************************************************************************                               
* 函数名 : HAL_GetTick
* 描述   : 覆盖STM32F7xx HAL HAL_GetTick函数，因为Micrium实时系统有自己的滴答计时器的值
* 形参   : 无
* 返回值 : 滴答计时器的值
* 注意   ：请确保滴答时钟任务比应用程序启动任务具有更高的优先级
*********************************************************************************************************
*/
uint32_t  HAL_GetTick(void)
{
    CPU_INT32U  os_tick_ctr;
#if (OS_VERSION >= 30000u)
    OS_ERR      os_err;


    os_tick_ctr = OSTimeGet(&os_err);
#else
    os_tick_ctr = OSTimeGet();
#endif

    return os_tick_ctr;
}

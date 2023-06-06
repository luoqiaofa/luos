#ifndef __I2C_EE_H
#define	__I2C_EE_H


#include "stm32f10x.h"
#define INT_ABS(x) ((x) >= 0? (x):(0 - (x)))

/**************************I2C参数定义，I2C1或I2C2********************************/
#define             EEPROM_I2Cx                                I2C1
#define             EEPROM_I2C_APBxClock_FUN                   RCC_APB1PeriphClockCmd
#define             EEPROM_I2C_CLK                             RCC_APB1Periph_I2C1
#define             EEPROM_I2C_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define             EEPROM_I2C_GPIO_CLK                        RCC_APB2Periph_GPIOB
#define             EEPROM_I2C_SCL_PORT                        GPIOB
#define             EEPROM_I2C_SCL_PIN                         GPIO_Pin_6
#define             EEPROM_I2C_SDA_PORT                        GPIOB
#define             EEPROM_I2C_SDA_PIN                         GPIO_Pin_7


/*等待超时时间*/
#define I2CT_FLAG_TIMEOUT         ((uint32_t)0x8000)
#define I2CT_LONG_TIMEOUT         ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))


/*信息输出*/
#define EEPROM_DEBUG_ON         0

#define EEPROM_INFO(fmt,arg...)           sys_printf("<<-EEPROM-INFO->> " fmt "\n",##arg)
#define EEPROM_ERROR(fmt,arg...)          sys_printf("<<-EEPROM-ERROR->> " fmt "\n",##arg)
#define EEPROM_DEBUG(fmt,arg...)          do{\
                                          if(EEPROM_DEBUG_ON)\
                                          sys_printf("<<-EEPROM-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)


/*
 * AT24C02 2kb = 2048bit = 2048/8 B = 256 B
 * 32 pages of 8 bytes each
 *
 * Device Address
 * 1 0 1 0 A2 A1 A0 R/W
 * 1 0 1 0 0  0  0  0 = 0XA0
 * 1 0 1 0 0  0  0  1 = 0XA1
 */

/* EEPROM Addresses defines */
#define EEPROM_Block0_ADDRESS 0xA0   /* E2 = 0 */
//#define EEPROM_Block1_ADDRESS 0xA2 /* E2 = 0 */
//#define EEPROM_Block2_ADDRESS 0xA4 /* E2 = 0 */
//#define EEPROM_Block3_ADDRESS 0xA6 /* E2 = 0 */

struct i2c_msg {
        uint16_t addr;     /* slave address                        */
        uint16_t flags;
#define I2C_M_TEN               0x0010  /* this is a ten bit chip address */
#define I2C_M_RD                0x0001  /* read data, from slave to master */
#define I2C_M_STOP              0x8000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART           0x4000  /* if I2C_FUNC_NOSTART */
#define I2C_M_REV_DIR_ADDR      0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK        0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK         0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN          0x0400  /* length will be first received byte */
        uint16_t len;              /* msg length                           */
        uint8_t *buf;              /* pointer to msg data                  */
};


/**
  * @brief  ADT75 Temperature Sensor I2C Interface pins
  */
#define ADT75_I2C                         I2C1
#define ADT75_I2C_CLK                     RCC_APB1Periph_I2C1
#define ADT75_I2C_SCL_PIN                 GPIO_Pin_6                  /* PB.06 */
#define ADT75_I2C_SCL_GPIO_PORT           GPIOB                       /* GPIOB */
#define ADT75_I2C_SCL_GPIO_CLK            RCC_APB2Periph_GPIOB
#define ADT75_I2C_SDA_PIN                 GPIO_Pin_7                  /* PB.07 */
#define ADT75_I2C_SDA_GPIO_PORT           GPIOB                       /* GPIOB */
#define ADT75_I2C_SDA_GPIO_CLK            RCC_APB2Periph_GPIOB
#define ADT75_I2C_SMBUSALERT_PIN          GPIO_Pin_5                  /* PB.05 */
#define ADT75_I2C_SMBUSALERT_GPIO_PORT    GPIOB                       /* GPIOB */
#define ADT75_I2C_SMBUSALERT_GPIO_CLK     RCC_APB2Periph_GPIOB
#define ADT75_I2C_DR                      ((uint32_t)0x40005410)

#define ADT75_DMA_CLK                     RCC_AHBPeriph_DMA1
#define ADT75_DMA_TX_CHANNEL              DMA1_Channel6
#define ADT75_DMA_RX_CHANNEL              DMA1_Channel7
#define ADT75_DMA_TX_TCFLAG               DMA1_FLAG_TC6
#define ADT75_DMA_RX_TCFLAG               DMA1_FLAG_TC7


typedef enum
{
  ADT75_DMA_TX = 0,
  ADT75_DMA_RX = 1
}ADT75_DMADirection_TypeDef;

/**
  * @brief  TSENSOR Status
  */
typedef enum
{
  ADT75_OK = 0,
  ADT75_FAIL
}ADT75_Status_TypDef;

/**
  * @}
  */

/** @defgroup STM32_EVAL_I2C_TSENSOR_Exported_Constants
  * @{
  */

/* Uncomment the following line to use Timeout_User_Callback ADT75_TimeoutUserCallback().
   If This Callback is enabled, it should be implemented by user in main function .
   ADT75_TimeoutUserCallback() function is called whenever a timeout condition
   occure during communication (waiting on an event that doesn't occur, bus
   errors, busy devices ...). */
/* #define USE_TIMEOUT_USER_CALLBACK */

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define ADT75_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define ADT75_LONG_TIMEOUT         ((uint32_t)(10 * ADT75_FLAG_TIMEOUT))


/**
  * @brief  Block Size
  */
#define ADT75_REG_TEMP       0x00  /*!< Temperature Register of ADT75 */
#define ADT75_REG_CONF       0x01  /*!< Configuration Register of ADT75 */
#define ADT75_REG_THYS       0x02  /*!< Temperature Register of ADT75 */
#define ADT75_REG_TOS        0x03  /*!< Over-temp Shutdown threshold Register of ADT75 */
#define I2C_TIMEOUT         ((uint32_t)0x3FFFF) /*!< I2C Time out */
#define ADT75_ADDR           0x90   /*!< ADT75 address */
#define ADT75_I2C_SPEED      100000 /*!< I2C Speed */



/**
  * @}
  */

/** @defgroup STM32_EVAL_I2C_TSENSOR_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32_EVAL_I2C_TSENSOR_Exported_Functions
  * @{
  */
void ADT75_DeInit(void);
void ADT75_Init(void);
ErrorStatus ADT75_GetStatus(void);
int16_t ADT75_ReadTemp(void);
uint16_t ADT75_ReadReg(uint8_t RegName);
uint8_t ADT75_WriteReg(uint8_t RegName, uint16_t RegValue);
uint8_t ADT75_ReadConfReg(void);
uint8_t ADT75_WriteConfReg(uint8_t RegValue);
uint8_t ADT75_ShutDown(FunctionalState NewState);
void ADT75_LowLevel_DeInit(void);
void ADT75_LowLevel_Init(void);

/**
  * @brief  Timeout user callback function. This function is called when a timeout
  *         condition occurs during communication with IO Expander. Only protoype
  *         of this function is decalred in IO Expander driver. Its implementation
  *         may be done into user application. This function may typically stop
  *         current operations and reset the I2C peripheral and IO Expander.
  *         To enable this function use uncomment the define USE_TIMEOUT_USER_CALLBACK
  *         at the top of this file.
  */
#define ADT75_INVALID_TEMP  (-1000)
#define ADT75_TIMEOUT_UserCallback()  ADT75_FAIL


int i2cTransfer(I2C_TypeDef* i2c, struct i2c_msg *msgs,int num);
int i2cAdt75RegRead(I2C_TypeDef* i2c, uint8_t i2c_addr, uint8_t reg, uint16_t *data);


void I2C_EE_Init(void);
int I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite);
int I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr);
int I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite);
int I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead);
void I2C_EE_WaitEepromStandbyState(void);
void BspI2cInit(I2C_TypeDef * i2c);


#endif /* __I2C_EE_H */


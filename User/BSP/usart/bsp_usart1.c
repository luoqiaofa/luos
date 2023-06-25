#include "string.h"
#include "bsp_usart1.h"
#include "coreLib.h"

#define CONSOLE_BUF_DBG 0
// #define USE_RX_SEM
// static FILE __stdin;
// static FILE __stdout;
// static FILE __stderr;
#ifdef USE_RX_SEM 
static SEMAPHORE semConsole;
#endif
/*
 * ===========================================================================
 * ��������: USARTx_Config
 * ��������: console ��������
 * �������: ��
 * �������: ��
 * �� �� ֵ����
 * ����˵����
 * �޸�����        �汾��     �޸���         �޸�����
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    ���Ƿ�         ����
 * ===========================================================================
 */
void USARTx_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    /* config macUSARTx clock */
    macUSART_APBxClock_FUN(macUSART_CLK, ENABLE);
    macUSART_GPIO_APBxClock_FUN(macUSART_GPIO_CLK, ENABLE);

    /* macUSARTx GPIO config */
    /* Configure macUSARTx Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =  macUSART_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(macUSART_TX_PORT, &GPIO_InitStructure);
    /* Configure macUSARTx Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = macUSART_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(macUSART_RX_PORT, &GPIO_InitStructure);

    //�ݳ�ʼ�� NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;  //��ռ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;      //�����ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //IRQ ͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);        //�ж����ȼ���ʼ��

    /* macUSARTx mode config */
    USART_InitStructure.USART_BaudRate = macUSART_BAUD_RATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(macUSARTx, &USART_InitStructure);

    /* Enable macUSARTx receive interrupt */
    USART_ITConfig(macUSARTx, USART_IT_RXNE, ENABLE);

    USART_Cmd(macUSARTx, ENABLE);

#ifdef USE_RX_SEM 
    semCInit(&semConsole, 0, 0);
#endif
}

#define NUM_RX_BUF  1024

static char uart_rx_buf[NUM_RX_BUF];
static volatile int  buf_wr_offset = 0;
static volatile int  buf_rd_offset = 0;
static int  uart_rx_cnt = 0;

void UART_Receive(void)
{
    //ע�⣡����ʹ��if(USART_GetITStatus(macUSARTx, USART_IT_RXNE) != RESET)���ж�
    if (USART_GetFlagStatus(macUSARTx, USART_FLAG_ORE) != RESET)
    {
        USART_ReceiveData(macUSARTx);
    }
    if(USART_GetITStatus(macUSARTx, USART_IT_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        uart_rx_buf[buf_wr_offset] = (char)USART_ReceiveData(macUSARTx);
        buf_wr_offset = (buf_wr_offset + 1) % NUM_RX_BUF;
        uart_rx_cnt++;
        /* Clear the macUSARTx Receive interrupt */
        USART_ClearITPendingBit(macUSARTx, USART_IT_RXNE);
#ifdef USE_RX_SEM 
        semGive(&semConsole);
#endif
    }
}


/*
 * ===========================================================================
 * ��������: USART1_IRQHandler
 * ��������: console�жϴ������
 * �������: ��
 * �������: ��
 * �� �� ֵ����
 * ����˵����
 * �޸�����        �汾��     �޸���         �޸�����
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    ���Ƿ�         ����
 * ===========================================================================
 */
#ifndef USE_RX_SEM
void USART1_IRQHandler(void)                	//����1�жϷ������
{
    UART_Receive();
}
#endif

/// �ض���c�⺯��printf��macUSARTx
int fputc(int ch, FILE *f)
{
    /* ����һ���ֽ����ݵ�macUSARTx */
    while (USART_GetFlagStatus(macUSARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(macUSARTx, (uint8_t) ch);

    /* �ȴ�������� */
    if ('\n' == ch) {
        while (USART_GetFlagStatus(macUSARTx, USART_FLAG_TXE) == RESET);
        USART_SendData(macUSARTx, (uint8_t)'\r');
    }

    return (ch);
}

int putc(int c, FILE *stream)
{
    return fputc(c, stream);
}

int puts(const char *s)
{
    int rc = 0;

    while (*s) {
        fputc(*s, stdout);
        rc++;
        s++;
    }
    return rc;
}

#define rxbuf_empty() (buf_rd_offset == buf_wr_offset)
/// �ض���c�⺯��scanf��macUSARTx
int fgetc(FILE *f)
{
    int ch = -1;

#ifdef USE_RX_SEM
    semTake(&semConsole, WAIT_FOREVER);
#else
    while (rxbuf_empty()) {
        taskDelay(2);
    }
#endif

    ch = uart_rx_buf[buf_rd_offset];
    if (buf_rd_offset < (NUM_RX_BUF - 1)) {
        buf_rd_offset = buf_rd_offset + 1;
    } else {
        buf_rd_offset = 0;
    }

    return ch;
}

int tstc(void)
{
    if (rxbuf_empty()) {
        return 0;
    }
    return 1;
}

int getc(FILE *stream)
{
    return fgetc(stream);
}

int sysHwInit(void)
{
    USARTx_Config();
    return 0;
}

/*********************************************END OF FILE**********************/

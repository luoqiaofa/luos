#include "string.h"
#include "bsp_usart1.h"
#include "coreLib.h"

SEM_ID consoleSemId = NULL;
#define CONSOLE_BUF_DBG 0
// static FILE __stdin;
// static FILE __stdout;
// static FILE __stderr;

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

    consoleSemId = semCCreate(SEM_Q_PRIORITY, 0);
}

#define NUM_RX_BUF  1024

static char uart_rx_buf[NUM_RX_BUF];
static int  buf_wr_offset = 0;
static int  buf_rd_offset = 0;
static int  uart_rx_cnt = 0;

void UART_Receive(void)
{
    //ע�⣡����ʹ��if(USART_GetITStatus(macUSARTx, USART_IT_RXNE) != RESET)���ж�
    if (USART_GetFlagStatus(macUSARTx, USART_FLAG_ORE) != RESET)
    {
        USART_ReceiveData(macUSARTx);
    }
#if 0
    //���ڴ�����һ��ͨѶ�����յ����ݲ�����
    if(bRecieveOK)
    {
        if(USART_GetITStatus(macUSARTx, USART_IT_RXNE) != RESET)
            USART_ClearITPendingBit(macUSARTx, USART_IT_RXNE);
        return;//processing receive data,don't receive again
    }
#endif
    if(USART_GetITStatus(macUSARTx, USART_IT_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        uart_rx_buf[buf_wr_offset] = (char)USART_ReceiveData(macUSARTx);
        buf_wr_offset = (buf_wr_offset + 1) % NUM_RX_BUF;
        uart_rx_cnt++;
         if (uart_rx_cnt > NUM_RX_BUF) {
            // buf overfollow, note main app to deal this error !!!
            // such as sempost to notify task to deal
			buf_rd_offset = (buf_wr_offset + NUM_RX_BUF) % NUM_RX_BUF;
			uart_rx_cnt = NUM_RX_BUF;
        }
        /* Clear the macUSARTx Receive interrupt */
        USART_ClearITPendingBit(macUSARTx, USART_IT_RXNE);
        semGive(consoleSemId);
    }
}

/*
 * ===========================================================================
 * ��������: read
 * ��������: console �������ݶ�ȡ
 * �������: ��
 * �������: ��
 * �� �� ֵ����
 * ����˵����
 * �޸�����        �汾��     �޸���         �޸�����
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    ���Ƿ�         ����
 * ===========================================================================
 */
int read(int fd, char *buf, size_t len)
{
    int i;
    int real_len = 0;

     if (uart_rx_cnt > 0) {
        real_len = (uart_rx_cnt >= len ? len: uart_rx_cnt);
		uart_rx_cnt -= real_len;
        for (i = 0; i < real_len; i++) {
			buf[i] = uart_rx_buf[buf_rd_offset];
			if (buf_rd_offset < (NUM_RX_BUF - 1)) {
				buf_rd_offset = buf_rd_offset + 1;
			} else {
				 buf_rd_offset = 0;
			}
		}
    }

    return real_len;
}

/*
 * ===========================================================================
 * ��������: USART1_IRQHandler
 * ��������: console�жϴ�������
 * �������: ��
 * �������: ��
 * �� �� ֵ����
 * ����˵����
 * �޸�����        �汾��     �޸���         �޸�����
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    ���Ƿ�         ����
 * ===========================================================================
 */
// void USART1_IRQHandler(void)                	//����1�жϷ������
// {
// UART_Receive();
// }


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


/// �ض���c�⺯��scanf��macUSARTx
int fgetc(FILE *f)
{
    int ch = -1;
    int level;

    semTake(consoleSemId, WAIT_FOREVER);
    level = intLock();
    if (uart_rx_cnt > 0) {
        uart_rx_cnt--;
        ch = uart_rx_buf[buf_rd_offset];
        if (buf_rd_offset < (NUM_RX_BUF - 1)) {
            buf_rd_offset = buf_rd_offset + 1;
        } else {
            buf_rd_offset = 0;
        }
    }
    intUnlock(level);

    return ch;
}

int tstc(void)
{
    if (uart_rx_cnt > 0) {
        return 1;
    }
    return 0;
}

int getc(FILE *stream)
{
    return fgetc(stream);
}


#ifndef STDIN_FILENO
#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */
#endif

int _write (int fd, char *pBuffer, int size)
{
	int i;
    switch (fd) {
        case STDOUT_FILENO:
        case STDERR_FILENO:
        break;
        default :
            return -1;
        break;
    }
    for (i = 0; i < size; i++) {
        fputc(pBuffer[i], stdout);
    }
    return size;
}

int _read(int fd, char *ptr, int len)
{
    int rc = 0;

    if (STDIN_FILENO != fd) {
        return -1;
    }
    rc = read(fd, ptr, len);
    return rc;
}

/*********************************************END OF FILE**********************/
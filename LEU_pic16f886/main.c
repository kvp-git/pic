/* Lineside Equipment Unit firmware by KVP in 2026, CC-BY-SA-NC */

#define DEVICE_ADDRESS (42)

// ----------------------------------------------------------------
#define _XTAL_FREQ (8000000)         // 8MHz clock

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT // Internal oscillator, I/O on RA6/RA7
#pragma config WDTE = OFF            // Watchdog Timer disabled
#pragma config PWRTE = ON            // Power-up Timer enabled
#pragma config MCLRE = OFF           // MCLR pin enabled
#pragma config CP = OFF              // Code protection disabled
#pragma config CPD = OFF             // Data code protection disabled
#pragma config BOREN = ON            // Brown-out Reset enabled
#pragma config IESO = OFF            // Internal/External Switchover disabled
#pragma config FCMEN = OFF           // Fail-Safe Clock Monitor disabled
#pragma config LVP = OFF             // Low Voltage Programming disabled

// CONFIG2
#pragma config BOR4V = BOR40V        // Brown-out Reset set to 4.0V

#include <xc.h>
#include <stdint.h>

#define bool uint8_t
#define true (1)
#define false (0)

void UART_Init(uint32_t baud)
{
    // set TX (RC6) and RX (RC7) as inputs for UART control, set RC5 as RS485 transmit enable output
    TRISC6 = 1;
    TRISC7 = 1;
    TRISC5 = 0;
    RC5 = 0;
    // calculate baud rate for high-speed (BRGH=1)
    // formula: SPBRG = ((Fosc / Baud) / 16) - 1 // 16 bit mode may be needed for higher speeds like 115200
    TXSTAbits.BRGH = 1;
    SPBRG = (uint8_t)((_XTAL_FREQ / baud) / 16) - 1;
    // configure Port
    TXSTAbits.SYNC = 0;    // asynchronous mode
    RCSTAbits.SPEN = 1;    // enable serial port pins
    // enable transmitter and receiver
    TXSTAbits.TXEN = 1;    // enable transmission
    RCSTAbits.CREN = 1;    // enable continuous reception
}

void UART_Write(int data, bool txeOn, bool txeOff)
{
    if (txeOn)
        RC5 = 1;
    while (!PIR1bits.TXIF); // wait for buffer to be empty
    TXREG = (uint8_t)data;  // load data to transmit
    while(!TXSTAbits.TRMT); // wait for buffer to be empty
    if (txeOff)
        RC5 = 0;
}

int UART_Read(void)
{
    if (RCSTAbits.OERR)    // handle overrun error
    {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
    if (!PIR1bits.RCIF)    // check if data has arrived
        return -1;
    return RCREG;          // return received byte
}

inline void setOutputs(uint16_t v)
{
    // TODO!!! set output ports here
}

#define CMD_SIZE (5) // address, data0, data1, data2, chksum (data bits: 01234567891ABCDEF)
#define RES_SIZE (2) // data, chksum (response bits: 0123456)

uint8_t recvMode = 0;
uint8_t recvBuf[CMD_SIZE];
uint8_t recvCnt = 0;
uint8_t resBuf[RES_SIZE];

inline void sendResponse()
{
    UART_Write(resBuf[0], true, false);
    UART_Write(resBuf[1], false, true);
}

inline void commandIn()
{
    uint8_t chksum = 0xff;
    for (uint8_t t = 0; t < CMD_SIZE; t++)
        chksum += recvBuf[t];
    if ((chksum & 127) != 0x00)
        return;
    uint16_t v = ((uint16_t)recvBuf[1]) | (((uint16_t)recvBuf[2])<<7) | (((uint16_t)recvBuf[3])<<14);
    setOutputs(v);
    resBuf[0] = 0;
    chksum = 0xff + resBuf[0];
    resBuf[1] = (0 - chksum) & 127;
    sendResponse();
}

inline void dataIn(uint8_t data)
{
    if ((data & 128) != 0)
    {
        if (data == (DEVICE_ADDRESS | 128))
        {
            recvCnt = 0;
            recvMode = 1;
        } else
            recvMode = 0;
    }
    if (recvMode == 0)
        return;
    recvBuf[recvCnt++] = data;
    if (recvCnt == CMD_SIZE)
    {
        commandIn();
        recvMode = 0;
    }
}

int main()
{
    UART_Init(9600); // initialize at 9600 baud
    __delay_ms(1000);
    while(1)
    {
        int rxData = UART_Read();
        if (rxData >= 0)
            dataIn((uint8_t)rxData); // UART_Write(rxData, true, true);
    }
    return 0;
}

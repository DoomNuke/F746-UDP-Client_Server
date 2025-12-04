#ifndef CLIENT_C
#define CLIENT_C

#define SERVERPORT 8456

typedef enum
{
    UART_TEST = 1,
    I2C_TEST = 2,
    SPI_TEST = 3
}OPERATION;

typedef struct log
{
    char test[20]; 
    char result[64];
    char timestamp[32];
    unsigned int ADC_Value;
}logger;

#endif
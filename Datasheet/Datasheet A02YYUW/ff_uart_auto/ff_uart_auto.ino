/***********************************************************************
 * Copyright(C) 2021 Shenzhen Dianyingpu Technology
 * File : ff_uart_auto.ino
 * Description : Listen to sensor serial data (ff protocol), convert it into distance measurement information, and print it to PC serial port
 *          This example has only been tested on the Arduino Uno development board and includes the SoftwareSerial third-party library. Please add it yourself.
 *          A1 is connected to the sensor RX pin, A0 is connected to the sensor TX pin
 * Version and Date Author Description
 * 211123-A0 CH FF protocol adds header judgment
 * 211123-A0 CH FF protocol adds header judgment
 ***********************************************************************/
/* Include Header Files -------------------------------------------------------------------*/
#include <Arduino.h>
#include <SoftwareSerial.h>

/* Macro Definition -----------------------------------------------------------------------*/
#define SOFT_VERSION "UART_AUTO-211123-A0"  // Software version

#define COM_TX_PIN A1  // Development board analog serial TX pin
#define COM_RX_PIN A0  // Development board analog serial RX pin

/* Type Definition -------------------------------------------------------/
/ Private Variable ------------------------------------------------------/
SoftwareSerial softSerial( COM_RX_PIN, COM_TX_PIN );  // Software simulated serial port
/ Extended Variable -----------------------------------------------------/
/ Function Declaration --------------------------------------------------/
/ Function Implementation ------------------------------------------------*/
void digital_toggle( uint8_t pin )
{
    digitalRead( pin ) ? digitalWrite( pin, 0 ) : digitalWrite( pin, 1 );
}

void setup()
{
    // put your setup code here, to run once:
    pinMode( LED_BUILTIN, OUTPUT );
    Serial.begin( 9600 );
    Serial.println( SOFT_VERSION );
    softSerial.begin( 9600 );
}

void loop()
{
    if ( softSerial.available() )
    {
        static uint8_t  recv_buf[ 10 ] = { 0 };
        static uint16_t recv_buf_cnt   = 0;
        uint16_t        len            = softSerial.readBytes( ( uint8_t* )&recv_buf, 4 );
        if ( len == 4 && recv_buf[0] == 0xff)
        {
            uint8_t sum = recv_buf[ 0 ] + recv_buf[ 1 ] + recv_buf[ 2 ];
            if ( sum == recv_buf[ 3 ] )
            {
                uint16_t distance      = recv_buf[ 1 ] << 8 | recv_buf[ 2 ];
                char     out_txt[ 16 ] = { 0 };
                sprintf( out_txt, "%d mm \r\n", distance );
                Serial.print( out_txt );
                // Serial.write( recv_buf[ 1 ] ), Serial.write( recv_buf[ 2 ] );
            }
        }
    }

    static uint32_t led_cnt = millis();
    if ( millis() - led_cnt > 100 )
    {
        digital_toggle( LED_BUILTIN );
        led_cnt = millis();
    }
}
// main.cpp
/***********************************************************************
 * Copyright(C) 2021 Shenzhen Dianyingpu Technology 
 * Document	:	ff_uart_ctrl.ino
 * Description : Trigger sensor output regularly, analyze sensor output data (ff protocol data) and print to PC serial port
 * This routine has only been tested on the arduino unon development board. It contains two third-party libraries, FlexiTimer2 and SoftwareSerial. Please add them yourself.
 * A1 is connected to the sensor RX pin, A0 is connected to the sensor TX pin
 * Version and Date Author Description
 * 211102-A0  CH  created the file
 * 211123-A0  CH  FF protocol adds header judgment
 ***********************************************************************/
/* include header files -------------------------------------------------------------------*/
#include <Arduino.h>
#include <FlexiTimer2.h>
#include <SoftwareSerial.h>

/* Macro definition -----------------------------------------------------------------------*/
#define SOFT_VERSION "UART_CTRL-211123-A0"  //软件版本

#define COM_TX_PIN A1  // 开发板模拟串口 TX 引脚
#define COM_RX_PIN A0  // 开发板模拟串口 RX 引脚

/* type definition ---------------------------------------------------------------------*/
/* extended variable ---------------------------------------------------------------------*/
/* function declaration ---------------------------------------------------------------------*/
void digital_toggle( uint8_t pin );
void trigger_cb();
/* private variable ---------------------------------------------------------------------*/
SoftwareSerial softSerial( COM_RX_PIN, COM_TX_PIN );  // 软件模拟串口
/* Function implementation ---------------------------------------------------------------------*/
void digital_toggle( uint8_t pin )
{
    digitalRead( pin ) ? digitalWrite( pin, 0 ) : digitalWrite( pin, 1 );
}

void trigger_cb()
{
    digitalWrite( COM_TX_PIN, HIGH );  // Stop triggering
    FlexiTimer2::stop();
}

void setup()
{
    // put your setup code here, to run once:
    pinMode( LED_BUILTIN, OUTPUT );
    Serial.begin( 9600 );
    Serial.println( SOFT_VERSION );
    softSerial.begin( 9600 );
    pinMode( COM_TX_PIN, OUTPUT );  // Low-level pulse, trigger output
    digitalWrite( COM_TX_PIN, HIGH );

    FlexiTimer2::set( 5, 0.0001, trigger_cb );  // Set 500us timing
}

void loop()
{
    static uint32_t trigger_cnt = 0;
    if ( millis() - trigger_cnt > 500 )
    {
        digitalWrite( COM_TX_PIN, LOW );  // Trigger sensor output
        FlexiTimer2::start();

        static uint8_t  recv_buf[ 10 ] = { 0 };
        static uint16_t recv_buf_cnt   = 0;
        uint16_t        len            = softSerial.readBytes( ( uint8_t* )&recv_buf, 4 );
        if ( len == 4 && recv_buf[ 0 ] == 0xff )
        {
            uint8_t sum = recv_buf[ 0 ] + recv_buf[ 1 ] + recv_buf[ 2 ];
            if ( sum == recv_buf[ 3 ] )
            {
                uint16_t distance      = recv_buf[ 1 ] << 8 | recv_buf[ 2 ];
                char     out_txt[ 16 ] = { 0 };
                sprintf( out_txt, "%d mm \r\n", distance );
                Serial.print( out_txt );
            }
        }

        trigger_cnt = millis();
    }

    static uint32_t led_cnt = millis();
    if ( millis() - led_cnt > 100 )
    {
        digital_toggle( LED_BUILTIN );
        led_cnt = millis();
    }
}
// main.cpp
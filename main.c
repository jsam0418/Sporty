#include <msp430.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
void Init_UART(void);
void OUTA_UART(unsigned char A);
unsigned char INCHAR_UART(void);
unsigned char HEX_TO_ASCII(unsigned int A);
unsigned int ASCII_TO_HEX(unsigned char A);
void PRINT_STRING(unsigned char* string);

#define LEFT_FWD BIT0
#define RIGHT_FWD BIT1
#define LEFT_REV BIT2
#define RIGHT_REV BIT3
#define LEFT_ENB BIT4
#define RIGHT_ENB BIT5
unsigned char turn_state;// STOP = 0, FWD = 1, RIGHT = 2, REV = 3, LEFT = 4
int fwd_state; //0=stop, 1 = fwd, -1 = rev

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    _enable_interrupt();
    Init_UART();
    P1DIR |= LEFT_ENB + LEFT_FWD + LEFT_REV + RIGHT_ENB + RIGHT_FWD + RIGHT_REV;
    P1OUT &= 0x00;
    /*
    while(true){
      PRINT_STRING("\n\rWaiting for Data: ");
        unsigned char a = INCHAR_UART();
        OUTA_UART(a);
    }*/
    for(;;){
    }
    return 0;
}


void OUTA_UART(unsigned char A){
    //---------------------------------------------------------------
    //***************************************************************
    //---------------------------------------------------------------
    // IFG2 register (1) = 1 transmit buffer is empty,
    // UCA0TXBUF 8-bit transmit buffer
    // wait for the transmit buffer to be empty before sending the
    // data out
    do{
    }while ((IFG2&0x02)==0);
    // send the data to the transmit buffer
    UCA0TXBUF = A;
}

unsigned char HEX_TO_ASCII(unsigned int A){
    //---------------------------------------------------------------
    //***************************************************************
    //---------------------------------------------------------------
    unsigned char ret;
    if(A <= 9){
        ret = A + 0x30;
    } else {
        ret = A + 0x37;
    }
    return ret;
}

void PRINT_STRING(unsigned char* string)
{
    unsigned char character = *string;
    while(character != 0x00){
        OUTA_UART(character);
        string++;
        character = *string;
    }
}

unsigned int ASCII_TO_HEX(unsigned char A){
    //---------------------------------------------------------------
    //***************************************************************
    //---------------------------------------------------------------
    unsigned int ret;
    if(A <= '9'){
        ret = A - 0x30;
    } else {
        ret = A - 0x37;
    }
    return ret;
}

unsigned char INCHAR_UART(void){
    //---------------------------------------------------------------
    //***************************************************************
    //---------------------------------------------------------------
    // IFG2 register (0) = 1 receive buffer is full,
    // UCA0RXBUF 8 bit receive buffer
    // wait for the receive buffer is full before getting the data
    do{
    }while ((IFG2&0x01)==0);
    // go get the char from the receive buffer
    return UCA0RXBUF;
}

void Init_UART(void){
    //---------------------------------------------------------------
    // Initialization code to set up the UART
    //---------------------------------------------------------------

// P1SEL = 0x06; // Transmit and Receive to port 1 bits 1 and 2
// P1SEL2 = 0x06; // Bits P1.2 transmit and P1.1 receive

 P1SEL |= BIT1 + BIT2;  // P1.1 = RXD, P1.2 = TXD
 P1SEL2 |= BIT1 + BIT2;  // P1.1 = RXD, P1.2 = TXD

 /*UCA0CTL1 |= UCSSEL_2;  // Use system Master Clock at 1MHz
 UCA0BR0 = 104;  // (1 MHz)/(9600 bps)
 UCA0BR1 = 0;
 UCA0MCTL = UCBRS0;  // modulation UCBRSx = 1 found on pg 424 of datasheet
 UCA0CTL1 &= ~UCSWRST;  // ** initialize USCI state machine **
 IE2 |= UCA0RXIE;  // Enable USCI_A0 RX interrupt
 //IE2 |= UCB0RXIE;
 //IE2 = 0x00;
*/

 UCA0CTL0 = 0x00;
 UCA0CTL1 = 0x81;
 UCA0BR0 = 0x68;
 UCA0BR1 = 0x00;
 UCA0MCTL = 0x02;
 UCA0STAT = 0x00;
 UCA0CTL1  = 0x80;
 UCA0CTL1 &= ~UCSWRST;
 IE2 |= UCA0RXIE;
}


//UART RX USCI Interrupt. This triggers when the USCI receives a char.
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    unsigned char b, c;
    if(UCA0RXBUF == 1){
        b = INCHAR_UART();
        c = INCHAR_UART();
        if(abs((b-100)) > abs((c-100))){
            if(b-100 > 10){
              turn_state = 1;
            } else if(b-100 < -10){
                turn_state = 3;
            } else {
                turn_state = 0;
            }
        } else {
            if(c-100 > 10){
              turn_state = 1;
            } else if(c-100 < -10){
                turn_state = 3;
            } else {
                turn_state = 0;
            }
        }
    } else if(UCA0RXBUF == 2){
        b = INCHAR_UART();
        if(b-100 > 10){
          fwd_state = 1;
        } else if(b-100 < -10){
            fwd_state = -1;
        } else {
            fwd_state = 0;
        }
        //c = b>>4&0x0F;
        //b = b&0x0F;
    } else if(UCA0RXBUF == 3){
        c = b>>4&0x0F;
        b = b&0x0F;
    }
}

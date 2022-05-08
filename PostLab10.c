/* 
 * File:   PostLab10.c
 * Author: Marian Lopez
 *
 * Created on 4 de mayo de 2022, 09:14 AM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 1000000

// VARIABLES
char menu[38] = {'1', '.', ' ', 'L', 'e', 'e', 'r', ' ', 'P', 'o', 't', 'e', 'n', 'c', 'i', 'o', 'm', 'e', 't', 'r', 'o', 13, '2', '.', ' ', 'E', 'n', 'v', 'i', 'a', 'r', ' ', 'A', 'S', 'C', 'I', 'I', 13};
char op1[4] = {2, 5, 6, 13};
char op2[2] = {'F', 13};
char respuesta;
char unidades = 0;  
char decenas = 0;
char residuo = 0;
char centenas = 0;
int pot = 0;
int bandera = 0;

// PROTOTIPO DE FUNCIONES
void setup (void);
char transmision (int cant_caracteres, char* mensaje);
void conv_decimal(int var_bin);

// INTERRUPCIONES
void __interrupt() isr (void){
    if(PIR1bits.RCIF){          // Hay datos recibidos?
        respuesta = RCREG;
        PORTB = respuesta;
        bandera = 1;
        }
    if(PIR1bits.ADIF){          // Fue int. ADC?
        conv_decimal(ADRESH);
        PIR1bits.ADIF = 0;      // Limpia bandera int. ADC
    }
    return;
}

// CICLO PRINCIPAL
void main (void){
    setup();
    while(1){
        transmision(38, menu);      // Muesta el menú
        while (!bandera){           // Bandera indica si ya hay respuesta
            if (!ADCON0bits.GO){    // No hay proceso de conversión?
                ADCON0bits.GO = 1;      // Inicia la conversión
            }
        }
        if (respuesta == 0x31){     // Si escoge opción 1
            op1[2] = unidades + 48; // Para código ASCII
            op1[1] = decenas + 48;
            op1[0] = centenas + 48;
            transmision(4, op1);    // Muesta valor de potenciómetro
            bandera = 0;
        }
        if (respuesta == 0x32){     // Si escoge opción 2
            transmision(2, op2);    // Muestra caracter
            bandera = 0;
        }
        
    }
    return;
}

// CONFIGURACIONES
void setup (void){
    ANSEL = 1;
    ANSELH = 0;
    
    TRISA = 1;
    PORTA = 0;
    TRISB = 0;
    PORTB = 0;
    
    // Configuración reloj interno
    OSCCONbits.IRCF = 0B100;    // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuración comunicación serial asíncrona Tabla 12-5
    TXSTAbits.SYNC = 0;         // Modo asíncrono
    TXSTAbits.BRGH = 1;         // Usar BAUD con alta velocidad
    BAUDCTLbits.BRG16 = 1;      // Usar el registro en 16 bits
    
    SPBRG = 25;                 // Baudaje 9600 aprox
    SPBRGH = 0;                 
    
    RCSTAbits.SPEN = 1;         // Habilitar comunicación serial
    TXSTAbits.TX9 = 0;          // Utilizar 8 bits
    TXSTAbits.TXEN = 1;         // Habilitar transmisión
    RCSTAbits.CREN = 1;         // Habilitar recepción
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    
    ADCON1bits.VCFG0 = 0;       // VDD *Referencias internas
    ADCON1bits.VCFG1 = 0;       // VSS
    
    ADCON0bits.CHS = 0b0000;    // Seleccionar AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitar modulo ADC
    __delay_us(40);             // Delay de 40 us
    
    // Configuración interrupciones
    INTCONbits.GIE = 1;         // Int. globales
    INTCONbits.PEIE = 1;        // Int. periféricos
    PIE1bits.RCIE = 1;          // Int. recepciones
    PIE1bits.ADIE = 1;          // Int. ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera int. ADC
    
    ADCON0bits.GO = 1;
    return;
}

// TANSMISION DE DATOS DESDE EL MICRO
char transmision (int cant_caracteres, char* mensaje){
    int indice = 0;
    while(indice < cant_caracteres){
        if (PIR1bits.TXIF){             // Registro libre para transmitir
            TXREG = mensaje[indice];    // Esta info se va al TSR
            indice++;
        }
    }
}

// CONVERSIÓN BINARIO A DECIMAL
void conv_decimal(int var_bin){   
    centenas = var_bin/100;     // Obtener centenas
    residuo = var_bin%100;      // Almacenar el residuo
    decenas = residuo/10;       // Obtener decenas
    unidades = var_bin%10;      // Obtener unidades 
    return;
}
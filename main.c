/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC16F1824
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"

/*
                         Main application
 */


uint16_t     dac_valueA, dac_valueB;
uint8_t      bBUFMSB, bBUFLSB;
uint16_t     AD_NoteIn, AD_Scale, AD_RootNote;
uint32_t     NoteVal, ScaleVal, RootVal, ScaleNumber;
uint32_t     NoteNumber, ScaleNoteNumber, RootNumber;
uint16_t     NoteModulo;
uint16_t     DAC_NoteOutputVal;
uint16_t     ScalNum;

#define ROWS 16
#define COLS 7

// Terminator is 255
uint8_t      ScaleTable[ROWS][COLS] = {
                                    {0,  2,  4,  5,  7,   9,  11}, // Ionian
                                    {0,  2,  4,  6,  7,   9,  11}, // Lydian
                                    {0,  2,  4,  5,  7,   9,  10}, // Mixolydian
                                    {0,  2,  3,  5,  7,   9,  10}, // Dorian
                                    {0,  2,  3,  5,  7,   8,  10}, // Aeolian (nat. Min.))
                                    
                                    {0,  1,  3,  5,  7,   8,  10}, // Phrygian
                                    {0,  1,  3,  5,  6,   8,  10}, // Locrian
                                    {0,  1,  3,  4,  6,   8,  10}, // super Locrian
                                    {0,  2,  3,  5,  7,   8,  11}, // harm. Min.
                                    {0,  2,  3,  5,  7,   9,  11}, // asc. mel. Min.
                                    
                                    {0,  1,  4,  5,  7,   8,  11}, // Gipsy
                                    {0,  3,  5,  7, 10,   0,   0}, // Min. Pent.
                                    {0,  2,  4,  7,  9,   0,   0}, // Maj. Pent.
                                    {0,  3,  5,  6,  7,  10,   0}, // Blues
                                    {0,  1,  3,  5,  6,   7,  11}, // Todi
                                    
                                    {0,  2,  3,  7,  8,   0,   0}  // Hirajoshi
                                };

// ScaleToneSelection:             1st, 9th, 3rd, 11th, 5th, 13th, 7th, 
uint8_t     ScaleToneMask[COLS] = {1,   16,   2,   32,   4,   64,  8}; 

uint16_t    FindNearestNote(uint8_t   Scales[ROWS][COLS], 
                            uint32_t  cNoteNumber, 
                            uint16_t  cScaleSel,
                            uint16_t  cRootNumber,
                            uint16_t  cScaleToneSelection){
    
    
    int16_t  NoteModulo, NoteOctave, currScaleNote;
    int16_t i, smallestDiff, bestNearestNote, currDiff;
    
    NoteModulo = ((int16_t)cNoteNumber - cRootNumber) % 12;
    NoteOctave = ((int16_t)cNoteNumber - cRootNumber) / 12;
    
    if (cScaleToneSelection == 0) cScaleToneSelection = 255;
    
    // Initialization
    i=0;
    smallestDiff  = 255;
    currScaleNote = Scales[cScaleSel][0];
    
    while(i < COLS){ 
        
        // use tone only, if it's a selected scale tone
        if((uint16_t)ScaleToneMask[i] & cScaleToneSelection){
            
            currScaleNote = Scales[cScaleSel][i];
            currDiff      = abs(currScaleNote - NoteModulo);
        
            if(currDiff < smallestDiff){

                smallestDiff    = currDiff;
                bestNearestNote = currScaleNote;
            }
        }
        i++; // try next scale tone
    }
    return (bestNearestNote + NoteOctave*12 + cRootNumber);
 } 


    
void main(void)
{
    
    #define DAC_A      0x00
    #define DAC_B      0x80
    #define OUTENABLE  0x10
      

     // initialize the device
    SYSTEM_Initialize();

    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    //INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    // INTERRUPT_PeripheralInterruptDisable();

    while (1)
    {
        
        AD_NoteIn    = (ADC_GetConversion(NoteIn))   >> 6; 
        AD_Scale     = (ADC_GetConversion(Scale))    >> 6;
        AD_RootNote  = (ADC_GetConversion(RootNote)) >> 6;
        
        // Anm.: Add. 7 zur Vermeidung von "Grenzfällen"
        NoteVal    = ((~AD_NoteIn)   & 0x3FF) + 7; 
        ScaleVal   = ((~AD_Scale)    & 0x3FF) + 2; 
        RootVal    = ((~AD_RootNote) & 0x3FF) - 2 ;
        
        // C3 ... B3 ~ 0 ... 11   ,   C4 ... B4 ~ 12 ... 23   ,   ...
        // C8 ... B8 ~ 60 ... 71  
        NoteNumber  = (((NoteVal <<7)   + (NoteVal <<9)  + (NoteVal<<12)) >>16)  - 2 ;
        ScaleNumber = (((ScaleVal<<13)  + (ScaleVal<<9)  + (ScaleVal<<7)) >>16)  - 8;
        RootNumber  = (((RootVal <<13)  + (RootVal <<9)  + (RootVal <<7)+
                        (RootVal << 6)  + (RootVal <<5)  + (RootVal <<3)) >>16)  - 8;
        
        ScaleNumber = ScaleNumber >= ROWS ? 0 : ScaleNumber; 
        
        
        //NoteModulo = NoteNumber % 12;
        
        ScaleNoteNumber = FindNearestNote(ScaleTable, 
                                          NoteNumber, 
                                          (uint16_t) ScaleNumber, 
                                          (uint16_t) 0,            // eigentlich RootNumber
                                          (uint16_t) RootNumber); // eigentlich ScaleNoteSel
        
        
        // Calculate NoteOutputValue from NoteNumber
        DAC_NoteOutputVal = (uint16_t)((ScaleNoteNumber * 3641) >> 6);  
                
        //DAC-Ausgabewert A berechnen (AD_NoteIn auf 12 bit bringen,
        // invertieren und auf 12 bit beschneiden)
        dac_valueA = DAC_NoteOutputVal;
        // DAC-Ausgabewert B berechnen
        //dac_valueB = (dac_valueB + 2) & 0xFFF;
        
        SPI_Open(SPI_DEFAULT);
        
 
        DAC_CS_SetHigh();
        
        // Ausgabe auf SPI-DAC-Port A zusammenbauen 
        bBUFMSB = (uint8_t)(dac_valueA >> 8)| DAC_A | OUTENABLE; // build MSB
        bBUFLSB = (uint8_t)(dac_valueA & 0x00FF);                // build LSB
        
        DAC_CS_SetLow();
        
        // Ausgabe auf SPI-DAC-Port A
        SPI_WriteByte(bBUFMSB); // transfer MSB
        while(!SSPSTATbits.BF);
        
        SPI_WriteByte(bBUFLSB); // transfer LSB
        while(!SSPSTATbits.BF);
        
        DAC_CS_SetHigh();
        SPI_Close();
        
        /*
        // --------------------------------------------------
        __delay_us(delay_us);
        // --------------------------------------------------
        */
        /*
        SPI_Open(SPI_DEFAULT);
        
        DAC_CS_SetHigh();
        
        // Ausgabe auf SPI-DAC-Port B zusammenbauen
        bBUFMSB = (uint8_t)(dac_valueB >> 8)| DAC_B | OUTENABLE; // build MSB
        bBUFLSB = (uint8_t)(dac_valueB & 0x00FF);                // build LSB
        
        DAC_CS_SetLow();
        
        // Ausgabe auf SPI-DAC-Port B
        SPI_WriteByte(bBUFMSB); // transfer MSB
        while(!SSPSTATbits.BF);
        
        SPI_WriteByte(bBUFLSB); // transfer LSB
        while(!SSPSTATbits.BF);
        
        DAC_CS_SetHigh();
                 
        
        SPI_Close();
        
        __delay_us(delay_us);
        */
    }
}
/**
 End of File
*/
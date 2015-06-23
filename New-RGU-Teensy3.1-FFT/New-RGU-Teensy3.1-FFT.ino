// New RGU-Teensy3.1-FFT

//#include "Functions_and_defines_for_RGU.h"
#include "Arduino.h" 								
#include "arm_math.h"		
#include <ADC.h>
#include <Wire.h>
#include <FFT256.h>	

/////// Necessary for FFT Audio Analysis /////
//typedef struct audio_block_struct {       //
//  unsigned char ref_count;                //
//  unsigned char memory_pool_index;        //
//  unsigned char reserved1;                //
//  unsigned char reserved2;                //
//  int16_t data[512];                      //
//} audio_block_t;                          //
//////////////////////////////////////////////

//////////////////// Defines /////////////////////////
#define AUDIO_BLOCK_SAMPLES  128 	  	    //
#define AUDIO_SAMPLE_RATE    44117.64706       	    //
#define AUDIO_SAMPLE_RATE_EXACT 44117.64706	    //
						    //
#define LOG_OUT 1 // use the log output Functions   //
#define FHT_N 25600 // set to 256 point fht 	    //
//////////////////////////////////////////////////////

////// Global Variables //////
FFT256 FFT1;		    // object to do Fourier Analysis on ADC0
FFT256 FFT2;		    // object to do Fourier Analysis on ADC1
const int readPin = A9;     // pin on ADC0
const int readPin2 = A2;    // pin on ADC1
ADC *adc = new ADC(); 	    // ADC pointer to be used for gathering analog values
//////////////////////////////


void setup()
{

	Serial.begin(9600);
        while ( !Serial ) ; // Wait for Serial port
        Serial.println("Begining Setup...");


	// 	SETUP PINS	//
	//pinMode(LED_BUILTIN, OUTPUT);
	pinMode(readPin, INPUT);
	pinMode(readPin2, INPUT);
        
        Serial.println("adc init...");
	adc_init();	

        // setup PDB
        adc->adc0->stopPDB();
        adc->adc0->startPDB( AUDIO_SAMPLE_RATE ); //frequency in Hz
        adc->adc1->stopPDB();
        adc->adc1->startPDB( AUDIO_SAMPLE_RATE ); //frequency in Hz
          
        //FFT1.windowFunction(AudioWindowHanning256);
        //FFT2.windowFunction(AudioWindowHanning256);
        

        Serial.println("Initialization Complete.");
}

int t=0;
int value, value2;
int16_t audioDataADC0[256];
int16_t audioDataADC1[256];


void loop()
{
    // allocate for audio data
    memset(audioDataADC0, 0, sizeof(audioDataADC0));
    memset(audioDataADC1, 0, sizeof(audioDataADC1));  
  
    // read synchronus data from ADC at 44.1KHz
    for ( int i=0; i<256; i++ )
    {
        value = (uint16_t)adc->analogReadContinuous(ADC_0);   // the unsigned is necessary for 16 bits, otherwise values larger than 3.3/2 V are negative!
        //Serial.println(value*3.3/adc->getMaxValue(ADC_0), DEC);
        //Serial.print("ADC0 val: ");
        //Serial.println(value);
        value2 = (uint16_t)adc->analogReadContinuous(ADC_1);   // the unsigned is necessary for 16 bits, otherwise values larger than 3.3/2 V are negative
        //Serial.println(value2*3.3/adc->getMaxValue(ADC_1), DEC);
        //Serial.print("ADC1 val: ");
        //Serial.println(value2);

    
        // Sometimes the comparison errors flip, Don't know why.
        // I assume it may be because I don't actually have anything
        // attached to the pins to be read.
        if(adc->adc0->fail_flag) {
            Serial.print("ADC0 error flags: 0x");
            Serial.println(adc->adc0->fail_flag, HEX);
            if(adc->adc0->fail_flag == ADC_ERROR_COMPARISON) {
                adc->adc0->fail_flag &= ~ADC_ERROR_COMPARISON; // clear that error
                Serial.println("Comparison error in ADC0");
            }
        }
        if(adc->adc1->fail_flag) {
            Serial.print("ADC1 error flags: 0x");
            Serial.println(adc->adc1->fail_flag, HEX);
            if(adc->adc1->fail_flag == ADC_ERROR_COMPARISON) {
                adc->adc1->fail_flag &= ~ADC_ERROR_COMPARISON; // clear that error
                Serial.println("Comparison error in ADC1");
            }
        }
        
        audioDataADC0[i] = value;
        audioDataADC1[i] = value2;
    }
    

    // write data to FFT objects

    FFT1.inputAudio(audioDataADC0);
    FFT2.inputAudio(audioDataADC1);
    
    
//    for(int i=0; i < 256; i++)
//    {
//      Serial.print("Original mem : "); Serial.println((int)&audioDataADC0[i]);  // Just for testing
//      Serial.print("The Copy mem : "); Serial.println((int)&FFT1.buffer[i]);
//    } HALT();

//    Serial.print("sizeof(audioDataADC0)="); Serial.println(sizeof(audioDataADC0));
//    HALT();
    
    
    // update FFT object
    
    FFT1.update();
    FFT2.update();

   
    // read analysis data from FFT objects
    float analysisData1[512];
    float analysisData2[512];
    memset(analysisData1,0 ,sizeof(analysisData1));
    memset(analysisData2,0 ,sizeof(analysisData2));
    
    if ( FFT1.available() ) 
    {
      for (int i=0; i<40; i++)
      {
        analysisData1[i] = FFT1.read(i);
        if (analysisData1[i] >= 0.01)
        {
          Serial.print(analysisData1[i]);
          Serial.print(" ");
        }
        else
          Serial.print(" - "); // Don't print '0.00'
      } 
      Serial.println();
    }
      
      
      
    if ( FFT2.available() ) 
    {
      for (int i=0; i<40; i++)
      {
        analysisData2[i] = FFT2.read(i);
        if (analysisData2[i] >= 0.01)
        {
          Serial.print(analysisData2[i]);
          Serial.print(" ");
        }
        else
          Serial.print(" - "); // Don't print '0.00'
      } 
      Serial.println();
    }
    
   if (t++ > 10000) HALT();
}



void HALT()
{
  Serial.println("Halting."); 
  while( 1 ) ;
}

// If you enable interrupts make sure to call readSingle() to clear the interrupt.
void adc0_isr() {
        adc->adc0->readSingle();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
}

#if defined(ADC_TEENSY_3_1)
void adc1_isr() {
        adc->adc1->readSingle();
        digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
}
#endif

// pdb interrupt is enabled in case you need it.
void pdb_isr(void) {
        PDB0_SC &=~PDB_SC_PDBIF; // clear interrupt
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
}



void adc_init()	 	// Initialize ADCs by setting proper config on both ADCs and reading once. Must be called before PDB can be initialized.
{
  ///// ADC0 /////
  // reference can be ADC_REF_3V3, ADC_REF_1V2 (not for Teensy LC) or ADC_REF_EXT.
  //adc->setReference(ADC_REF_1V2, ADC_0); // change all 3.3 to 1.2 if you change the reference to 1V2

  adc->setAveraging(1, ADC_0); // set number of averages
  adc->setResolution(16, ADC_0); // set bits of resolution
  
  

  // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED_16BITS, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
  // see the documentation for more information
  adc->setConversionSpeed(ADC_VERY_HIGH_SPEED, ADC_0); // change the conversion speed
  // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
  adc->setSamplingSpeed(ADC_VERY_HIGH_SPEED, ADC_0); // change the sampling speed

  adc->enableInterrupts(ADC_0); // it's necessary to enable interrupts for PDB to work (why?)

  adc->analogRead(readPin, ADC_0); // call this to setup everything before the pdb starts
  //adc->adc0->readSingle();
  //Serial.println("Finished ADC0 init");

  ////// ADC1 /////
#if defined(ADC_TEENSY_3_1)
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(16, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_VERY_HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_VERY_HIGH_SPEED, ADC_1); // change the sampling speed

  adc->enableInterrupts(ADC_1);

  adc->analogRead(readPin2, ADC_1); // call this to setup everything before the pdb starts
  //adc->adc1->readSingle();
  //Serial.println("Finished ADC1 init");

#endif

}


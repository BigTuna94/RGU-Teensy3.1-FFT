// New Class to collect FFT info, derived from stock analyze_fft256 class from Teensy Audio Library.

/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 // This redisigned library is built with the intention to 
 // be used on audio data, outside of the standard Teensy 
 // audio library. Specifically, it is mean to be used with 
 // the Teensy 3.1. This Library's entire purpose is to allow  
 // the use of both ADCs for audio input, by way of the new ADC 
 // library using PBD triggering for true audio frequency data 
 // collection, for FFT analysis, without the hassle of the audio
 // library class objects and inherencies.


// For General use:
// create and object.
// Write audio data to the buffer via inputAudio
// call Update function 
// read analysis via read function

#ifndef FFT256_h_
#define FFT256_h_

#include "Arduino.h"
#include "arm_math.h"
//#include "/home/ubuntu-03/dev/arduino-1.6.4/hardware/teensy/avr/libraries/Audio/utility/sqrt_integer.h"
#include "/home/ubuntu-03/dev/arduino-1.6.4/hardware/teensy/avr/libraries/Audio/utility/dspinst.h"


extern "C" {
extern const int16_t AudioWindowHanning256[];
extern const int16_t AudioWindowBartlett256[];
extern const int16_t AudioWindowBlackman256[];
extern const int16_t AudioWindowFlattop256[];
extern const int16_t AudioWindowBlackmanHarris256[];
extern const int16_t AudioWindowNuttall256[];
extern const int16_t AudioWindowBlackmanNuttall256[];
extern const int16_t AudioWindowWelch256[];
extern const int16_t AudioWindowHamming256[];
extern const int16_t AudioWindowCosine256[];
extern const int16_t AudioWindowTukey256[];
}

class FFT256
{
	public:
		FFT256()
		{
			count = 0;
			naverage = 8;
			outputflag = false;
			arm_cfft_radix4_init_q15(&fft_inst, 256, 0, 1);
		}

		// FFT256(uint8_t newAverage) 	// Set your own average, within reason.
		// {
		// 	count = 0;
		// 	naverage = (newAverage >= 1 && newAverage <= 16) ? newAverage : 8;
		// 	outputflag = false;
		// 	arm_cfft_radix4_init_q15(&fft_inst, 256, 0, 1);	
		// }

		bool available()
		{
			if ( outputflag )
			{
				outputflag = false;	 // available for output, flip the flag.
				return true;		 // it is assumed that the entire "Block"
									 // of analysis data will then be dumped 
									 // and read through the read member.
			}
			return false; 			 // not available for output yet.
		}

		void inputAudio(int16_t data[])
		{
			for (int i=0; i<256; i++) 	// Probably a faster way to
				buffer[i] = data[i]; 	// do this, could porbably 
										// pass data by reference.	
		}

		float read(unsigned int binNumber)
		{
			if (binNumber > 127) 	// outside of range
				return 0.0;
			return (float)(output[binNumber]) * (1.0 / 16384.0);
		}

		float read(unsigned int binFirst, unsigned int binLast)
		{
			if ( binFirst > binLast )
			{
				unsigned int tmp = binLast;
				binLast = binFirst;
				binFirst = tmp;
			}
			if ( binFirst > 127 ) 
				return 0.0;
			if (binLast > 127) 
				binLast = 127;
			uint32_t sum = 0;
			do {
				sum += output[binFirst++];
			} while(binFirst < binLast);
			return (float)sum * (1.0 / 16384.0);
		}

		void averageTogether(uint8_t n)
		{
			if ( n == 0 )
				n = 1;
			naverage = n;
		}

		void windowFunction(const int16_t *w) 
		{
			window = w;
		}

		virtual void update(void);
		uint16_t output[128] __attribute__ ((aligned (4)));

	private:
		const int16_t *window;
		int16_t buffer[512] __attribute__ ((aligned (4)));
		uint32_t sum[128];	// Maybe better to use 256, since we don't have to worry about audio block size, not sure yet.
		uint8_t count;
		uint8_t naverage;	
		bool outputflag;
		arm_cfft_radix4_instance_q15 fft_inst;
};

#endif
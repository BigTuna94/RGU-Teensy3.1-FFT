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

#include "FFT256.h"
#include <stdint.h>


 static void apply_window_to_fft_buffer(void *buffer, const void *window)
{
	int16_t *buf = (int16_t *)buffer;
	const int16_t *win = (int16_t *)window;;

	for (int i=0; i < 256; i++) {
		int32_t val = *buf * *win++;
		//*buf = signed_saturate_rshift(val, 16, 15);
		*buf = val >> 15;
		buf += 2;
	}

}

const uint16_t sqrt_integer_guess_table[33] = {
55109,
38968,
27555,
19484,
13778,
 9742,
 6889,
 4871,
 3445,
 2436,
 1723,
 1218,
  862,
  609,
  431,
  305,
  216,
  153,
  108,
   77,
   54,
   39,
   27,
   20,
   14,
   10,
    7,
    5,
    4,
    3,
    2,
    1,
    0
};

inline uint32_t sqrt_uint32_approx(uint32_t in) __attribute__((always_inline,unused));
inline uint32_t sqrt_uint32_approx(uint32_t in)
{
	uint32_t n = sqrt_integer_guess_table[__builtin_clz(in)];
	n = ((in / n) + n) / 2;
	n = ((in / n) + n) / 2;
	return n;
}


 void FFT256::update(void)
 {

 	if (window) apply_window_to_fft_buffer(buffer, window);
	arm_cfft_radix4_q15(&fft_inst, buffer);

	// G. Heinzel's paper says we're supposed to average the 
	// magnitude squared, then do the square root at the end.
 	if ( count == 0 )
 	{
 		for ( int i=0; i<128; i++ )
 		{
 			uint32_t tmp = *((uint32_t *)buffer + i);
 			uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
 			sum[i] = magsq / naverage; 
 		}
 	}
 	else
 	{
 		for (int i=0; i < 128; i++) 
 		{
			uint32_t tmp = *((uint32_t *)buffer + i);
			uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
			sum[i] += magsq / naverage;
		}
 	}

 	if ( ++count == naverage )
 	{
 		count = 0;
 		for ( int i=0; i<128; i++ )
 		{
 			output[i] = sqrt_uint32_approx(sum[i]);
 		}
 		outputflag = true;
 	}
 }
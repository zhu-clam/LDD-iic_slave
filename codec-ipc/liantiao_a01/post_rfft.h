/* 
   ITU-T G.711-Appendix III  ANSI-C Source Code
   Copyright (c) 2007-2009
   NTT, France Telecom, VoiceAge Corp., Huawei

   Version: 1.0
   Revision Date: Sep. 25, 2009
*/
/*
 *------------------------------------------------------------------------
 *  File: post_rfft.h
 *  header: real fft and ifft prototypes
 *------------------------------------------------------------------------
 */

#ifndef POST_RFFT_H
#define POST_RFFT_H
 
//extern const Word16 r[64];
//extern const Word16 w[16];
/* Index mapping table for FFT */
const Word16 r[64] = {
         0, 32, 16, 48,  8, 40, 24, 56,
         4, 36, 20, 52, 12, 44, 28, 60,
         2, 34, 18, 50, 10, 42, 26, 58,
         6, 38, 22, 54, 14, 46, 30, 62,
         1, 33, 17, 49,  9, 41, 25, 57,
         5, 37, 21, 53, 13, 45, 29, 61,
         3, 35, 19, 51, 11, 43, 27, 59,
         7, 39, 23, 55, 15, 47, 31, 63
};

/* Twiddle factors for FFT */
const Word16 w[16] = {
       -1,  3212,  6393,  9512, 12540, 15447, 18205, 20788,
    23170, 25330, 27246, 28899, 30274, 31357, 32138, 32610
};
 
void rfft16_64(Word16 *xw, Word16 *X);
void rsifft16_64(Word16 *data);

#endif /* POST_RFFT_H */

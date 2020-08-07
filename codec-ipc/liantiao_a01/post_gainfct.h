/* 
   ITU-T G.711-Appendix III  ANSI-C Source Code
   Copyright (c) 2007-2009
   NTT, France Telecom, VoiceAge Corp., Huawei

   Version: 1.0
   Revision Date: Sep. 25, 2009
*/
/*
 *------------------------------------------------------------------------
 *  File: post_gainfct.h
 *  Header: Header of post-processing filter estimation
 *------------------------------------------------------------------------
 */

#ifndef POST_GAINFCT_H
#define POST_GAINFCT_H

//extern const Word16 WinFilt[L_FLT_DIV2+1];
/* Truncating window for impulse response of noise reduction filter */
const Word16 WinFilt[L_FLT_DIV2+1] = {
        0,   278,  1106,  2454,  4276,  6510,  9081, 11900, 14872,
    17895, 20867, 23686, 26257, 28491, 30313, 31661, 32489,
};
 
void postProc_GainProcess(Word16 *X, Word16 X_shift, VAR_GAIN *var_gain,
                          VAR_ANASYNTH *var_anasynth);

void postProc_InitGainProcess(VAR_GAIN *var_gain);

#endif /* POST_GAINFCT_H */

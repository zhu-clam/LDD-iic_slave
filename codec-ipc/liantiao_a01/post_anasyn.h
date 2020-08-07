/* 
   ITU-T G.711-Appendix III  ANSI-C Source Code
   Copyright (c) 2007-2009
   NTT, France Telecom, VoiceAge Corp., Huawei

   Version: 1.0
   Revision Date: Sep. 25, 2009
*/
/*
 *------------------------------------------------------------------------
 *  File: post_anasyn.h
 *  header: Header of analysis/synthesis for post-processing
 *------------------------------------------------------------------------
 */

#ifndef POST_ANASYN_H
#define POST_ANASYN_H

#include "post_const.h"
 
//extern const Word16 Hann_sh16[L_WIN];
//extern const Word16 Hann_sh16_p6[7];
//extern const Word16 Hann_sh16_p6m1[7];
/* Asymmetric Hanning window (for 64-point FFT processing) */
const Word16 Hann_sh16[L_WIN] = {
        34      ,   137     ,   308     ,   547     ,
        852     ,   1222    ,   1656    ,   2151    ,
        2706    ,   3319    ,   3986    ,   4705    ,
        5474    ,   6288    ,   7144    ,   8039    ,
        8969    ,   9931    ,   10919   ,   11930   ,
        12960   ,   14005   ,   15059   ,   16119   ,
        17180   ,   18237   ,   19287   ,   20325   ,
        21346   ,   22346   ,   23322   ,   24268   ,
        25181   ,   26057   ,   26893   ,   27684   ,
        28429   ,   29122   ,   29763   ,   30347   ,
        30872   ,   31337   ,   31739   ,   32077   ,
        32349   ,   32554   ,   32691   ,   32759   ,
        32694   ,   32104   ,   30947   ,   29263   ,
        27113   ,   24576   ,   21743   ,   18716   ,
        15604   ,   12521   ,   9578    ,   6880    ,
        4526    ,   2601    ,   1174    ,   296
    };

/* Half of 16-point Hanning window for filter interpolation */
const Word16 Hann_sh16_p6[7] = {
        1656    ,   5474    ,   10919   ,   17180   ,
        23322   ,   28429   ,   31739
    };

/* Complementary half of Hanning window for filter interpolation */
const Word16 Hann_sh16_p6m1[7] = {
        31111   ,   27293   ,   21848   ,   15587   ,
        9445    ,   4338    ,   1028
    };



void   postProc_Init_anaSynth(VAR_ANASYNTH *var_anaSynth);
void   postProc_Analysis(const Word16 *bloc_in, VAR_ANASYNTH *var_anaSynth);
void   postProc_Synthesis(Word16 *bloc_out, const Word16 *filtre,
                          VAR_ANASYNTH *var_anaSynth);
Word16 postProc_Filter16(const Word16 *filter, const Word16 *dataIn,
                         Word16 filt_length);

#endif /* POST_ANASYN_H */


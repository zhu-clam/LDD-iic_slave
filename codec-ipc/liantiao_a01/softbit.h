/* 
   ITU-T G.711-Appendix III  ANSI-C Source Code
   Copyright (c) 2007-2009
   NTT, France Telecom, VoiceAge Corp., Huawei

   Version: 1.0
   Revision Date: Sep. 25, 2009
*/
/*
 *------------------------------------------------------------------------
 *  File: softbit.h
 *  Function: Header of conversion between hardbit and softbit
 *------------------------------------------------------------------------
 */

#ifndef SOFTBIT_H
#define SOFTBIT_H

#define G192_SYNCHEADER      (const unsigned short)0x6B21
#define G192_SYNCHEADER_FER  (const unsigned short)0x6B20
#define G192_BITONE          (const unsigned short)0x0081
#define G192_BITZERO         (const unsigned short)0x007F

#define idxG192_SyncHeader       0 /* Synchronization Header */
#define idxG192_BitstreamLength  1 /* Bitstream Length in soft bit */

#define G192_HeaderSize  (const unsigned int)(idxG192_BitstreamLength+1)


void  hardbit2softbit(int, const unsigned char*, unsigned short*);
void  softbit2hardbit(int, const unsigned short*, unsigned char*);

int   checksoftbit( const unsigned short* bitstream );

#endif

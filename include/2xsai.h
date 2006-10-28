// TODO: see the .cpp file: do we need it?

#ifndef __2XSAI_H__
#define __2XSAI_H__



extern int Init_2xSaI(Uint32);
extern void Super2xSaI(Uint8*srcPtr,Uint32 srcPitch,Uint8 *dstPtr,Uint32 dstPitch,int width,int height);
extern void SuperEagle(Uint8*srcPtr,Uint32 srcPitch,Uint8 *dstPtr,Uint32 dstPitch,int width,int height);
extern void _2xSaI(Uint8*,Uint32,Uint8*,Uint32,int,int);
extern void Scale_2xSaI(Uint8*,Uint32,Uint8*,Uint32,Uint32,Uint32,int,int);



#endif

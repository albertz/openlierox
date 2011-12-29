//
//  CodeAttributes.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 25.12.11.
//  Copyright (c) 2011 Albert Zeyer. All rights reserved.
//
//  code under LGPL

#ifndef OpenLieroX_CodeAttributes_h
#define OpenLieroX_CodeAttributes_h

#ifdef _MSC_VER
#define INLINE __forceinline
#else
#define INLINE __attribute__((always_inline)) inline
#endif

#endif

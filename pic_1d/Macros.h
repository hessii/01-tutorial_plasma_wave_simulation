//
//  Macros.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Macros_h
#define Macros_h


// root namespace
//
#ifndef PIC1D_NAMESPACE
#define PIC1D_NAMESPACE P1D
#define PIC1D_BEGIN_NAMESPACE namespace PIC1D_NAMESPACE {
#define PIC1D_END_NAMESPACE }
#endif


// option to enable funneling boundary pass through the main thread
//
#ifndef PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
#define PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS 0
#endif


#endif /* Macros_h */

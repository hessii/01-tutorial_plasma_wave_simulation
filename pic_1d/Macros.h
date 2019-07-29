//
//  Macros.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Macros_h
#define Macros_h


// root namespace
//
#ifndef HYBRID1D_NAMESPACE
#define HYBRID1D_NAMESPACE H1D
#define HYBRID1D_BEGIN_NAMESPACE namespace HYBRID1D_NAMESPACE {
#define HYBRID1D_END_NAMESPACE }
#endif


// option to enable funneling boundary pass through the main thread
//
#ifndef HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
#define HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS 0
#endif


#endif /* Macros_h */

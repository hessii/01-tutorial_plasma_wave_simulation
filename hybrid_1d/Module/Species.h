//
//  Species.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "../Inputs.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../Predefined.h"
#include "../Macros.h"

HYBRID1D_BEGIN_NAMESPACE
class EField;
class BField;

class Species {
public:
    explicit Species() noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* Species_h */

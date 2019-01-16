//
//  Current.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Current_h
#define Current_h

#include "../Inputs.h"
#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"

HYBRID1D_BEGIN_NAMESPACE
class BField;
class EField;
class Species;

class Current : public GridQ<Vector> {
public:
    explicit Current() noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* Current_h */

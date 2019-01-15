//
//  EField.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef EField_h
#define EField_h

#include "../Inputs.h"
#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"

HYBRID1D_BEGIN_NAMESPACE
class BField;
class Charge;
class Current;

class EField : protected GridQ<Vector> {
public:
};
HYBRID1D_END_NAMESPACE

#endif /* EField_h */

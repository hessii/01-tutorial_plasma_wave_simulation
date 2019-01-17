//
//  BField.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef BField_h
#define BField_h

#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"

HYBRID1D_BEGIN_NAMESPACE
class EField;

class BField : public GridQ<Vector> {
public:
    explicit BField();

    void update(EField const &efield, Real const dt) noexcept;
private:
    static inline void _update(BField &B, EField const &E, Real cdtODx) noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* BField_h */

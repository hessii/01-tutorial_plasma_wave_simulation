//
//  EField.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef EField_h
#define EField_h

#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"
#include "../Utility/Scalar.h"

HYBRID1D_BEGIN_NAMESPACE
class BField;
class Charge;
class Current;

class EField : public GridQ<Vector> {
    GridQ<Vector> Je;
    GridQ<Scalar> Pe;

public:
    void update(BField const &bfield, Charge const &charge, Current const &current) noexcept;

private:
    static inline void _update_Pe(decltype(Pe) &Pe, Charge const &rho) noexcept;
    static inline void _update_Je(decltype(Je) &Je, Current const &Ji, BField const &B) noexcept;
    inline void _update_E(EField &E, BField const &B, Charge const &rho) const noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* EField_h */

//
//  BField.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef BField_h
#define BField_h

#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"
#include "../InputWrapper.h"

PIC1D_BEGIN_NAMESPACE
class EField;

class BField : public GridQ<Vector, Input::Nx> {
public:
    explicit BField();
    BField &operator=(BField const &) noexcept;

    static Vector const B0; // background magnetic field

    void update(EField const &efield, Real const dt) noexcept;
private:
    static inline void _update(BField &B, EField const &E, Real const cdtODx) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* BField_h */

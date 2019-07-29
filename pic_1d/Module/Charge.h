//
//  Charge.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Charge_h
#define Charge_h

#include "../Utility/GridQ.h"
#include "../Utility/Scalar.h"

PIC1D_BEGIN_NAMESPACE
class Species;

/// charge density
///
class Charge : public GridQ<Scalar> {
    GridQ<Scalar> tmp;

public:
    virtual ~Charge() = default;

    void reset() noexcept { this->fill(Scalar{0}); }
    void smooth() noexcept { _smooth(tmp, *this), this->swap(tmp); }
    virtual Charge &operator+=(Species const &sp) noexcept;
};

/// Λ
///
class Lambda : public Charge {
    using Charge::smooth;

public:
    Lambda &operator+=(Species const &sp) noexcept override;
};
PIC1D_END_NAMESPACE

#endif /* Charge_h */

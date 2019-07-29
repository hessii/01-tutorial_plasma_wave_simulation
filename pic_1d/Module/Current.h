//
//  Current.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Current_h
#define Current_h

#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"

PIC1D_BEGIN_NAMESPACE
class BField;
class EField;
class Lambda;
class Species;
class Gamma;

/// current density
///
class Current : public GridQ<Vector> {
    GridQ<Vector> tmp;

public:
    virtual ~Current() = default;

    void reset() noexcept { this->fill(Vector{0}); }
    void smooth() noexcept { _smooth(tmp, *this), this->swap(tmp); }
    virtual Current &operator+=(Species const &sp) noexcept;
    void advance(Lambda const &lambda, Gamma const &gamma, BField const &bfield, EField const &efield, Real const dt) noexcept;

private:
    static inline void _advance(Current &J, Lambda const &L, Gamma const &G, BField const &B, EField const &E, Real const dt) noexcept;
};

/// Γ
///
class Gamma : public Current {
    using Current::smooth;
    using Current::advance;

public:
    Gamma &operator+=(Species const &sp) noexcept override;
};
PIC1D_END_NAMESPACE

#endif /* Current_h */

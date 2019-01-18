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
class Lambda;
class Species;

/// Γ
///
class Gamma : public GridQ<Vector> {
public:
    virtual ~Gamma() = default;

    void reset() noexcept { this->fill(Vector{0}); }
    virtual Gamma &operator+=(Species const &sp) noexcept;
};

/// current density
///
class Current : public Gamma {
    GridQ<Vector> tmp;

public:
    void smooth() noexcept { GridQ<Vector>::smooth(tmp); }
    Current &operator+=(Species const &sp) noexcept override;
    void advance(Lambda const &lambda, Gamma const &gamma, BField const &bfield, EField const &efield, Real const dt) noexcept;

private:
    static inline void _advance(Current &J, Lambda const &L, Gamma const &G, BField const &B, EField const &E, Real const dt) noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* Current_h */

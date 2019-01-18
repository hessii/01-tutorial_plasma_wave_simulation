//
//  Charge.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Charge_h
#define Charge_h

#include "../Inputs.h"
#include "../Utility/GridQ.h"
#include "../Utility/Scalar.h"

HYBRID1D_BEGIN_NAMESPACE
class Species;

class Lambda : public GridQ<Scalar> {
public:
    virtual ~Lambda() = default;

    void reset() noexcept { this->fill(Scalar{0}); }
    virtual Lambda &operator+=(Species const &sp) noexcept;
};

class Charge : public Lambda {
    GridQ<Scalar> tmp;

public:
    void smooth() noexcept { GridQ<Scalar>::smooth(tmp); }
    Charge &operator+=(Species const &sp) noexcept override;
};
HYBRID1D_END_NAMESPACE

#endif /* Charge_h */

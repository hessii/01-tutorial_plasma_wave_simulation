//
//  Delegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Delegate.h"
#include "../Module/Domain.h"
#include "../InputWrapper.h"

#include <algorithm>

// MARK: Interface
//
void P1D::Delegate::once(Domain &)
{
}

void P1D::Delegate::partition(PartSpecies &sp, PartBucket &L_bucket, PartBucket &R_bucket)
{
    // note that particle position is already normalized by the grid size

    // group particles that have crossed left boundaries
    //
    auto L_it = std::partition(sp.bucket.begin(), sp.bucket.end(), [LB = 0.0](Particle const &ptl) noexcept->bool {
        return ptl.pos_x >= LB;
    });
    L_bucket.insert(L_bucket.cend(), L_it, sp.bucket.end());
    sp.bucket.erase(L_it, sp.bucket.end());

    // group particles that have crossed right boundaries
    //
    auto R_it = std::partition(sp.bucket.begin(), sp.bucket.end(), [RB = sp.params.domain_extent.len](Particle const &ptl) noexcept->bool {
        return ptl.pos_x < RB;
    });
    R_bucket.insert(R_bucket.cend(), R_it, sp.bucket.end());
    sp.bucket.erase(R_it, sp.bucket.end());
}
void P1D::Delegate::pass(Domain const& domain, PartSpecies &sp)
{
    PartSpecies::bucket_type L, R;
    partition(sp, L, R);
    pass(domain, L, R);
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}

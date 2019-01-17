//
//  Species.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "./SpeciesBase.h"

HYBRID1D_BEGIN_NAMESPACE
class EField;
class BField;

class Species : public _Species {
public:
    using _Species::_Species;
};
HYBRID1D_END_NAMESPACE

#endif /* Species_h */

//
//  main.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./VDF/MaxwellianVDF.h"
#include "./Module/Species.h"

#include <iostream>
#include <fstream>

int main(int argc, const char * argv[]) {
    {
        constexpr long Nc = 100;
        constexpr double vth1 = 2, T2OT1 = 1.5;
        H1D::Species const sp{1, 1, Nc, H1D::MaxwellianVDF{vth1, T2OT1}};

        std::ofstream of{"/Users/kyungguk/Downloads/maxwellian.m"};
        of << sp.bucket;
    }
    return 0;
}

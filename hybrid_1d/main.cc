//
//  main.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include <iostream>

#include "Module/BField.h"

int main(int argc, const char * argv[]) {
    {
        H1D::BField B;
        std::cout << B << std::endl;
    }
    return 0;
}

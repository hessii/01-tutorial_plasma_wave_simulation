//
//  main.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./Driver.h"

#include <chrono>
#include <iostream>

int main(int argc, const char * argv[]) {
    auto const start = std::chrono::steady_clock::now();
    {
        H1D::Driver{}.run();
    }
    auto const end = std::chrono::steady_clock::now();
    std::chrono::duration<double> const diff = end - start;
    std::cout << __FUNCTION__ << " - time elapsed: " << diff.count() << "s" << std::endl;
    return 0;
}

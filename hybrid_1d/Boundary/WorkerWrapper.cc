//
//  WorkerWrapper.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "WorkerWrapper.h"
#include "../Module/BField.h"
#include "../Module/EField.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

void H1D::WorkerWrapper::pass(Domain const&, Species &sp)
{
    constexpr auto tag = InterThreadComm::pass_species_tag{};

    comm.worker_thread_request_to_process(tag, &sp.bucket);
}
void H1D::WorkerWrapper::pass(Domain const&, BField &bfield)
{
    constexpr auto tag = InterThreadComm::pass_bfield_tag{};

    comm.worker_thread_request_to_process(tag, &bfield);
}
void H1D::WorkerWrapper::pass(Domain const&, EField &efield)
{
    constexpr auto tag = InterThreadComm::pass_efield_tag{};

    comm.worker_thread_request_to_process(tag, &efield);
}
void H1D::WorkerWrapper::pass(Domain const&, Charge &charge)
{
    constexpr auto tag = InterThreadComm::pass_charge_tag{};

    comm.worker_thread_request_to_process(tag, &charge);
}
void H1D::WorkerWrapper::pass(Domain const&, Current &current)
{
    constexpr auto tag = InterThreadComm::pass_current_tag{};

    comm.worker_thread_request_to_process(tag, &current);
}
void H1D::WorkerWrapper::gather(Domain const&, Charge &charge)
{
    constexpr auto tag = InterThreadComm::gather_charge_tag{};

    comm.worker_thread_request_to_process(tag, &charge);
}
void H1D::WorkerWrapper::gather(Domain const&, Current &current)
{
    constexpr auto tag = InterThreadComm::gather_current_tag{};

    comm.worker_thread_request_to_process(tag, &current);
}
void H1D::WorkerWrapper::gather(Domain const&, Species &sp)
{
    constexpr auto tag = InterThreadComm::gather_species_tag{};

    comm.worker_thread_request_to_process(tag, &sp.moments());
}

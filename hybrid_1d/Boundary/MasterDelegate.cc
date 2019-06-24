//
//  MasterDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MasterDelegate.h"
#include "../Module/BField.h"
#include "../Module/EField.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

#include <memory>

H1D::MasterDelegate::~MasterDelegate()
{
}
H1D::MasterDelegate::MasterDelegate(std::unique_ptr<Delegate> delegate) noexcept
: delegate{std::move(delegate)}
{
    for (unsigned i = 0; i < workers.size(); ++i) {
        workers[i].master = this;
        workers[i].id = i;
    }
}

#if defined(HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
void H1D::MasterDelegate::pass(Domain const& domain, Species &sp)
{
    if (this->_particle_pass_flag) {
        std::deque<Particle> L, R;
        delegate->partition(sp, L, R);
        {
            delegate->pass(domain, L, R);
            for (WorkerDelegate &worker : workers) {
                worker.constant_comm.send(*this, std::make_pair(&L, &R))();
                delegate->pass(domain, L, R);
            }
        }
        sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
        sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
    } else {
        std::deque<Particle> L, R;
        delegate->partition(sp, L, R);
        
        // 1. gather
        //
        auto const payload = std::make_pair(&L, &R);
        for (WorkerDelegate &worker : workers) {
            worker.constant_comm.send<3>(*this, payload)();
        }
        
        // 2. boundary pass
        //
        delegate->pass(domain, L, R); // L and R switched upon return
        
        // 3. distribute
        //
        for (WorkerDelegate &worker : workers) {
            tickets.push_back(worker.constant_comm.send<3>(*this, payload));
        }
        unsigned long const stride = workers.size() + 1;
        for (unsigned long i = stride - 1, n = payload.first->size(); i < n; i += stride) {
            sp.bucket.push_back(payload.first->operator[](i));
        }
        
        for (unsigned long i = stride - 1, n = payload.second->size(); i < n; i += stride) {
            sp.bucket.push_back(payload.second->operator[](i));
        }
        tickets.clear();
    }
}
void H1D::MasterDelegate::pass(Domain const& domain, BField &bfield)
{
    delegate->pass(domain, bfield);
    broadcast_to_workers(bfield);
}
void H1D::MasterDelegate::pass(Domain const& domain, EField &efield)
{
    delegate->pass(domain, efield);
    broadcast_to_workers(efield);
}
void H1D::MasterDelegate::pass(Domain const& domain, Charge &charge)
{
    delegate->pass(domain, charge);
    broadcast_to_workers(charge);
}
void H1D::MasterDelegate::pass(Domain const& domain, Current &current)
{
    delegate->pass(domain, current);
    broadcast_to_workers(current);
}
#endif
void H1D::MasterDelegate::gather(Domain const& domain, Charge &charge)
{
    collect_from_workers(charge);
    delegate->gather(domain, charge);
    broadcast_to_workers(charge);
}
void H1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    collect_from_workers(current);
    delegate->gather(domain, current);
    broadcast_to_workers(current);
}
void H1D::MasterDelegate::gather(Domain const& domain, Species &sp)
{
    {
        collect_from_workers(sp.moment<0>());
        collect_from_workers(sp.moment<1>());
        collect_from_workers(sp.moment<2>());
    }
    delegate->gather(domain, sp);
    {
        broadcast_to_workers(sp.moment<0>());
        broadcast_to_workers(sp.moment<1>());
        broadcast_to_workers(sp.moment<2>());
    }
}

template <class T>
void H1D::MasterDelegate::broadcast_to_workers(GridQ<T> const &payload)
{
    for (WorkerDelegate &worker : workers) {
        tickets.push_back(worker.constant_comm.send(*this, &payload));
    }
    tickets.clear();
}
template <class T>
void H1D::MasterDelegate::collect_from_workers(GridQ<T> &buffer)
{
    // the first worker will collect all workers'
    //
    if (auto first = workers.begin(); first != workers.end()) {
        first->mutable_comm.send(*this, &buffer)();
    }
}

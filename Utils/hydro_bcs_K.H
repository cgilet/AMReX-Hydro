#ifndef HYDRO_BCS_K_H
#define HYDRO_BCS_K_H

#include <AMReX_Gpu.H>
#include <AMReX_BCRec.H>
#include <AMReX_BC_TYPES.H>
#include <AMReX_Array.H>
#include <iomanip>
#include <hydro_constants.H>

/* This header file contains the inlined __host__ __device__ functions required for
   the bc routines for Godunov and MOL. It also contains function declarations
   for controlling host functions.  */

namespace {

AMREX_GPU_HOST_DEVICE AMREX_FORCE_INLINE
void SetXEdgeBCs( const int i, const int j, const int k, const int n,
                  const amrex::Array4<const amrex::Real> &s,
                  amrex::Real &lo, amrex::Real &hi,
                  const int bclo, const int domlo,
                  const int bchi, const int domhi,
                  const bool is_velocity )
{
    using namespace amrex;

    if (i == domlo)
    {
        if (bclo == BCType::ext_dir)
        {
            lo = s(domlo-1, j, k, n);
            if ( n==XVEL && is_velocity ) hi=lo;
        }
        else if ( bclo == BCType::foextrap || bclo == BCType::hoextrap ||
                  bclo == BCType::reflect_even )
        {
            lo = hi;
        }
        else if (bclo == BCType::reflect_odd)
        {
            hi = 0.;
            lo = hi;
        }
    }
    else if (i == domhi+1)
    {
        if (bchi == BCType::ext_dir)
        {
            hi = s(domhi+1, j, k, n);
            if ( n==XVEL && is_velocity ) lo = hi;
        }
        else if ( bchi == BCType::foextrap || bchi == BCType::hoextrap ||
                  bchi == BCType::reflect_even )
        {
            hi = lo;
        }
        else if (bchi == BCType::reflect_odd)
        {
            lo = 0.;
            hi = lo;
        }
    }
    else
        return;
}


AMREX_GPU_HOST_DEVICE AMREX_FORCE_INLINE
void SetYEdgeBCs ( const int i, const int j, const int k, const int n,
                   const amrex::Array4<const amrex::Real> &s,
                   amrex::Real &lo, amrex::Real &hi,
                   const int bclo, const int domlo,
                   const int bchi, const int domhi,
                   const bool is_velocity )
{
    using namespace amrex;

    if (j == domlo)
    {
        if (bclo == BCType::ext_dir)
        {
            lo = s(i, domlo-1, k, n);
            if ( n==YVEL && is_velocity ) hi=lo;
        }
        else if ( bclo == BCType::foextrap || bclo == BCType::hoextrap ||
                  bclo == BCType::reflect_even )
        {
            lo = hi;
        }
        else if (bclo == BCType::reflect_odd)
        {
            hi = 0.;
            lo = hi;
        }
    }
    else  if (j == domhi+1)
    {
        if (bchi == BCType::ext_dir)
        {
            hi = s(i, domhi+1, k, n);
            if ( n==YVEL && is_velocity ) lo=hi;
        }
        else if ( bchi == BCType::foextrap || bchi == BCType::hoextrap ||
                  bchi == BCType::reflect_even )
        {
            hi = lo;
        }
        else if(bchi == BCType::reflect_odd)
        {
            lo = 0.;
            hi = lo;
        }
    }
    else
        return;
}


//                                                                             //
//   =============================== 3D ONLY ===============================   //
//                                                                             //
#if (AMREX_SPACEDIM==3)
AMREX_GPU_HOST_DEVICE AMREX_FORCE_INLINE
void SetZEdgeBCs ( const int i, const int j, const int k, const int n,
                   const amrex::Array4<const amrex::Real> &s,
                   amrex::Real &lo, amrex::Real &hi,
                   const int bclo, const int domlo,
                   const int bchi, const int domhi,
                   const bool is_velocity )
{
    using namespace amrex;

    if (k == domlo)
    {
        if (bclo == BCType::ext_dir)
        {
            lo = s(i, j, domlo-1, n);
            if ( n==ZVEL && is_velocity ) hi=lo;
        }
        else if ( bclo == BCType::foextrap || bclo == BCType::hoextrap ||
                  bclo == BCType::reflect_even )
        {
            lo = hi;
        }
        else if(bclo == BCType::reflect_odd)
        {
            hi = 0.;
            lo = hi;
        }
    }
    else if (k == domhi+1)
    {
        if (bchi == BCType::ext_dir)
        {
            hi = s(i,j,domhi+1, n);
            if ( n==ZVEL && is_velocity ) lo=hi;
        }
        else if ( bchi == BCType::foextrap || bchi == BCType::hoextrap ||
                  bchi == BCType::reflect_even )
        {
            hi = lo;
        }
        else if(bchi == BCType::reflect_odd)
        {
            lo = 0.;
            hi = lo;
        }
    }
    else
        return;
}
#endif
} // namespace
#endif

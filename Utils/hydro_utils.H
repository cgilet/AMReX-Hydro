#ifndef HYDRO_UTILS_H
#define HYDRO_UTILS_H

#include <AMReX_MultiFabUtil.H>
#include <AMReX_BCRec.H>

#ifdef AMREX_USE_EB
#include <AMReX_EBFabFactory.H>
#include <AMReX_EBMultiFabUtil.H>
#endif

namespace HydroUtils {

void
ComputeFluxesOnBoxFromState ( amrex::Box const& bx, int ncomp, amrex::MFIter& mfi,
                             amrex::Array4<amrex::Real const> const& q,
                             AMREX_D_DECL(amrex::Array4<amrex::Real> const& flux_x,
                                          amrex::Array4<amrex::Real> const& flux_y,
                                          amrex::Array4<amrex::Real> const& flux_z),
                             AMREX_D_DECL(amrex::Array4<amrex::Real> const& xface,
                                          amrex::Array4<amrex::Real> const& yface,
                                          amrex::Array4<amrex::Real> const& zface),
                             AMREX_D_DECL(amrex::Array4<amrex::Real const> const& umac,
                                          amrex::Array4<amrex::Real const> const& vmac,
                                          amrex::Array4<amrex::Real const> const& wmac),
                             amrex::Array4<amrex::Real const> const& divu,
                             amrex::Array4<amrex::Real const> const& fq,
                             amrex::Geometry geom,
                             amrex::Real l_dt,
                             amrex::Vector<amrex::BCRec> const& h_bcrec,
                             const amrex::BCRec* d_bcrec,
                             int const* iconserv,
#ifdef AMREX_USE_EB
                             const amrex::EBFArrayBoxFactory& ebfact,
#endif
                             bool godunov_use_ppm, bool godunov_use_forces_in_trans,
                             bool is_velocity, bool fluxes_are_area_weighted,
                             std::string& advection_type);

void
ExtrapVelToFaces ( amrex::MultiFab const& vel,
                   amrex::MultiFab const& vel_forces,
                   AMREX_D_DECL(amrex::MultiFab& u_mac,
                                amrex::MultiFab& v_mac,
                                amrex::MultiFab& w_mac),
                   amrex::Vector<amrex::BCRec> const& h_bcrec,
                   amrex::BCRec  const* d_bcrec,
                   amrex::Geometry& geom,
                   amrex::Real l_dt,
#ifdef AMREX_USE_EB
                   const amrex::EBFArrayBoxFactory& ebfact,
#endif
                   bool godunov_use_ppm, bool godunov_use_forces_in_trans,
                   std::string advection_type);

void ComputeFluxes ( amrex::Box const& bx,
                     AMREX_D_DECL( amrex::Array4<amrex::Real> const& flux_x,
                                   amrex::Array4<amrex::Real> const& flux_y,
                                   amrex::Array4<amrex::Real> const& flux_z),
                     AMREX_D_DECL( amrex::Array4<amrex::Real const> const& umac,
                                   amrex::Array4<amrex::Real const> const& vmac,
                                   amrex::Array4<amrex::Real const> const& wmac),
                     AMREX_D_DECL( amrex::Array4<amrex::Real const> const& xface,
                                   amrex::Array4<amrex::Real const> const& yface,
                                   amrex::Array4<amrex::Real const> const& zface),
                     amrex::Geometry const& geom, const int ncomp,
                     bool fluxes_are_area_weighted);

void ComputeDivergence ( amrex::Box const& bx,
                         amrex::Array4<amrex::Real> const& div,
                         AMREX_D_DECL( amrex::Array4<amrex::Real const> const& flux_x,
                                       amrex::Array4<amrex::Real const> const& flux_y,
                                       amrex::Array4<amrex::Real const> const& flux_z),
                         AMREX_D_DECL( amrex::Array4<amrex::Real const> const& xface,
                                       amrex::Array4<amrex::Real const> const& yface,
                                       amrex::Array4<amrex::Real const> const& zface),
                         AMREX_D_DECL( amrex::Array4<amrex::Real const> const& umac,
                                       amrex::Array4<amrex::Real const> const& vmac,
                                       amrex::Array4<amrex::Real const> const& wmac),
                         const int ncomp, amrex::Geometry const& geom,
                         int const* iconserv, const amrex::Real mult,
                         bool fluxes_are_area_weighted);


#if (AMREX_SPACEDIM==2)


void ComputeFluxesRZ ( amrex::Box const& bx,
                       amrex::Array4<amrex::Real> const& fx,
                       amrex::Array4<amrex::Real> const& fy,
                       amrex::Array4<amrex::Real const> const& umac,
                       amrex::Array4<amrex::Real const> const& vmac,
                       amrex::Array4<amrex::Real const> const& xed,
                       amrex::Array4<amrex::Real const> const& yed,
                       amrex::Array4<amrex::Real const> const& areax,
                       amrex::Array4<amrex::Real const> const& areay,
                       const int ncomp,
                       bool fluxes_are_area_weighted);

void ComputeDivergenceRZ ( amrex::Box const& bx,
                           amrex::Array4<amrex::Real> const& div,
                           amrex::Array4<amrex::Real const> const& fx,
                           amrex::Array4<amrex::Real const> const& fy,
                           amrex::Array4<amrex::Real const> const& xed,
                           amrex::Array4<amrex::Real const> const& yed,
                           amrex::Array4<amrex::Real const> const& umac,
                           amrex::Array4<amrex::Real const> const& vmac,
                           amrex::Array4<amrex::Real const> const& areax,
                           amrex::Array4<amrex::Real const> const& areay,
                           amrex::Array4<amrex::Real const> const& vol,
                           const int ncomp, int const* iconserv,
                           const amrex::Real mult,
                           bool fluxes_are_area_weighted);


#endif


#ifdef AMREX_USE_EB

void EB_ComputeFluxes ( amrex::Box const& bx,
                        AMREX_D_DECL( amrex::Array4<amrex::Real> const& fx,
                                      amrex::Array4<amrex::Real> const& fy,
                                      amrex::Array4<amrex::Real> const& fz),
                        AMREX_D_DECL( amrex::Array4<amrex::Real const> const& umac,
                                      amrex::Array4<amrex::Real const> const& vmac,
                                      amrex::Array4<amrex::Real const> const& wmac),
                        AMREX_D_DECL( amrex::Array4<amrex::Real const> const& xedge,
                                      amrex::Array4<amrex::Real const> const& yedge,
                                      amrex::Array4<amrex::Real const> const& zedge),
                        AMREX_D_DECL( amrex::Array4<amrex::Real const> const& apx,
                                      amrex::Array4<amrex::Real const> const& apy,
                                      amrex::Array4<amrex::Real const> const& apz),
                        amrex::Geometry const& geom, const int ncomp,
                        amrex::Array4<amrex::EBCellFlag const> const& flag,
                        bool fluxes_are_area_weighted);


void EB_ComputeDivergence ( amrex::Box const& bx,
                            amrex::Array4<amrex::Real> const& div,
                            AMREX_D_DECL( amrex::Array4<amrex::Real const> const& fx,
                                          amrex::Array4<amrex::Real const> const& fy,
                                          amrex::Array4<amrex::Real const> const& fz),
                            amrex::Array4<amrex::Real const> const& vfrac,
                            const int ncomp, amrex::Geometry const& geom,
                            const amrex::Real mult,
                            bool fluxes_are_area_weighted);

#endif
}
#endif

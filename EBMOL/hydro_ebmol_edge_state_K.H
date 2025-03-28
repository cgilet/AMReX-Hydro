#ifndef HYDRO_EBMOL_EDGE_STATE_K_H_
#define HYDRO_EBMOL_EDGE_STATE_K_H_

#include <AMReX_EB_slopes_K.H>
#include <AMReX_BCRec.H>
#include <hydro_constants.H>
#include <hydro_bcs_K.H>

namespace {

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hydro_ebmol_xedge_state_extdir ( AMREX_D_DECL(int i, int j, int k), int n,
                                             amrex::Array4<amrex::Real const> const& q,
                                             amrex::Array4<amrex::Real const> const& umac,
					     AMREX_D_DECL(amrex::Array4<amrex::Real const> const& fcx,
							  amrex::Array4<amrex::Real const> const& fcy,
							  amrex::Array4<amrex::Real const> const& fcz),
                                             amrex::Array4<amrex::Real const> const& ccc,
                                             amrex::Array4<amrex::Real const> const& vfrac,
                                             amrex::Array4<amrex::EBCellFlag const> const& flag,
                                             amrex::BCRec const* const d_bcrec,
                                             amrex::Box const&  domain,
                                             int order, const bool is_velocity) noexcept
{

#if (AMREX_SPACEDIM==2)
    const int k = 0;
#endif

    amrex::Real qs;
    int  domlo = domain.smallEnd(0);
    int  domhi = domain.bigEnd(0);
    bool extlo = d_bcrec[n].lo(0) == amrex::BCType::ext_dir;
    bool exthi = d_bcrec[n].hi(0) == amrex::BCType::ext_dir;

    if ( extlo && i <= domlo)
    {
        qs = q(domlo-1,j,k,n);
    }
    else if ( exthi && i >= domhi+1)
    {
        qs = q(domhi+1,j,k,n);
    }
    else
    {
        const int domain_ilo = domain.smallEnd(0);
	const int domain_ihi = domain.bigEnd(0);
	const int domain_jlo = domain.smallEnd(1);
	const int domain_jhi = domain.bigEnd(1);
#if (AMREX_SPACEDIM == 3)
	const int domain_klo = domain.smallEnd(2);
	const int domain_khi = domain.bigEnd(2);
#endif

        amrex::Real yf = fcx(i,j,k,0); // local (y,z) of centroid of z-face we are extrapolating to
#if (AMREX_SPACEDIM==3)
        amrex::Real zf = fcx(i,j,k,1);
#endif

        AMREX_D_TERM(amrex::Real xc = ccc(i,j,k,0);, // centroid of cell (i,j,k)
                     amrex::Real yc = ccc(i,j,k,1);,
                     amrex::Real zc = ccc(i,j,k,2););

        AMREX_D_TERM(amrex::Real delta_x = 0.5 + xc;,
                     amrex::Real delta_y = yf  - yc;,
                     amrex::Real delta_z = zf  - zc;);

        amrex::Real cc_qmax = amrex::max(q(i,j,k,n),q(i-1,j,k,n));
        amrex::Real cc_qmin = amrex::min(q(i,j,k,n),q(i-1,j,k,n));

	AMREX_D_TERM(bool extdir_or_ho_ilo = (d_bcrec[n].lo(0) == amrex::BCType::ext_dir) or
		                             (d_bcrec[n].lo(0) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_jlo = (d_bcrec[n].lo(1) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].lo(1) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_klo = (d_bcrec[n].lo(2) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].lo(2) == amrex::BCType::hoextrap););

	AMREX_D_TERM(bool extdir_or_ho_ihi = (d_bcrec[n].hi(0) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(0) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_jhi = (d_bcrec[n].hi(1) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(1) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_khi = (d_bcrec[n].hi(2) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(2) == amrex::BCType::hoextrap););

	const auto& slopes_eb_hi =
            amrex_lim_slopes_extdir_eb(i, j, k, n, q, ccc, vfrac,
                                       AMREX_D_DECL(fcx,fcy,fcz), flag,
                                       AMREX_D_DECL(extdir_or_ho_ilo, extdir_or_ho_jlo, extdir_or_ho_klo),
                                       AMREX_D_DECL(extdir_or_ho_ihi, extdir_or_ho_jhi, extdir_or_ho_khi),
                                       AMREX_D_DECL(domain_ilo, domain_jlo, domain_klo),
                                       AMREX_D_DECL(domain_ihi, domain_jhi, domain_khi),
                                       order);


#if (AMREX_SPACEDIM==3)
        amrex::Real qpls = q(i  ,j,k,n) - delta_x * slopes_eb_hi[0]
                                        + delta_y * slopes_eb_hi[1]
                                        + delta_z * slopes_eb_hi[2];
#else
        amrex::Real qpls = q(i  ,j,k,n) - delta_x * slopes_eb_hi[0]
                                        + delta_y * slopes_eb_hi[1];
#endif

        qpls = amrex::max(amrex::min(qpls, cc_qmax), cc_qmin);

        AMREX_D_TERM(xc = ccc(i-1,j,k,0);, // centroid of cell (i-1,j,k)
                     yc = ccc(i-1,j,k,1);,
                     zc = ccc(i-1,j,k,2););

        AMREX_D_TERM( delta_x = 0.5 - xc;,
                      delta_y = yf  - yc;,
                      delta_z = zf  - zc;);

        // Compute slopes of component "n" of q
        const auto& slopes_eb_lo =
            amrex_lim_slopes_extdir_eb(i-1, j, k, n, q, ccc, vfrac,
                                       AMREX_D_DECL(fcx,fcy,fcz), flag,
                                       AMREX_D_DECL(extdir_or_ho_ilo, extdir_or_ho_jlo, extdir_or_ho_klo),
                                       AMREX_D_DECL(extdir_or_ho_ihi, extdir_or_ho_jhi, extdir_or_ho_khi),
                                       AMREX_D_DECL(domain_ilo, domain_jlo, domain_klo),
                                       AMREX_D_DECL(domain_ihi, domain_jhi, domain_khi),
                                       order);

#if (AMREX_SPACEDIM==3)
        amrex::Real qmns = q(i-1,j,k,n) + delta_x * slopes_eb_lo[0]
                                        + delta_y * slopes_eb_lo[1]
                                        + delta_z * slopes_eb_lo[2];
#else
        amrex::Real qmns = q(i-1,j,k,n) + delta_x * slopes_eb_lo[0]
                                        + delta_y * slopes_eb_lo[1];
#endif

        qmns = amrex::max(amrex::min(qmns, cc_qmax), cc_qmin);

        SetXEdgeBCs(i, j, k, 0, q, qmns, qpls, d_bcrec[n].lo(0), domain_ilo, d_bcrec[n].hi(0), domain_ihi, is_velocity);

        if ( (i==domain_ilo) && (d_bcrec[n].lo(0) == amrex::BCType::foextrap || d_bcrec[n].lo(0) == amrex::BCType::hoextrap) )
        {
            if ( umac(i,j,k) >= 0. && n==XVEL && is_velocity )  qpls = amrex::min(qpls,0.);
            qmns = qpls;
        }
        if ( (i==domain_ihi+1) && (d_bcrec[n].hi(0) == amrex::BCType::foextrap || d_bcrec[n].hi(0) == amrex::BCType::hoextrap) )
        {
            if ( umac(i,j,k) <= 0. && n==XVEL && is_velocity ) qmns = amrex::max(qmns,0.);
            qpls = qmns;
        }

        if (umac(i,j,k) > small_vel)
        {
            qs = qmns;
        }
        else if (umac(i,j,k) < - small_vel)
        {
            qs = qpls;
        }
        else
        {
            qs = 0.5*(qmns+qpls);
        }
    }

    return qs;
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hydro_ebmol_xedge_state ( AMREX_D_DECL(int i, int j, int k), int n,
                                      amrex::Array4<amrex::Real const> const& q,
                                      amrex::Array4<amrex::Real const> const& umac,
                                      AMREX_D_DECL(amrex::Array4<amrex::Real const> const& fcx,
                                                   amrex::Array4<amrex::Real const> const& fcy,
                                                   amrex::Array4<amrex::Real const> const& fcz),
                                      amrex::Array4<amrex::Real const> const& ccc,
                                      amrex::Array4<amrex::Real const> const& vfrac,
                                      amrex::Array4<amrex::EBCellFlag const> const& flag,
                                      amrex::BCRec const* const d_bcrec,
                                      amrex::Box const&  domain,
                                      int order, const bool is_velocity) noexcept
{
#if (AMREX_SPACEDIM==2)
    const int k = 0;
#endif

    const int domain_ilo = domain.smallEnd(0);
    const int domain_ihi = domain.bigEnd(0);

    // local (y,z) of centroid of x-face we are extrapolating to
    amrex::Real yf = fcx(i,j,k,0);
#if (AMREX_SPACEDIM==3)
    amrex::Real zf = fcx(i,j,k,1);
#endif

    AMREX_D_TERM(amrex::Real xc = ccc(i,j,k,0);, // centroid of cell (i,j,k)
                 amrex::Real yc = ccc(i,j,k,1);,
                 amrex::Real zc = ccc(i,j,k,2););

    AMREX_D_TERM(amrex::Real delta_x = 0.5 + xc;,
                 amrex::Real delta_y = yf  - yc;,
                 amrex::Real delta_z = zf  - zc;);

    amrex::Real cc_qmax = amrex::max(q(i,j,k,n),q(i-1,j,k,n));
    amrex::Real cc_qmin = amrex::min(q(i,j,k,n),q(i-1,j,k,n));

    // Compute slopes of component "n" of q
    const auto& slopes_eb_hi = amrex_lim_slopes_eb(i, j, k, n, q, ccc, vfrac,
                                                   AMREX_D_DECL(fcx,fcy,fcz),
                                                   flag, order);

#if (AMREX_SPACEDIM==3)
    amrex::Real qpls = q(i  ,j,k,n) - delta_x * slopes_eb_hi[0]
                                    + delta_y * slopes_eb_hi[1]
                                    + delta_z * slopes_eb_hi[2];
#else
    amrex::Real qpls = q(i  ,j,k,n) - delta_x * slopes_eb_hi[0]
                                    + delta_y * slopes_eb_hi[1];
#endif

    qpls = amrex::max(amrex::min(qpls, cc_qmax), cc_qmin);

    AMREX_D_TERM(xc = ccc(i-1,j,k,0);, // centroid of cell (i-1,j,k)
                 yc = ccc(i-1,j,k,1);,
                 zc = ccc(i-1,j,k,2););

    AMREX_D_TERM(delta_x = 0.5 - xc;,
                 delta_y = yf  - yc;,
                 delta_z = zf  - zc;);

    // Compute slopes of component "n" of q
    const auto& slopes_eb_lo = amrex_lim_slopes_eb(i-1, j, k, n, q, ccc, vfrac,
                                                   AMREX_D_DECL(fcx,fcy,fcz),
                                                   flag, order);

#if (AMREX_SPACEDIM==3)
    amrex::Real qmns = q(i-1,j,k,n) + delta_x * slopes_eb_lo[0]
                                    + delta_y * slopes_eb_lo[1]
                                    + delta_z * slopes_eb_lo[2];
#else
    amrex::Real qmns = q(i-1,j,k,n) + delta_x * slopes_eb_lo[0]
                                    + delta_y * slopes_eb_lo[1];
#endif

    qmns = amrex::max(amrex::min(qmns, cc_qmax), cc_qmin);

    SetXEdgeBCs(i, j, k, 0, q, qmns, qpls, d_bcrec[n].lo(0), domain_ilo, d_bcrec[n].hi(0), domain_ihi, is_velocity);

    if ( (i==domain_ilo) && (d_bcrec[n].lo(0) == amrex::BCType::foextrap || d_bcrec[n].lo(0) == amrex::BCType::hoextrap) )
    {
        if ( umac(i,j,k) >= 0. && n==XVEL && is_velocity )  qpls = amrex::min(qpls,0.);
        qmns = qpls;
    }
    if ( (i==domain_ihi+1) && (d_bcrec[n].hi(0) == amrex::BCType::foextrap || d_bcrec[n].hi(0) == amrex::BCType::hoextrap) )
    {
        if ( umac(i,j,k) <= 0. && n==XVEL && is_velocity ) qmns = amrex::max(qmns,0.);
        qpls = qmns;
    }

    amrex::Real qs;

    if (umac(i,j,k) > small_vel)
    {
        qs = qmns;
    }
    else if (umac(i,j,k) < - small_vel)
    {
        qs = qpls;
    }
    else
    {
        qs = 0.5*(qmns+qpls);
    }

    return qs;
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hydro_ebmol_yedge_state_extdir ( AMREX_D_DECL(int i, int j, int k), int n,
                                             amrex::Array4<amrex::Real const> const& q,
                                             amrex::Array4<amrex::Real const> const& vmac,
					     AMREX_D_DECL(amrex::Array4<amrex::Real const> const& fcx,
                                             amrex::Array4<amrex::Real const> const& fcy,
							  amrex::Array4<amrex::Real const> const& fcz),
                                             amrex::Array4<amrex::Real const> const& ccc,
                                             amrex::Array4<amrex::Real const> const& vfrac,
                                             amrex::Array4<amrex::EBCellFlag const> const& flag,
                                             amrex::BCRec const* const d_bcrec,
                                             amrex::Box const&  domain,
                                             int order, const bool is_velocity) noexcept
{

#if (AMREX_SPACEDIM==2)
    const int k = 0;
#endif

    amrex::Real qs;
    int domlo = domain.smallEnd(1);
    int domhi = domain.bigEnd(1);
    bool extlo = d_bcrec[n].lo(1) == amrex::BCType::ext_dir;
    bool exthi = d_bcrec[n].hi(1) == amrex::BCType::ext_dir;

    if ( extlo && j <= domlo)
    {
        qs = q(i,domlo-1,k,n);
    }
    else if (exthi && j >= domhi+1)
    {
        qs = q(i,domhi+1,k,n);
    }
    else
    {
        const int domain_ilo = domain.smallEnd(0);
	const int domain_ihi = domain.bigEnd(0);
	const int domain_jlo = domain.smallEnd(1);
	const int domain_jhi = domain.bigEnd(1);
#if (AMREX_SPACEDIM == 3)
	const int domain_klo = domain.smallEnd(2);
	const int domain_khi = domain.bigEnd(2);
#endif

	AMREX_D_TERM(bool extdir_or_ho_ilo = (d_bcrec[n].lo(0) == amrex::BCType::ext_dir) or
		                             (d_bcrec[n].lo(0) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_jlo = (d_bcrec[n].lo(1) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].lo(1) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_klo = (d_bcrec[n].lo(2) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].lo(2) == amrex::BCType::hoextrap););

	AMREX_D_TERM(bool extdir_or_ho_ihi = (d_bcrec[n].hi(0) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(0) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_jhi = (d_bcrec[n].hi(1) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(1) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_khi = (d_bcrec[n].hi(2) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(2) == amrex::BCType::hoextrap););

        amrex::Real xf = fcy(i,j,k,0); // local (x,z) of centroid of z-face we are extrapolating to
#if (AMREX_SPACEDIM==3)
        amrex::Real zf = fcy(i,j,k,1);
#endif

        AMREX_D_TERM(amrex::Real xc = ccc(i,j,k,0);, // centroid of cell (i,j,k)
                     amrex::Real yc = ccc(i,j,k,1);,
                     amrex::Real zc = ccc(i,j,k,2););

        AMREX_D_TERM(amrex::Real delta_x = xf  - xc;,
                     amrex::Real delta_y = 0.5 + yc;,
                     amrex::Real delta_z = zf  - zc;);

        amrex::Real cc_qmax = amrex::max(q(i,j,k,n),q(i,j-1,k,n));
        amrex::Real cc_qmin = amrex::min(q(i,j,k,n),q(i,j-1,k,n));

        // Compute slopes of component "n" of q
        const auto& slopes_eb_hi =
            amrex_lim_slopes_extdir_eb(i, j, k, n, q, ccc, vfrac,
                                       AMREX_D_DECL(fcx,fcy,fcz), flag,
                                       AMREX_D_DECL(extdir_or_ho_ilo, extdir_or_ho_jlo, extdir_or_ho_klo),
                                       AMREX_D_DECL(extdir_or_ho_ihi, extdir_or_ho_jhi, extdir_or_ho_khi),
                                       AMREX_D_DECL(domain_ilo, domain_jlo, domain_klo),
                                       AMREX_D_DECL(domain_ihi, domain_jhi, domain_khi),
                                       order);

#if (AMREX_SPACEDIM==3)
        amrex::Real qpls = q(i  ,j,k,n) + delta_x * slopes_eb_hi[0]
                                        - delta_y * slopes_eb_hi[1]
                                        + delta_z * slopes_eb_hi[2];
#else
        amrex::Real qpls = q(i  ,j,k,n) + delta_x * slopes_eb_hi[0]
                                        - delta_y * slopes_eb_hi[1];
#endif

        qpls = amrex::max(amrex::min(qpls, cc_qmax), cc_qmin);

        AMREX_D_TERM(xc = ccc(i,j-1,k,0);, // centroid of cell (i,j-1,k)
                     yc = ccc(i,j-1,k,1);,
                     zc = ccc(i,j-1,k,2););

        AMREX_D_TERM(delta_x = xf  - xc;,
                     delta_y = 0.5 - yc;,
                     delta_z = zf  - zc;);

        // Compute slopes of component "n" of q
        const auto& slopes_eb_lo =
            amrex_lim_slopes_extdir_eb(i, j-1, k, n, q, ccc, vfrac,
                                       AMREX_D_DECL(fcx,fcy,fcz), flag,
                                       AMREX_D_DECL(extdir_or_ho_ilo, extdir_or_ho_jlo, extdir_or_ho_klo),
                                       AMREX_D_DECL(extdir_or_ho_ihi, extdir_or_ho_jhi, extdir_or_ho_khi),
                                       AMREX_D_DECL(domain_ilo, domain_jlo, domain_klo),
                                       AMREX_D_DECL(domain_ihi, domain_jhi, domain_khi),
                                       order);


#if (AMREX_SPACEDIM==3)
        amrex::Real qmns = q(i,j-1,k,n) + delta_x * slopes_eb_lo[0]
                                        + delta_y * slopes_eb_lo[1]
                                        + delta_z * slopes_eb_lo[2];
#else
        amrex::Real qmns = q(i,j-1,k,n) + delta_x * slopes_eb_lo[0]
                                        + delta_y * slopes_eb_lo[1];
#endif

        SetYEdgeBCs(i, j, k, n, q, qmns, qpls, d_bcrec[n].lo(1), domain_jlo, d_bcrec[n].hi(1), domain_jhi, is_velocity);

        if ( (j==domain_jlo) && (d_bcrec[n].lo(1) == amrex::BCType::foextrap || d_bcrec[n].lo(1) == amrex::BCType::hoextrap) )
        {
            if ( vmac(i,j,k) >= 0. && n==YVEL && is_velocity )  qpls = amrex::min(qpls,0.);
            qmns = qpls;
        }
        if ( (j==domain_jhi+1) && (d_bcrec[n].hi(1) == amrex::BCType::foextrap || d_bcrec[n].hi(1) == amrex::BCType::hoextrap) )
        {
            if ( vmac(i,j,k) <= 0. && n==YVEL && is_velocity ) qmns = amrex::max(qmns,0.);
             qpls = qmns;
        }

        qmns = amrex::max(amrex::min(qmns, cc_qmax), cc_qmin);

        if (vmac(i,j,k) > small_vel)
        {
            qs = qmns;
        }
        else if (vmac(i,j,k) < - small_vel)
        {
            qs = qpls;
        }
        else
        {
            qs = 0.5*(qmns+qpls);
        }
    }

    return qs;
}



AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hydro_ebmol_yedge_state ( AMREX_D_DECL(int i, int j, int k), int n,
                                      amrex::Array4<amrex::Real const> const& q,
                                      amrex::Array4<amrex::Real const> const& vmac,
                                      AMREX_D_DECL(amrex::Array4<amrex::Real const> const& fcx,
                                                   amrex::Array4<amrex::Real const> const& fcy,
                                                   amrex::Array4<amrex::Real const> const& fcz),
                                      amrex::Array4<amrex::Real const> const& ccc,
                                      amrex::Array4<amrex::Real const> const& vfrac,
                                      amrex::Array4<amrex::EBCellFlag const> const& flag,
                                      amrex::BCRec const* const d_bcrec,
                                      amrex::Box const&  domain,
                                      int order, const bool is_velocity) noexcept
{
#if (AMREX_SPACEDIM==2)
    const int k = 0;
#endif

    const int domain_jlo = domain.smallEnd(1);
    const int domain_jhi = domain.bigEnd(1);

    // local (x,z) of centroid of z-face we are extrapolating to
    amrex::Real xf = fcy(i,j,k,0);
#if (AMREX_SPACEDIM==3)
    amrex::Real zf = fcy(i,j,k,1);
#endif

    AMREX_D_TERM(amrex::Real xc = ccc(i,j,k,0);, // centroid of cell (i,j,k)
                 amrex::Real yc = ccc(i,j,k,1);,
                 amrex::Real zc = ccc(i,j,k,2););

    AMREX_D_TERM(amrex::Real delta_x = xf  - xc;,
                 amrex::Real delta_y = 0.5 + yc;,
                 amrex::Real delta_z = zf  - zc;);

    amrex::Real cc_qmax = amrex::max(q(i,j,k,n),q(i,j-1,k,n));
    amrex::Real cc_qmin = amrex::min(q(i,j,k,n),q(i,j-1,k,n));

    // Compute slopes of component "n" of q
    const auto& slopes_eb_hi = amrex_lim_slopes_eb(i, j, k, n, q, ccc, vfrac,
                                                   AMREX_D_DECL(fcx,fcy,fcz),
                                                   flag, order);

#if (AMREX_SPACEDIM==3)
    amrex::Real qpls = q(i  ,j,k,n) + delta_x * slopes_eb_hi[0]
                                    - delta_y * slopes_eb_hi[1]
                                    + delta_z * slopes_eb_hi[2];
#else
    amrex::Real qpls = q(i  ,j,k,n) + delta_x * slopes_eb_hi[0]
                                    - delta_y * slopes_eb_hi[1];
#endif

    qpls = amrex::max(amrex::min(qpls, cc_qmax), cc_qmin);

    AMREX_D_TERM(xc = ccc(i,j-1,k,0);, // centroid of cell (i,j-1,k)
                 yc = ccc(i,j-1,k,1);,
                 zc = ccc(i,j-1,k,2););

    AMREX_D_TERM(delta_x = xf  - xc;,
                 delta_y = 0.5 - yc;,
                 delta_z = zf  - zc;);

    // Compute slopes of component "n" of q
    const auto& slopes_eb_lo = amrex_lim_slopes_eb(i, j-1, k, n, q, ccc, vfrac,
                                                   AMREX_D_DECL(fcx,fcy,fcz),
                                                   flag, order);

#if (AMREX_SPACEDIM==3)
    amrex::Real qmns = q(i,j-1,k,n) + delta_x * slopes_eb_lo[0]
                                    + delta_y * slopes_eb_lo[1]
                                    + delta_z * slopes_eb_lo[2];
#else
    amrex::Real qmns = q(i,j-1,k,n) + delta_x * slopes_eb_lo[0]
                                    + delta_y * slopes_eb_lo[1];
#endif

    qmns = amrex::max(amrex::min(qmns, cc_qmax), cc_qmin);

    SetYEdgeBCs(i, j, k, n, q, qmns, qpls, d_bcrec[n].lo(1), domain_jlo, d_bcrec[n].hi(1), domain_jhi, is_velocity);

    if ( (j==domain_jlo) && (d_bcrec[n].lo(1) == amrex::BCType::foextrap || d_bcrec[n].lo(1) == amrex::BCType::hoextrap) )
    {
        if ( vmac(i,j,k) >= 0. && n==YVEL && is_velocity )  qpls = amrex::min(qpls,0.);
        qmns = qpls;
    }
    if ( (j==domain_jhi+1) && (d_bcrec[n].hi(1) == amrex::BCType::foextrap || d_bcrec[n].hi(1) == amrex::BCType::hoextrap) )
    {
        if ( vmac(i,j,k) <= 0. && n==YVEL && is_velocity ) qmns = amrex::max(qmns,0.);
         qpls = qmns;
    }

    amrex::Real qs;

    if (vmac(i,j,k) > small_vel)
    {
        qs = qmns;
    }
    else if (vmac(i,j,k) < - small_vel)
    {
        qs = qpls;
    }
    else
    {
        qs = 0.5*(qmns+qpls);
    }

    return qs;
}




#if (AMREX_SPACEDIM==3)

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hydro_ebmol_zedge_state_extdir ( int i, int j, int k, int n,
                                             amrex::Array4<amrex::Real const> const& q,
                                             amrex::Array4<amrex::Real const> const& wmac,
					     AMREX_D_DECL(amrex::Array4<amrex::Real const> const& fcx,
							  amrex::Array4<amrex::Real const> const& fcy,
							  amrex::Array4<amrex::Real const> const& fcz),
                                             amrex::Array4<amrex::Real const> const& ccc,
                                             amrex::Array4<amrex::Real const> const& vfrac,
                                             amrex::Array4<amrex::EBCellFlag const> const& flag,
                                             amrex::BCRec const* const d_bcrec,
                                             amrex::Box const&  domain,
                                             int order, const bool is_velocity) noexcept
{
    amrex::Real qs;
    int domlo = domain.smallEnd(2);
    int domhi = domain.bigEnd(2);
    bool extlo = d_bcrec[n].lo(2) == amrex::BCType::ext_dir;
    bool exthi = d_bcrec[n].hi(2) == amrex::BCType::ext_dir;

    if ( extlo && k <= domlo)
    {
        qs = q(i,j,domlo-1,n);
    }
    else if ( exthi && k >= domhi+1)
    {
        qs = q(i,j,domhi+1,n);
    }
    else
    {
        const int domain_ilo = domain.smallEnd(0);
	const int domain_ihi = domain.bigEnd(0);
	const int domain_jlo = domain.smallEnd(1);
	const int domain_jhi = domain.bigEnd(1);
#if (AMREX_SPACEDIM == 3)
	const int domain_klo = domain.smallEnd(2);
	const int domain_khi = domain.bigEnd(2);
#endif

	AMREX_D_TERM(bool extdir_or_ho_ilo = (d_bcrec[n].lo(0) == amrex::BCType::ext_dir) or
		                             (d_bcrec[n].lo(0) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_jlo = (d_bcrec[n].lo(1) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].lo(1) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_klo = (d_bcrec[n].lo(2) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].lo(2) == amrex::BCType::hoextrap););

	AMREX_D_TERM(bool extdir_or_ho_ihi = (d_bcrec[n].hi(0) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(0) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_jhi = (d_bcrec[n].hi(1) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(1) == amrex::BCType::hoextrap);,
		     bool extdir_or_ho_khi = (d_bcrec[n].hi(2) == amrex::BCType::ext_dir) or
                                             (d_bcrec[n].hi(2) == amrex::BCType::hoextrap););

        amrex::Real xf = fcz(i,j,k,0); // local (x,y) of centroid of z-face we are extrapolating to
        amrex::Real yf = fcz(i,j,k,1);

        amrex::Real xc = ccc(i,j,k,0); // centroid of cell (i,j,k)
        amrex::Real yc = ccc(i,j,k,1);
        amrex::Real zc = ccc(i,j,k,2);

        amrex::Real delta_x = xf  - xc;
        amrex::Real delta_y = yf  - yc;
        amrex::Real delta_z = 0.5 + zc;

        amrex::Real cc_qmax = amrex::max(q(i,j,k,n),q(i,j,k-1,n));
        amrex::Real cc_qmin = amrex::min(q(i,j,k,n),q(i,j,k-1,n));

        // Compute slopes of component "n" of q
        const auto& slopes_eb_hi =
            amrex_lim_slopes_extdir_eb(i, j, k, n, q, ccc, vfrac,
                                       AMREX_D_DECL(fcx,fcy,fcz), flag,
                                       AMREX_D_DECL(extdir_or_ho_ilo, extdir_or_ho_jlo, extdir_or_ho_klo),
                                       AMREX_D_DECL(extdir_or_ho_ihi, extdir_or_ho_jhi, extdir_or_ho_khi),
                                       AMREX_D_DECL(domain_ilo, domain_jlo, domain_klo),
                                       AMREX_D_DECL(domain_ihi, domain_jhi, domain_khi),
                                       order);

        amrex::Real qpls = q(i,j,k  ,n) + delta_x * slopes_eb_hi[0]
                                        + delta_y * slopes_eb_hi[1]
                                        - delta_z * slopes_eb_hi[2];

        qpls = amrex::max(amrex::min(qpls, cc_qmax), cc_qmin);

        xc = ccc(i,j,k-1,0); // centroid of cell (i,j,k-1)
        yc = ccc(i,j,k-1,1);
        zc = ccc(i,j,k-1,2);

        delta_x = xf  - xc;
        delta_y = yf  - yc;
        delta_z = 0.5 - zc;

        // Compute slopes of component "n" of q
        const auto& slopes_eb_lo =
            amrex_lim_slopes_extdir_eb(i, j, k-1, n, q, ccc, vfrac,
                                       AMREX_D_DECL(fcx,fcy,fcz), flag,
                                       AMREX_D_DECL(extdir_or_ho_ilo, extdir_or_ho_jlo, extdir_or_ho_klo),
                                       AMREX_D_DECL(extdir_or_ho_ihi, extdir_or_ho_jhi, extdir_or_ho_khi),
                                       AMREX_D_DECL(domain_ilo, domain_jlo, domain_klo),
                                       AMREX_D_DECL(domain_ihi, domain_jhi, domain_khi),
                                       order);

        amrex::Real qmns = q(i,j,k-1,n) + delta_x * slopes_eb_lo[0]
                                        + delta_y * slopes_eb_lo[1]
                                        + delta_z * slopes_eb_lo[2];

        qmns = amrex::max(amrex::min(qmns, cc_qmax), cc_qmin);

        SetZEdgeBCs(i, j, k, n, q, qmns, qpls, d_bcrec[n].lo(2), domain_klo, d_bcrec[n].hi(2), domain_khi, is_velocity);

        if ( (k==domain_klo) && (d_bcrec[n].lo(2) == amrex::BCType::foextrap || d_bcrec[n].lo(2) == amrex::BCType::hoextrap) )
        {
            if ( wmac(i,j,k) >= 0. && n==ZVEL && is_velocity )  qpls = amrex::min(qpls,0.);
            qmns = qpls;
        }
        if ( (k==domain_khi+1) && (d_bcrec[n].hi(2) == amrex::BCType::foextrap || d_bcrec[n].hi(2) == amrex::BCType::hoextrap) )
        {
            if ( wmac(i,j,k) <= 0. && n==ZVEL && is_velocity ) qmns = amrex::max(qmns,0.);
            qpls = qmns;
        }

        if (wmac(i,j,k) > small_vel)
        {
            qs = qmns;
        }
        else if (wmac(i,j,k) < -small_vel)
        {
            qs = qpls;
        }
        else
        {
            qs = 0.5*(qmns+qpls);
        }
    }

    return qs;
}




AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hydro_ebmol_zedge_state ( int i, int j, int k, int n,
                                      amrex::Array4<amrex::Real const> const& q,
                                      amrex::Array4<amrex::Real const> const& wmac,
                                      AMREX_D_DECL(amrex::Array4<amrex::Real const> const& fcx,
                                                   amrex::Array4<amrex::Real const> const& fcy,
                                                   amrex::Array4<amrex::Real const> const& fcz),
                                      amrex::Array4<amrex::Real const> const& ccc,
                                      amrex::Array4<amrex::Real const> const& vfrac,
                                      amrex::Array4<amrex::EBCellFlag const> const& flag,
                                      amrex::BCRec const* const d_bcrec,
                                      amrex::Box const&  domain,
                                      int order, const bool is_velocity) noexcept
{
    const int domain_klo = domain.smallEnd(2);
    const int domain_khi = domain.bigEnd(2);

    amrex::Real xf = fcz(i,j,k,0); // local (x,y) of centroid of z-face we are extrapolating to
    amrex::Real yf = fcz(i,j,k,1);

    amrex::Real xc = ccc(i,j,k,0); // centroid of cell (i,j,k)
    amrex::Real yc = ccc(i,j,k,1);
    amrex::Real zc = ccc(i,j,k,2);

    amrex::Real delta_x = xf  - xc;
    amrex::Real delta_y = yf  - yc;
    amrex::Real delta_z = 0.5 + zc;

    amrex::Real cc_qmax = amrex::max(q(i,j,k,n),q(i,j,k-1,n));
    amrex::Real cc_qmin = amrex::min(q(i,j,k,n),q(i,j,k-1,n));

    // Compute slopes of component "n" of q
    const auto& slopes_eb_hi = amrex_lim_slopes_eb(i, j, k, n, q, ccc, vfrac,
                                                   AMREX_D_DECL(fcx,fcy,fcz),
                                                   flag, order);

    amrex::Real qpls = q(i,j,k  ,n) + delta_x * slopes_eb_hi[0]
                                    + delta_y * slopes_eb_hi[1]
                                    - delta_z * slopes_eb_hi[2];

    qpls = amrex::max(amrex::min(qpls, cc_qmax), cc_qmin);

    xc = ccc(i,j,k-1,0); // centroid of cell (i,j,k-1)
    yc = ccc(i,j,k-1,1);
    zc = ccc(i,j,k-1,2);

    delta_x = xf  - xc;
    delta_y = yf  - yc;
    delta_z = 0.5 - zc;

    // Compute slopes of component "n" of q
    const auto& slopes_eb_lo = amrex_lim_slopes_eb(i, j, k-1, n, q, ccc, vfrac,
                                                   AMREX_D_DECL(fcx,fcy,fcz),
                                                   flag, order);

    amrex::Real qmns = q(i,j,k-1,n) + delta_x * slopes_eb_lo[0]
                                    + delta_y * slopes_eb_lo[1]
                                    + delta_z * slopes_eb_lo[2];

    qmns = amrex::max(amrex::min(qmns, cc_qmax), cc_qmin);

    SetZEdgeBCs(i, j, k, n, q, qmns, qpls, d_bcrec[n].lo(2), domain_klo, d_bcrec[n].hi(2), domain_khi, is_velocity);

    if ( (k==domain_klo) && (d_bcrec[n].lo(2) == amrex::BCType::foextrap || d_bcrec[n].lo(2) == amrex::BCType::hoextrap) )
    {
        if ( wmac(i,j,k) >= 0. && n==ZVEL && is_velocity )  qpls = amrex::min(qpls,0.);
        qmns = qpls;
    }
    if ( (k==domain_khi+1) && (d_bcrec[n].hi(2) == amrex::BCType::foextrap || d_bcrec[n].hi(2) == amrex::BCType::hoextrap) )
    {
        if ( wmac(i,j,k) <= 0. && n==ZVEL && is_velocity ) qmns = amrex::max(qmns,0.);
        qpls = qmns;
    }

    amrex::Real qs;

    if (wmac(i,j,k) > small_vel)
    {
        qs = qmns;
    }
    else if (wmac(i,j,k) < -small_vel)
    {
        qs = qpls;
    }
    else
    {
        qs = 0.5*(qmns+qpls);
    }


    return qs;
}

#endif

}

#endif

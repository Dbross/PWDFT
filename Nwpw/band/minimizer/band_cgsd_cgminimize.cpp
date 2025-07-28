
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "Control2.hpp"
#include "band_Geodesic.hpp"

#include "Ion.hpp"
#include "Solid.hpp"
#include "Parallel.hpp"
#include "Cneb.hpp"
#include "util_date.hpp"
#include "util_linesearch.hpp"
#include "util_linesearch_robust.hpp"

namespace pwdft {

/* create dummy function call to Geodesic class functions */
static band_Geodesic *mygeodesic_ptr;
static double dummy_energy(double t) { return mygeodesic_ptr->energy(t); }
static double dummy_denergy(double t) { return mygeodesic_ptr->denergy(t); }

/******************************************
 *                                        *
 *            band_cgsd_cgminimize        *
 *                                        *
 ******************************************/
double band_cgsd_cgminimize(Solid &mysolid, band_Geodesic *mygeodesic, double *E,
                            double *deltae, double *deltac, int current_iteration,
                            int it_in, double tole, double tolc) {
  bool done = false;
  double tmin = 0.0;
  double deltat_min = 1.0e-3;
  double deltat;
  double sum0, sum1, scale, total_energy;
  double dE, max_sigma, min_sigma;
  double Eold, dEold, Enew;
  double tmin0, deltae0;

  Cneb *mygrid = mysolid.mygrid;
  mygeodesic_ptr = mygeodesic;

  /* get the initial gradient and direction */
  // Use persistent buffers instead of allocating new ones
  double *G1 = mysolid.get_persistent_G1();
  double *H0 = mysolid.get_persistent_H0();

  //|-\____|\/-----\/\/->    Start Parallel Section    <-\/\/-----\/|____/-|

#if defined(ENABLE_SCF_DEBUG)
  std::cerr << "[CG DEBUG] Starting band_cgsd_cgminimize" << std::endl;
#endif

  total_energy = mysolid.psi_1get_TSgradient(G1);
#if defined(ENABLE_SCF_DEBUG)
  std::cerr << "[CG DEBUG] psi_1get_TSgradient completed, energy=" << total_energy << std::endl;
#endif

  sum1 = mygrid->gg_traceall(G1, G1);
#if defined(ENABLE_SCF_DEBUG)
  std::cerr << "[CG DEBUG] gg_traceall completed, sum1=" << sum1 << std::endl;
#endif

  Enew = total_energy;

  mygrid->gg_copy(G1, H0);
#if defined(ENABLE_SCF_DEBUG)
  std::cerr << "[CG DEBUG] gg_copy completed" << std::endl;
#endif

  /******************************************
   ****                                  ****
   **** Start of conjugate gradient loop ****
   ****                                  ****
   ******************************************/
  int it = 0;
  tmin = deltat_min;
  while ((!done) && ((it++) < it_in)) {
#if defined(ENABLE_SCF_DEBUG)
    std::cerr << "[CG DEBUG] Starting iteration " << it << std::endl;
#endif

    /* initialize the geoedesic line data structure */
#if defined(ENABLE_SCF_DEBUG)
    std::cerr << "[CG DEBUG] About to call mygeodesic->start" << std::endl;
#endif
    dEold = mygeodesic->start(H0, &max_sigma, &min_sigma);
#if defined(ENABLE_SCF_DEBUG)
    std::cerr << "[CG DEBUG] mygeodesic->start completed, dEold=" << dEold << std::endl;
#endif

    /* line search */
    if (tmin > deltat_min)
      deltat = tmin;
    else
      deltat = deltat_min;

    tmin0 = tmin;
    deltae0 = *deltae;

    Eold = Enew;

#if defined(ENABLE_SCF_DEBUG)
    std::cerr << "[CG DEBUG] About to call util_linesearch_robust" << std::endl;
#endif
            Enew = util_linesearch_robust(0.0, Eold, dEold, deltat, &dummy_energy,
                               &dummy_denergy, 0.50, &tmin0, &deltae0, 2);
#if defined(ENABLE_SCF_DEBUG)
    std::cerr << "[CG DEBUG] util_linesearch_robust completed, Enew=" << Enew << std::endl;
#endif

    tmin = tmin0;
    *deltae = deltae0;
    *deltac = mysolid.rho_error();
    mygeodesic->psi_final(tmin);

    /* exit loop early */
    done = ((it >= it_in) || ((std::fabs(*deltae) < tole) && (*deltac < tolc)));

    /* transport the previous search directions */
    mygeodesic->psi_1transport(tmin, H0);

    /* make psi1 <--- psi2(tmin) */
    mysolid.swap_psi1_psi2();

    if (!done) {
      /* get the new gradient - also updates densities */
#if defined(ENABLE_SCF_DEBUG)
      std::cerr << "[CG DEBUG] About to call psi_1get_TSgradient for new gradient" << std::endl;
#endif
      total_energy = mysolid.psi_1get_TSgradient(G1);
#if defined(ENABLE_SCF_DEBUG)
      std::cerr << "[CG DEBUG] psi_1get_TSgradient completed for new gradient" << std::endl;
#endif

      sum0 = sum1;
      sum1 = mygrid->gg_traceall(G1, G1);

      /* the new direction using Fletcher-Reeves */
      if ((std::fabs(*deltae) <= (1.0e-2)) && (tmin > deltat_min)) {
        if (sum0 > 1.0e-9)
          scale = sum1 / sum0;
        else
          scale = 0.0;

        mygrid->g_Scale(scale, H0);
        mygrid->gg_Sum2(G1, H0);
      }

      /* the new direction using steepest-descent */
      else
        mygrid->gg_copy(G1, H0);

      // mygrid->gg_copy(G1,H0);
    }
  }
  // Making an extra call to electron.run and energy
#if defined(ENABLE_SCF_DEBUG)
  std::cerr << "[CG DEBUG] About to call gen_all_energies" << std::endl;
#endif
  total_energy = mysolid.gen_all_energies();
#if defined(ENABLE_SCF_DEBUG)
  std::cerr << "[CG DEBUG] gen_all_energies completed, total_energy=" << total_energy << std::endl;
#endif

  //|-\____|\/-----\/\/->    End Parallel Section    <-\/\/-----\/|____/-|

  // No deallocation needed - persistent buffers are reused
  // mygrid->g_deallocate(H0);
  // mygrid->g_deallocate(G1);

  return total_energy;
}

} // namespace pwdft

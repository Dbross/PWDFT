#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "Control2.hpp"
#include "band_Geodesic12.hpp"
#include "Ion.hpp"
#include "Solid.hpp"
#include "Parallel.hpp"
#include "Cneb.hpp"
#include "iofmt.hpp"
#include "band_lmbfgs.hpp"
#include "band_lmbfgs2.hpp"
#include "util_date.hpp"
#include "nwpw_cscf_mixing.hpp"
#include "nwpw_scf_adaptive_threshold.hpp"

#include "band_cgsd.hpp"

#include "iofmt.hpp"
#include "debug_macros.hpp"

namespace pwdft {

/******************************************
 *                                        *
 *            band_cgsd_noit_energy       *
 *                                        *
 ******************************************/
double band_cgsd_noit_energy(Solid &mysolid, bool doprint, std::ostream &coutput) 
{
   Parallel *parall = mysolid.mygrid->c3db::parall;
 
   /* generate phase factors and local psp and semicore density */
   mysolid.phafacs_vl_potential_semicore();
 
   double total_energy = mysolid.gen_all_energies();
 
   /* report summary of results */
   if (parall->base_stdio_print && doprint) 
   {
      coutput << "     ==================  optimization turned off  ===================" << std::endl << std::endl;
      coutput << std::endl;
      coutput << mysolid;
   }
   coutput << mysolid.print_filled(parall->base_stdio_print && doprint);
 
   return total_energy;
}

/******************************************
 *                                        *
 *           band_cgsd_energy             *
 *                                        *
 ******************************************/
double band_cgsd_energy(Control2 &control, Solid &mysolid, bool doprint, std::ostream &coutput) 
{
   Parallel *parall = mysolid.mygrid->c3db::parall;
   Cneb *mygrid = mysolid.mygrid;
   Ion *myion = mysolid.myion;
   

 

   
   bool stalled = false;
   int ne[2],ispin,nion;
   double E[70],total_energy,deltae,deltae_old,deltac;
 
   int it_in   = control.loop(0);
   int it_out  = control.loop(1);
   double tole = control.tolerances(0);
   double tolc = control.tolerances(1);
   double scf_alpha = control.scf_alpha();
   double scf_beta  = control.scf_beta();
   double kerker_g0 = control.kerker_g0();
   int diis_histories = control.diis_histories();
   int scf_algorithm = control.scf_algorithm();
 
   double dt = control.time_step();
   double dte = dt/sqrt(control.fake_mass());
 
   int minimizer   = control.minimizer();
   int lmbfgs_size = control.lmbfgs_size();
 
   bool hprint = (parall->is_master() && control.print_level("high") && doprint);
   bool oprint = (parall->is_master() && control.print_level("medium") && doprint);
   bool lprint = (parall->is_master() && control.print_level("low") && doprint);

   // CRITICAL FIX: Check if wavefunction reinitialization is needed
   // This prevents statefulness issues when the function is called multiple times
   if (mysolid.should_force_reinit_wavefunction()) {
      if (oprint) {
         coutput << "[PWDFT] Forcing wavefunction reinitialization due to previous failure." << std::endl;
      }
      
      // Force wavefunction reinitialization
      std::string guess = control.initial_wavefunction_guess();
      if (guess == "atomic") {
         if (oprint) coutput << "[PWDFT] Using atomic guess for wavefunction reinitialization." << std::endl;
         mygrid->g_generate_atomic_guess(mysolid.psi1);
      } else if (guess == "superposition") {
         if (oprint) coutput << "[PWDFT] Using superposition guess for wavefunction reinitialization." << std::endl;
         mygrid->g_generate_superposition_guess(mysolid.psi1);
      } else if (guess == "gaussian") {
         if (oprint) coutput << "[PWDFT] Using Gaussian guess for wavefunction reinitialization." << std::endl;
         mygrid->g_generate_gaussian_guess(mysolid.psi1);
      } else if (guess == "mixed") {
         if (oprint) coutput << "[PWDFT] Using mixed guess for wavefunction reinitialization." << std::endl;
         mygrid->g_generate_mixed_guess(mysolid.psi1);
      } else {
         if (oprint) coutput << "[PWDFT] Using random initialization for wavefunction reinitialization." << std::endl;
         mygrid->g_generate_random(mysolid.psi1);
      }
      mysolid.newpsi = true;
      mysolid.clear_force_reinit_wavefunction();
   }

   bool extra_rotate = control.scf_extra_rotate(); 
 
   for (auto ii=0; ii<70; ++ii)
      E[ii] = 0.0;
 
   if (oprint) 
   {
      if (minimizer == 1) coutput << "     ======= bundled Grassmann conjugate gradient iteration =======" << std::endl;
      if (minimizer == 2) coutput << "     =========+=== bundled Grassmann lmbfgs iteration =============" << std::endl;
      if (minimizer == 3) coutput << "     ======== Kohn-Sham scf iteration (Grassmann) iteration =======" << std::endl;
      if (minimizer == 4) coutput << "     ============ Stiefel conjugate gradient iteration ============" << std::endl;
      if (minimizer == 5) coutput << "     ============ Kohn-Sham scf iteration (potential) =============" << std::endl;
      if (minimizer == 6) coutput << "     ========== Kohn-Sham scf iteration (lmbfgs) iteration ========" << std::endl;
      if (minimizer == 7) coutput << "     ================== Stiefel lmbfgs iteration ==================" << std::endl;
      if (minimizer == 8) coutput << "     ============= Kohn-Sham scf iteration (density) ==============" << std::endl;
     
      coutput << "          >>> iteration started at " << util_date() << "  <<<" << std::endl;;
      if (minimizer==3)
      {
         coutput << "     iter.                   energy    delta energy       delta rho       delta scf" << std::endl;
         coutput << "     ------------------------------------------------------------------------------" << std::endl;
      }
      else
      {
         coutput << "     iter.                   energy    delta energy       delta rho" << std::endl;
         coutput << "     --------------------------------------------------------------" << std::endl;
      }
      // printf("%10d%25.12le%16.6le%16.6le\n",1000,99.99, 1.33434340e-4, 2.33434211e-6);

   }
 
   // if (minimizer > 1) band_Grsm_list_start()
   //if ((minimizer == 5) || (minimizer == 8))
   //   it_out = 1;
 
      band_Geodesic12 mygeodesic12(minimizer, &mysolid, control);

   /* generate phase factors and local psp and semicore density */
   mysolid.phafacs_vl_potential_semicore();

   // Allocate persistent buffers for minimization optimization
   mysolid.allocate_persistent_buffers();
 
   // std::cout << "band_cgsd_energy: minimizer = " << minimizer << std::endl;
   deltae = -1.0e-03;
   int bfgscount = 0;
   int icount = 0;
   bool converged = false;
 
   if (minimizer == 1) {
      std::cout << "[ENERGY DEBUG] Starting minimizer == 1 (Grassmann conjugate gradient)" << std::endl;
      
      if (mysolid.newpsi) 
      {
         std::cout << "[ENERGY DEBUG] mysolid.newpsi is true, performing steepest descent iterations" << std::endl;
         int it_in0 = 15;
         for (int it=0; it<it_in0; ++it)
            mysolid.sd_update(dte);
         if (oprint) coutput << "        - " << it_in0 << " steepest descent iterations performed" << std::endl;
      }
      
      std::cout << "[ENERGY DEBUG] About to call mysolid.energy()" << std::endl;
      double ee=mysolid.energy();
      std::cout << "[ENERGY DEBUG] mysolid.energy() completed, ee=" << ee << std::endl;
      
      while ((icount < it_out) && (!converged)) 
      {
         ++icount;
         std::cout << "[ENERGY DEBUG] Starting outer iteration " << icount << std::endl;
         
         if (stalled) 
         {
            std::cout << "[ENERGY DEBUG] stalled is true, performing steepest descent iterations" << std::endl;
            for (int it=0; it<it_in; ++it)
               mysolid.sd_update(dte);
            if (oprint)
               coutput << "        - " << it_in << " steepest descent iterations performed" << std::endl;
            bfgscount = 0;
         }
         
         std::cout << "[ENERGY DEBUG] About to call band_cgsd_cgminimize" << std::endl;
         deltae_old = deltae;
         total_energy = band_cgsd_cgminimize(mysolid,mygeodesic12.mygeodesic1,E,&deltae,
                                             &deltac,bfgscount,it_in,tole,tolc);
         std::cout << "[ENERGY DEBUG] band_cgsd_cgminimize completed, total_energy=" << total_energy << std::endl;
         
         ++bfgscount;
         if (oprint)
           coutput << Ifmt(10) << icount*it_in 
                   << Efmt(25,12) << total_energy
                   << Efmt(16,6) << deltae 
                   << Efmt(16,6) << deltac << std::endl;
         if ((std::fabs(deltae) > fabs(deltae_old)) ||
             (std::fabs(deltae) > 1.0e-2) || (deltae > 0.0))
            stalled = true;
         else
            stalled = false;
         converged = (std::fabs(deltae) < tole) && (deltac < tolc);
      }
 
   } else if (minimizer == 2) {
      if (mysolid.newpsi) {
         int it_in0 = 15;
         for (int it=0; it<it_in0; ++it)
            mysolid.sd_update(dte);
         if (oprint) coutput << "        - " << it_in0 << " steepest descent iterations performed" << std::endl;
      }
      band_lmbfgs psi_lmbfgs(mygeodesic12.mygeodesic1, lmbfgs_size);
      while ((icount < it_out) && (!converged)) {
         ++icount;
         if (stalled) {
            for (int it = 0; it < it_in; ++it)
               mysolid.sd_update(dte);
            if (oprint) coutput << "        - " << it_in << " steepest descent iterations performed" << std::endl;
            bfgscount = 0;
         }
         deltae_old = deltae;
         total_energy = band_cgsd_bfgsminimize(mysolid, mygeodesic12.mygeodesic1, psi_lmbfgs, E,
                                               &deltae, &deltac, bfgscount, it_in, tole, tolc);
         ++bfgscount;
         if (oprint)
            coutput << Ifmt(10) << icount * it_in 
                    << Efmt(25,12) << total_energy
                    << Efmt(16,6) << deltae 
                    << Efmt(16,6) << deltac << std::endl;
         if ((std::fabs(deltae) > fabs(deltae_old)) ||
             (std::fabs(deltae) > 1.0e-2) || (deltae > 0.0))
            stalled = true;
         else
            stalled = false;
         converged = (std::fabs(deltae) < tole) && (deltac < tolc);
      }
   } else if (minimizer == 3) {
      if (mysolid.newpsi)
      {
         int it_in0 = 15;
         for (int it=0; it<it_in0; ++it)
            mysolid.sd_update(dte);
         if (oprint) coutput << "        - " << it_in0 << " steepest descent iterations performed" << std::endl;
      }



      // Initial SCF setup
      // generate density and rotate orbitals 
      // Generate initial density, potential and then orbital diagonalization
      double total_energy0 = 0.0;
      
      // CRITICAL FIX: Proper state initialization for each SCF run
      // This ensures clean state for each call to band_cgsd_energy()
      mysolid.gen_hml();                       // Generate ⟨ψ|H|ψ⟩ (stored in hml)
      mysolid.diagonalize();      // Diagonalize H matrix (sets eig)
      mysolid.rotate1to2();       // Rotate ψ₁ to ψ₂ using eigenvectors
      mysolid.swap_psi1_psi2();   // Swap ψ₂ → ψ₁ (start clean state)
      
      // CRITICAL FIX: Regenerate density and potentials from psi1 before energy calculation
      mysolid.gen_scf_potentials_from_rho1();
      
      // CRITICAL FIX: Calculate energy AFTER wavefunctions and potentials are properly initialized
      total_energy0 = mysolid.energy0(); // Run Hψ = Eψ and compute E[0] - after initialization
      DEBUG_LOG("Initial energy calculation: total_energy0 = " << total_energy0);

      // Normalize total density (diagnostic)
      double x,sumxx = 0.0;
      int nfft3d = mygrid->nfft3d;
      int ispin = mygrid->ispin;
      double omega = mygrid->lattice->omega();
      double scal1 = 1.0 / ((double)((mygrid->nx) * (mygrid->ny) * (mygrid->nz)));
      double dv = omega * scal1;
      for (int i=0; i < nfft3d; ++i)
      {
         x = (mysolid.rho1[i]);
         x += (mysolid.rho1[i+(ispin-1)*nfft3d]);
         sumxx += x;
      }
      double sum0  = mygrid->c3db::parall->SumAll(1, sumxx) * dv;

      //std::cout << "total energy0=" << total_energy0 << std::endl;
      //std::cout << "total sum0=" << sum0 << std::endl;

      // Setup SCF loop
      deltae = -1.0e-03;
      int bfgscount = 0;
      int icount = 0;
      int ks_it_in  = control.ks_maxit_orb();
      int ks_it_out = control.ks_maxit_orbs();
      double scf_error = 0.0;
      double *vout = mysolid.rho1;
      double *vnew = mysolid.rho2;

      // Initialize adaptive diagonalization threshold parameters
      double current_ethr = control.scf_initial_ethr();
      double min_ethr = control.scf_min_ethr();
      double ethr_factor = control.scf_ethr_factor();
      bool adaptive_enabled = control.scf_adaptive_threshold();
      int nelec = mygrid->ne[0] + mygrid->ne[1];

      nwpw_cscf_mixing scfmix(mygrid,kerker_g0,
                             scf_algorithm,scf_alpha,scf_beta,diis_histories,
                             mygrid->ispin,mygrid->nfft3d,vout);

      // CRITICAL FIX: Reset SCF mixing state after wavefunction initialization
      // This prevents stale mixing history from affecting restart calculations
      if (!mysolid.newpsi) {
         scfmix.reset_mix(vout);
         if (oprint) coutput << "        - SCF mixing state reset for restart" << std::endl;
      }

      while ((icount < (it_out*it_in)) && (!converged))
      {
         ++icount;
         
         // Debug prints for inner loop iterations
         if (icount > it_in) {
            DEBUG_LOG("\nInner loop iteration " << icount << " (outer iteration " << (icount/it_in) << ")");
            std::cerr << "  E[0]: " << E[0] << std::endl;
            std::cerr << "  total_energy: " << total_energy << std::endl;
            std::cerr << "  deltae: " << deltae << std::endl;
            std::cerr << "  deltac: " << deltac << std::endl;
            std::cerr << "------------------------------------------------------" << std::endl;
         }
         if (stalled) 
         {
            for (int it=0; it<it_in; ++it)
               mysolid.sd_update(dte);
            if (oprint)
               coutput << "        - " << it_in << " steepest descent iterations performed" << std::endl;
            bfgscount = 0;
         }
         deltae_old = deltae;


         // minimize ks orbitals it_in steps with fixed ks potential
         // Use adaptive threshold for diagonalization if enabled
         double effective_tolc = adaptive_enabled ? current_ethr : tolc;
         double total_energy_fixedV = band_cgsd_cgksminimize(mysolid,mygeodesic12.mygeodesic1,E,&deltae,
                                                             &deltac,bfgscount,it_in,tole,effective_tolc);

         // diagonalize psi wrt current ks potential
         mysolid.gen_hml();
         mysolid.diagonalize();
         mysolid.rotate1to2();
         mysolid.swap_psi1_psi2();

         // Generate updated density from current ψ and occupations, generate ks potential 
         // and then calculate energy, after this rho1, dng1, rho1_all, 
         // and ks potentials and hpsi have been updated
         
         // Debug print before energy calculation
         if (icount > it_in) {
            DEBUG_LOG("Before energy calculation " << (icount/it_in) << ": calling mysolid.energy0()");
         }
         
         // CRITICAL FIX: Use energy0() instead of energy() to avoid state corruption
         // energy0() only updates Hpsi without modifying density/potentials
         total_energy = mysolid.energy0();
         
         // Debug print after energy calculation
         if (icount > it_in) {
            DEBUG_LOG("After energy calculation " << (icount/it_in) << ": total_energy = " << total_energy);
         }
         
         // CRITICAL FIX: Detect and handle unphysical energies
         // This prevents statefulness bugs when using loop command with multiple outer iterations
         if (std::isnan(total_energy) || std::isinf(total_energy) || total_energy > 100.0) {
            DEBUG_LOG("Detected unphysical energy: " << total_energy << ". Recalculating with clean state.");
            
            // Force a clean energy calculation by resetting the system state
            mysolid.gen_scf_potentials_from_rho1();  // Ensure potentials are up to date
            total_energy = mysolid.energy0();  // Recalculate energy with clean state using energy0()
            
            if (std::isnan(total_energy) || std::isinf(total_energy) || total_energy > 100.0) {
               DEBUG_LOG("Energy still unphysical after recalculation: " << total_energy);
               // If still unphysical, this indicates a deeper problem that needs investigation
            }
         }

         // [Insert fractional occupation update here if needed]
         // if (mysolid.fractional) update_occupations(...);


         //std::cout << "total_energy=" << total_energy << " " << total_energy2 
         //          << " " << total_energy2 - total_energy << std::endl;

         //define fractional occupation here
         scfmix.mix(vout,vnew,deltae,&scf_error);
         std::memcpy(vout,vnew,ispin*nfft3d*sizeof(double));
         
        // density now updated, and the ks potentials have to be updated here!?

         deltac = scf_error;

         // Apply adaptive diagonalization threshold adjustment
         if (adaptive_enabled) {
            double dr2 = deltac * deltac; // Use density residual squared
            double new_ethr = pwdft::get_scf_diagonalization_threshold(
                adaptive_enabled, icount, dr2, nelec, 
                current_ethr, control.scf_initial_ethr(), min_ethr, ethr_factor);
            
            if (new_ethr != current_ethr) {
               current_ethr = new_ethr;
               if (oprint) {
                  coutput << "        - Adaptive threshold adjusted to: " << Efmt(12,6) << current_ethr << std::endl;
               }
            }
         }

         converged = (std::fabs(deltae) < tole) && (deltac < tolc);
         //deltac = mysolid.rho_error();
         
         // CRITICAL FIX: Only update total_energy0 on the first iteration of each outer loop
         // This prevents statefulness issues when using loop command with multiple outer iterations
         if (icount <= it_in) {
            deltae = total_energy - total_energy0;
            total_energy0 = total_energy;
         } else {
            // For subsequent outer loop iterations, use the energy from the previous inner loop iteration
            deltae = total_energy - total_energy0;
            // Only update total_energy0 at the end of each outer loop iteration
            if (icount % it_in == 0) {
               total_energy0 = total_energy;
            }
         }
         
         // Debug print for energy difference calculation
         if (icount > it_in) {
            DEBUG_LOG("Energy diff calculation " << (icount/it_in) << ": total_energy = " << total_energy << ", total_energy0 = " << total_energy0 << ", deltae = " << deltae);
         }
         ++bfgscount;

         converged = (std::fabs(deltae) < tole) && (deltac < tolc);

         if ((oprint) && ((icount%it_in==0) || converged))
         {
            coutput << Ifmt(10)    << icount
                    << Efmt(25,12) << total_energy
                    << Efmt(16,6)  << deltae
                    << Efmt(16,6)  << deltac 
                    << Efmt(16,6)  << total_energy-total_energy_fixedV << std::endl;
         }
   
         // Finalize SCF step with updated potentials
         mysolid.gen_scf_potentials_from_rho1();
      }



   } else if (minimizer == 4) {
      if (mysolid.newpsi) {
        int it_in0 = 15;
        for (int it = 0; it < it_in0; ++it)
          mysolid.sd_update_sic(dte);
        if (oprint)
          coutput << "        - " << it_in0
                  << " steepest descent iterations performed" << std::endl;
      }
      while ((icount < it_out) && (!converged)) {
        ++icount;
        if (stalled) {
          for (int it = 0; it < it_in; ++it)
            mysolid.sd_update_sic(dte);
          if (oprint)
            coutput << "        - " << it_in
                      << " steepest descent iterations performed" << std::endl;
          bfgscount = 0;
        }
        deltae_old = deltae;
      //  total_energy =
      //      band_cgsd_cgminimize2(mysolid, mygeodesic12.mygeodesic2, E, &deltae,
      //                            &deltac, bfgscount, it_in, tole, tolc);
        ++bfgscount;
        if (oprint)
          coutput << Ifmt(10) << icount*it_in 
                  << Efmt(25,12) << total_energy
                  << Efmt(16,6) << deltae 
                  << Efmt(16,6) << deltac << std::endl;
        if ((std::fabs(deltae) > fabs(deltae_old)) ||
            (std::fabs(deltae) > 1.0e-2) || (deltae > 0.0))
          stalled = true;
        else
          stalled = false;
        converged = (std::fabs(deltae) < tole) && (deltac < tolc);
      }
   } else if (minimizer == 6) {
   } else if (minimizer == 7) {
      if (mysolid.newpsi) {
         int it_in0 = 15;
         for (int it=0; it<it_in0; ++it)
            mysolid.sd_update_sic(dte);
         if (oprint)
            coutput << "        - " << it_in0 << " steepest descent iterations performed" << std::endl;
      }
      band_lmbfgs2 psi_lmbfgs2(mygeodesic12.mygeodesic2, lmbfgs_size);
      while ((icount < it_out) && (!converged)) {
         ++icount;
         if (stalled) {
            for (int it=0; it<it_in; ++it)
               mysolid.sd_update_sic(dte);
            if (oprint)
               coutput << "        - " << it_in << " steepest descent iterations performed" << std::endl;
            bfgscount = 0;
         }
         deltae_old = deltae;
       //  total_energy = band_cgsd_bfgsminimize2(mysolid,mygeodesic12.mygeodesic2,psi_lmbfgs2,
       //                                         E,&deltae,&deltac,bfgscount,it_in,tole,tolc);
         ++bfgscount;
         if (oprint)
            coutput << Ifmt(10) << icount * it_in 
                    << Efmt(25,12) << total_energy
                    << Efmt(16,6) << deltae 
                    << Efmt(16,6) << deltac << std::endl;
         if ((std::fabs(deltae) > fabs(deltae_old)) ||
             (std::fabs(deltae) > 1.0e-2) || (deltae > 0.0))
            stalled = true;
         else
            stalled = false;
         converged = (std::fabs(deltae) < tole) && (deltac < tolc);
      }
   } else if (minimizer == 8) {

      if (mysolid.newpsi)
      {
         int it_in0 = 15;
         for (int it=0; it<it_in0; ++it)
            mysolid.sd_update(dte);
         if (oprint) coutput << "        - " << it_in0 << " steepest descent iterations performed" << std::endl;
      }

      // Initial SCF setup
      // generate density and rotate orbitals 
      // Generate initial density, potential and then orbital diagonalization
      double total_energy0 = mysolid.energy(); // Run Hψ = Eψ and compute E[0]
      mysolid.gen_hml();                       // Generate ⟨ψ|H|ψ⟩ (stored in hml)
      mysolid.diagonalize();      // Diagonalize H matrix (sets eig)
      mysolid.rotate1to2();       // Rotate ψ₁ to ψ₂ using eigenvectors
      mysolid.swap_psi1_psi2();   // Swap ψ₂ → ψ₁ (start clean state)


      // Fractional occupation: initialize occupation if needed
      // [Insert your occupation initialization routine here if using smeared/fractional occs]

      // Normalize total density (diagnostic)
      double x,sumxx = 0.0;
      int nfft3d = mygrid->nfft3d;
      int ispin = mygrid->ispin;
      double omega = mygrid->lattice->omega();
      double scal1 = 1.0 / ((double)((mygrid->nx) * (mygrid->ny) * (mygrid->nz)));
      double dv = omega * scal1;
      for (int i=0; i < nfft3d; ++i)
      {
         x = (mysolid.rho1[i]);
         x += (mysolid.rho1[i+(ispin-1)*nfft3d]);
         sumxx += x;
      }
      double sum0  = mygrid->c3db::parall->SumAll(1, sumxx) * dv;
      
      //std::cout << "total energy0=" << total_energy0 << std::endl;
      //std::cout << "total sum0=" << sum0 << std::endl;

      // Setup SCF loop
      int ks_it_in  = control.ks_maxit_orb();
      int ks_it_out = control.ks_maxit_orbs();
      double scf_error = 0.0;
      double *vout = mysolid.rho1;
      double *vnew = mysolid.rho2;

      // Initialize adaptive diagonalization threshold parameters
      double current_ethr = control.scf_initial_ethr();
      double min_ethr = control.scf_min_ethr();
      double ethr_factor = control.scf_ethr_factor();
      bool adaptive_enabled = control.scf_adaptive_threshold();
      int nelec = mygrid->ne[0] + mygrid->ne[1];

      nwpw_cscf_mixing scfmix(mygrid,kerker_g0,
                             scf_algorithm,scf_alpha,scf_beta,diis_histories,
                             mygrid->ispin,mygrid->nfft3d,vout);

      // CRITICAL FIX: Reset SCF mixing state after wavefunction initialization
      // This prevents stale mixing history from affecting restart calculations
      if (!mysolid.newpsi) {
         scfmix.reset_mix(vout);
         if (oprint) coutput << "        - SCF mixing state reset for restart" << std::endl;
      }

      while ((icount < (it_out*it_in)) && (!converged))
      {
         ++icount;

         // Restart SCF if stalled
         if ((stalled) && (icount%it_in==0))
         {
            for (int it=0; it<it_in; ++it)
               mysolid.sd_update(dte);

            // rotate orbitals
            if (extra_rotate)
            {
               mysolid.gen_hml();
               mysolid.diagonalize();
               mysolid.rotate1to2();
               mysolid.swap_psi1_psi2();
            }

            // [Optional] Re-initialize fractional occupation
            // define fractional occupation here

            scfmix.reset_mix(vout);

            bfgscount = 0;
            stalled = false;
         }

         deltae_old    = deltae;

         // Copy current density to old
         std::memcpy(vnew,vout,ispin*nfft3d*sizeof(double));

         // Minimize energy wrt orbitals
         // Use adaptive threshold for diagonalization if enabled
         double effective_tolc = adaptive_enabled ? current_ethr : tolc;
         total_energy = band_cgsd_bybminimize2(mysolid,mygeodesic12.mygeodesic1,E,&deltae,
                                          &deltac,bfgscount,ks_it_in,ks_it_out,
                                          tole,effective_tolc);

         // Optional orbital rotation post-minimization
         if (extra_rotate)
         {
            total_energy = mysolid.energy();
            mysolid.gen_hml();
            mysolid.diagonalize();
            mysolid.rotate1to2();
            mysolid.swap_psi1_psi2();
         }

         //  Generate updated density from current ψ
         mysolid.gen_rho1(); // updatating rho1==vout

         // [Insert fractional occupation update here if needed]
         // if (mysolid.fractional) update_occupations(...);


         //std::cout << "total_energy=" << total_energy << " " << total_energy2 
         //          << " " << total_energy2 - total_energy << std::endl;

         //define fractional occupation here
         scfmix.mix(vout,vnew,deltae,&scf_error);
         std::memcpy(vout,vnew,ispin*nfft3d*sizeof(double));

         deltac = scf_error;

         // Apply adaptive diagonalization threshold adjustment
         if (adaptive_enabled) {
            double dr2 = deltac * deltac; // Use density residual squared
            double new_ethr = pwdft::get_scf_diagonalization_threshold(
                adaptive_enabled, icount, dr2, nelec, 
                current_ethr, control.scf_initial_ethr(), min_ethr, ethr_factor);
            
            if (new_ethr != current_ethr) {
               current_ethr = new_ethr;
               if (oprint) {
                  coutput << "        - Adaptive threshold adjusted to: " << Efmt(12,6) << current_ethr << std::endl;
               }
            }
         }

         //deltac = mysolid.rho_error();
         deltae = total_energy - total_energy0;
         total_energy0 = total_energy;
         ++bfgscount;

         converged = (std::fabs(deltae) < tole) && (deltac < tolc);

         if ((oprint) && ((icount%it_in==0) || converged))
         {
            coutput << Ifmt(10)    << icount
                    << Efmt(25,12) << total_energy
                    << Efmt(16,6)  << deltae
                    << Efmt(16,6)  << deltac << std::endl;
         }

         // Finalize SCF step with updated potentials
         mysolid.gen_scf_potentials_from_rho1();

      }
   }

   else if (minimizer == 9)
   {
   }

   else if (minimizer == 10)
   {
   }

 
   if (oprint) 
   {
      if (converged)  coutput << "     *** tolerance ok. iteration terminated" << std::endl;
      if (!converged) coutput << "     *** arrived at the Maximum iteration.  terminated" << std::endl;
      coutput << "          >>> iteration ended at   " << util_date() << "  <<<" << std::endl;
   }
 
   /* report summary of results */
   total_energy  = mysolid.gen_all_energies();
   if (oprint) 
   {
      coutput << mysolid;
      coutput << std::endl;
   }
   coutput << mysolid.print_filled(oprint);;
   //coutput << mysolid(oprint);
 
   // Deallocate persistent buffers for minimization optimization
   mysolid.deallocate_persistent_buffers();
 
   return total_energy;
}

/******************************************
 *                                        *
 *         band_cgsd_energy_gradient      *
 *                                        *
 ******************************************/
void band_cgsd_energy_gradient(Solid &mysolid, double *grad_ion) 
{

   mysolid.psi_1local_force(grad_ion);
   mysolid.psi_1nonlocal_force(grad_ion);
   mysolid.semicore_force(grad_ion);
 
   mysolid.ion_fion(grad_ion);
}

} // namespace pwdft

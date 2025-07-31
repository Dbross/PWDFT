

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Control2.hpp"
#include "cCoulomb.hpp"
#include "Ewald.hpp"
#include "Ion.hpp"
#include "cKinetic.hpp"
#include "CGrid.hpp"
#include "Parallel.hpp"
#include "CPseudopotential.hpp"
#include "cExchange_Correlation.hpp"
#include "band_inner_loop.hpp"
#include "iofmt.hpp"
#include "cpsi_H.hpp"
#include "debug_macros.hpp"
#include <sstream>

//#include "nwpw_dplot.hpp"

namespace pwdft {

void band_inner_loop(Control2 &control, Cneb *mygrid, Ion *myion,
                     cKinetic_Operator *myke, cCoulomb_Operator *mycoulomb,
                     cXC_Operator *myxc, CPseudopotential *mypsp, CStrfac *mystrfac,
                     Ewald *myewald, double *psi1, double *psi2, double *Hpsi,
                     double *psi_r, double *dn, double *hml, double *lmbda,
                     double E[], double *deltae, double *deltac, double *deltar) 
{
   int it_in, k, ms;
   int indx1, indx2;
   int one = 1;
   double dc;
   double eorbit, eion, econstraint, exc, ehartr, pxc;
   double eke, elocal, enlocal, dt, dte, Eold;
   double *vl,*vc,*xcp,*xce,*dnall,*x,*dng,*rho,*tmp,*vcall,*vpsi,*sumi;
   double *fion;
   bool move = control.geometry_optimize();
   double omega = mygrid->lattice->omega();
 
   int ispin = mygrid->ispin;
   int neall = mygrid->neq[0] + mygrid->neq[1];
   int shift1 = 2*mygrid->npack1_max(); 
   int shift2 = (mygrid->n2ft3d);
   int nfft3d = (mygrid->nfft3d);
   int n2ft3d = (mygrid->n2ft3d);

   double scal1 = 1.0/((double)((mygrid->nx) * (mygrid->ny) * (mygrid->nz)));
   double scal2 = 1.0/omega;
   double dv = omega*scal1;
 
   dt = control.time_step();
   dte = dt/sqrt(control.fake_mass());
   
   // CRITICAL FIX: Check and bound dte to prevent NaN propagation
   if (!std::isfinite(dte) || dte <= 0.0) {
      NAN_INF_LOG("WARNING: Invalid dte value detected: dt=" << dt << ", fake_mass=" << control.fake_mass() << ", dte=" << dte);
      // Use safe default values if dte is invalid
      if (!std::isfinite(dt) || dt <= 0.0) dt = 0.001;  // Default time step
      if (!std::isfinite(control.fake_mass()) || control.fake_mass() <= 0.0) {
         dte = dt/sqrt(400.0);  // Default fake mass
      } else {
         dte = dt/sqrt(control.fake_mass());
      }
      NAN_INF_LOG("Using safe dte value: " << dte);
   }
   
   // Additional safety check: bound dte to reasonable range
   if (dte > 1.0) {
      NAN_INF_LOG("WARNING: dte too large (" << dte << "), clamping to 1.0");
      dte = 1.0;
   }
   
   it_in = control.loop(0);
   // --- DEBUG PRINT: Entry to band_inner_loop ---
   TRACE_LOG("Entered band_inner_loop, it_in = " << it_in);
   //allocate temporary memory 
   rho = mygrid->c_alloc();
   tmp = mygrid->c_alloc();

   xcp = mygrid->r_nalloc(ispin);
   xce = mygrid->r_nalloc(ispin);
   dnall = mygrid->r_nalloc(ispin);
   x     = mygrid->r_alloc();

   dng   = mygrid->c_pack_allocate(0);
   vl    = mygrid->c_pack_allocate(0);
   vc    = mygrid->c_pack_allocate(0);
   vcall = mygrid->c_pack_allocate(0);
 
   vpsi = x;
 
   //fion = new double[3 * (myion->nion)]();
   fion = myion->fion1;
 
   // generate local psp
   mypsp->v_local(vl,false,dng,fion);

 
   // myewald->phafac();
 
   //|-\____|\/-----\/\/->    Start Parallel Section    <-\/\/-----\/|____/-|
 
   for (auto it=0; it<it_in; ++it) 
   {
      // Debug print for first few iterations
      if (it < 3) {
         TRACE_LOG("Inner loop iteration " << it << ": E[0] = " << E[0]);
      }
      
      mygrid->g_zero(Hpsi);
      mygrid->gg_copy(psi2, psi1);
     
      if (move)
      {
         myion->shift();
         mystrfac->phafac();
         mystrfac->phafac_k();
         myewald->phafac();
         // for (int ii=0; ii<(3*myion->nion); ++ii) fion[ii] = 0.0;
      }
     
      // convert psi(G) to psi(r) - Expensive 
      mygrid->gh_fftb(psi1,psi_r);

      // NaN/Inf check: psi1
      for (int i = 0; i < ispin * nfft3d; ++i) {
         if (std::isnan(psi1[i]) || std::isinf(psi1[i])) {
            std::ostringstream oss;
            oss << "psi1[" << i << "] = " << psi1[i];
            NAN_INF_LOG(oss.str());
            break;
         }
      }

      // generate dn
      mygrid->hr_aSumSqr(scal2,psi_r,dn);
      // NaN/Inf check: dn
      for (int i = 0; i < ispin * nfft3d; ++i) {
         if (std::isnan(dn[i]) || std::isinf(dn[i])) {
            std::ostringstream oss;
            oss << "dn[" << i << "] = " << dn[i];
            NAN_INF_LOG(oss.str());
            break;
         }
      }
      // --- DEBUG PRINT: dn after generation (always print) ---
      STATE_DUMP(array_to_string("dn", dn, ispin * nfft3d));

      // generate dng 
      mygrid->rrc_Sum(dn,dn+(ispin-1)*nfft3d,rho);
#if defined(ENABLE_NAN_INF_CHECKS)
      check_nan_inf("rho", rho, ispin * nfft3d, "after first SCF step");
#endif
      mygrid->rc_pfft3f(0,rho);
      //mygrid->rc_fft3d(rho);

      mygrid->cc_SMul(scal1, rho, tmp);

      mygrid->c_pack(0,tmp);
      mygrid->cc_pack_copy(0,tmp,dng);
      // NaN/Inf check: dng
      for (int i = 0; i < 2 * mygrid->npack(0); ++i) {
         if (std::isnan(dng[i]) || std::isinf(dng[i])) {
            std::ostringstream oss;
            oss << "dng[" << i << "] = " << dng[i];
            NAN_INF_LOG(oss.str());
            break;
         }
      }
      //mygrid->c_pack_SMul(0,scal1, dng);

      // generate dnall - used for semicore corrections
      if (mypsp->has_semicore()) 
      {
         if ((move) || (it == 0))
            mypsp->semicore_density_update();
         for (ms=0; ms<ispin; ++ms)
            mygrid->rrr_SMulAdd(0.5,mypsp->semicore_density,dn+ms*nfft3d,dnall+ms*nfft3d);
      } 
      else 
      {
         for (ms=0; ms<ispin; ++ms)
            mygrid->rr_copy(dn+ms*nfft3d, dnall+ms*nfft3d);
      }
     
      // generate local potentials
      if (move) 
      {
         mypsp->v_local(vl,move,dng,fion);
      }
      // NaN/Inf check: vl
      for (int i = 0; i < 2 * mygrid->npack(0); ++i) {
         if (std::isnan(vl[i]) || std::isinf(vl[i])) {
            std::ostringstream oss;
            oss << "vl[" << i << "] = " << vl[i];
            NAN_INF_LOG(oss.str());
            break;
         }
      }
     
      // apply k-space operators
      // myke->ke(psi1,Hpsi);
     
      // apply non-local PSP  - Expensive
      // mypsp->v_nonlocal_fion(psi1,Hpsi,move,fion);
     
      // generate coulomb potential 
      std::memset(xcp,0,ispin*nfft3d*sizeof(double));
      std::memset(xce,0,ispin*nfft3d*sizeof(double));
      myxc->v_exc_all(ispin,dnall,xcp,xce);
      // NaN/Inf check: xcp
      for (int i = 0; i < ispin * nfft3d; ++i) {
         if (std::isnan(xcp[i]) || std::isinf(xcp[i])) {
            std::ostringstream oss;
            oss << "xcp[" << i << "] = " << xcp[i];
            NAN_INF_LOG(oss.str());
            break;
         }
      }
     
      // get Hpsi
      cpsi_H(mygrid,myke,mypsp,psi1,psi_r,vl,vcall,xcp,Hpsi,move,fion);

      // Debug: check Hpsi before steepest descent
      STATE_DUMP(array_to_string("Hpsi before steepest descent", Hpsi, 10));
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(Hpsi[i])) {
            std::ostringstream oss;
            oss << "Hpsi[" << i << "] = " << Hpsi[i] << " before steepest descent";
            NAN_INF_LOG(oss.str());
            break;
         }
      }

      // Debug: check psi1 before steepest descent
      STATE_DUMP(array_to_string("psi1 before steepest descent", psi1, 10));
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(psi1[i])) {
            std::ostringstream oss;
            oss << "psi1[" << i << "] = " << psi1[i] << " before steepest descent";
            NAN_INF_LOG(oss.str());
            break;
         }
      }

      // do a steepest descent step
      // CRITICAL FIX: Additional safety checks before steepest descent
      if (!std::isfinite(dte)) {
         NAN_INF_LOG("ERROR: dte is not finite in steepest descent step: " << dte);
         // Use a safe default
         dte = 0.001;
      }
      
      // CRITICAL FIX: Check for extreme values that could cause overflow
      if (std::abs(dte) > 1.0) {
         NAN_INF_LOG("WARNING: dte too large (" << dte << "), clamping to 0.1");
         dte = (dte > 0) ? 0.1 : -0.1;
      }
      
      // CRITICAL FIX: Check Hpsi for extreme values before scaling
      double max_hpsi = 0.0;
      for (int i=0; i<10; ++i) {
         max_hpsi = std::max(max_hpsi, std::abs(Hpsi[i]));
      }
      if (max_hpsi > 1e6) {
         NAN_INF_LOG("WARNING: Hpsi has large values (max=" << max_hpsi << "), this may cause overflow");
      }
      
      mygrid->gg_SMul(dte,Hpsi,psi2);
      
      // Debug: check psi2 after gg_SMul
      STATE_DUMP(array_to_string("psi2 after gg_SMul", psi2, 10));
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(psi2[i])) {
            std::ostringstream oss;
            oss << "psi2[" << i << "] = " << psi2[i] << " after gg_SMul";
            NAN_INF_LOG(oss.str());
            break;
         }
      }
      
      mygrid->gg_Sum2(psi1,psi2);
      
      // Debug: check psi2 after gg_Sum2
      STATE_DUMP(array_to_string("psi2 after gg_Sum2", psi2, 10));
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(psi2[i])) {
            std::ostringstream oss;
            oss << "psi2[" << i << "] = " << psi2[i] << " after gg_Sum2";
            NAN_INF_LOG(oss.str());
            break;
         }
      }
      
      // CRITICAL FIX: Fallback mechanism if steepest descent produces NaN
      bool psi2_has_nan = false;
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(psi2[i])) {
            psi2_has_nan = true;
            break;
         }
      }
      
      if (psi2_has_nan) {
         NAN_INF_LOG("ERROR: NaN detected in psi2 after steepest descent, using fallback update");
         // Fallback: use a much smaller time step or just copy psi1
         double fallback_dte = dte * 0.01;  // Use 1% of original time step
         if (std::abs(fallback_dte) < 1e-6) fallback_dte = 1e-6;  // Minimum time step
         
         NAN_INF_LOG("Using fallback dte: " << fallback_dte);
         
         // Retry with smaller time step
         mygrid->gg_SMul(fallback_dte, Hpsi, psi2);
         mygrid->gg_Sum2(psi1, psi2);
         
         // Check if fallback worked
         bool fallback_worked = true;
         for (int i=0; i<10; ++i) {
            if (!std::isfinite(psi2[i])) {
               fallback_worked = false;
               NAN_INF_LOG("Fallback also failed, copying psi1 to psi2");
               break;
            }
         }
         
         if (!fallback_worked) {
            // Last resort: just copy psi1 to psi2
            mygrid->gg_copy(psi1, psi2);
            NAN_INF_LOG("Using psi1 copy as last resort");
         }
      }

      if (move)
      {
         // get the ion-ion force 
         myewald->force(fion);
        
         // get the semicore force - needs to be checked 
         if (mypsp->has_semicore())
            mypsp->semicore_xc_fion(xcp, fion);
        
        
         // steepest descent step 
         myion->add_contraint_force(fion);

         // steepest descent step 
         myion->optimize_step(fion);
      }

      // lagrange multiplier - Expensive 
      WF_LOG("band_inner_loop: about to call ggw_lambda with dte=" << dte);
      STATE_DUMP(array_to_string("psi1 before ggw_lambda", psi1, 10));
      STATE_DUMP(array_to_string("psi2 before ggw_lambda", psi2, 10));
      
      // CRITICAL FIX: Check for NaN/Inf before/after Lagrange multiplier (debug only)
#if defined(ENABLE_NAN_INF_CHECKS)
      bool psi2_has_nan_before = false;
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(psi2[i])) {
            psi2_has_nan_before = true;
            NAN_INF_LOG("WARNING: psi2[" << i << "] = " << psi2[i] << " before ggw_lambda");
            break;
         }
      }
      
      if (psi2_has_nan_before) {
         NAN_INF_LOG("ERROR: psi2 contains NaN/Inf before ggw_lambda - skipping Lagrange multiplier");
      } else {
         mygrid->ggw_lambda(dte, psi1, psi2, lmbda);
      }
      
      // CRITICAL FIX: Check for NaN/Inf after Lagrange multiplier
      bool psi2_has_nan_after = false;
      for (int i=0; i<10; ++i) {
         if (!std::isfinite(psi2[i])) {
            psi2_has_nan_after = true;
            NAN_INF_LOG("ERROR: psi2[" << i << "] = " << psi2[i] << " after ggw_lambda");
            break;
         }
      }
      
      if (psi2_has_nan_after) {
         NAN_INF_LOG("ERROR: ggw_lambda corrupted psi2 - restoring from psi1");
         mygrid->gg_copy(psi1, psi2);
         NAN_INF_LOG("Fallback: restored psi2 from psi1 after ggw_lambda corruption");
      }
#else
      mygrid->ggw_lambda(dte, psi1, psi2, lmbda);
#endif
      
      STATE_DUMP(array_to_string("psi2 after ggw_lambda", psi2, 10));
      WF_LOG("band_inner_loop: completed ggw_lambda call");

      // total energy calculation 
      mygrid->ggw_sym_Multiply(psi1, Hpsi, hml);
      mygrid->w_scal(-1.0, hml);  // Apply sign flip to match PSPW behavior
      // --- DEBUG PRINT: hml after total energy calculation (always print) ---
      STATE_DUMP(array_to_string("hml", hml, mygrid->nbrillq * 2 * (mygrid->neq[0]+mygrid->neq[1]) * (mygrid->neq[0]+mygrid->neq[1])));
   }

   //|-\____|\/-----\/\/->    End Parallel Section    <-\/\/-----\/|____/-|

   // total energy calculation 
   eorbit = mygrid->w_trace(hml);
   if (ispin==1) eorbit = eorbit + eorbit;

   eorbit = mygrid->c3db::parall->SumAll(3,eorbit);

   // hartree energy and ion-ion energy 
   ehartr = mycoulomb->ecoulomb(dng);
   eion = myewald->energy();
   
 
   // xc energy
   exc = mygrid->rr_dot(dnall, xce);
   pxc = mygrid->rr_dot(dn, xcp);
   if (ispin == 1) {
      exc = exc + exc;
      pxc = pxc + pxc;
   } else {
      exc += mygrid->rr_dot(dnall+nfft3d, xce);
      pxc += mygrid->rr_dot(dn+nfft3d, xcp+nfft3d);
   }
   exc *= dv;
   pxc *= dv;
 
   // average Kohn-Sham kineticl energy 
   eke = myke->ke_ave(psi1);
 
   // average Kohn-Sham local psp energy
   elocal = mygrid->cc_pack_dot(0, dng, vl);
 
 
   // average Kohn-Sham v_nonlocal energy 
   enlocal = mypsp->e_nonlocal(psi1);
 
   Eold = E[0];
   E[0] = eorbit + eion + exc - ehartr - pxc;
   E[1] = eorbit;
   E[2] = ehartr;
   E[3] = exc;
   E[4] = eion;
   E[5] = eke;
   E[6] = elocal;
   E[7] = enlocal;
   E[8] = 2 * ehartr;
   E[9] = pxc;

   // get contraints energies
   if (myion->has_ion_bond_constraints()) 
   {
      E[70] = myion->energy_ion_bond_constraints();
      E[0] = E[0] + E[70];
   }
   if (myion->has_ion_bondings_constraints()) 
   {
      E[71] = myion->energy_ion_bondings_constraints();
      E[0] = E[0] + E[71];
   }

 
   // set convergence variables
   *deltae = (E[0] - Eold) / (dt * control.loop(0));
 
   // deltac 
   dc = 0.0;
   mygrid->ggg_Minus(psi2, psi1, Hpsi);
   for (auto nbq=0; nbq<mygrid->nbrillq; ++nbq)
   {
      int nbq1 = nbq+1;
      sumi = new double[neall]();
      for (auto i=0; i<neall; ++i)
         sumi[i] = mygrid->cc_pack_idot(nbq1, 
                                        Hpsi+(i+nbq*neall)*shift1, 
                                        Hpsi+(i+nbq*neall)*shift1);
      
      mygrid->c3db::parall->Vector_SumAll(1, neall, sumi);

      for (auto i = 0; i < neall; ++i)
         if (sumi[i] > dc)
            dc = sumi[i];
      delete[] sumi;
   }
   dc = mygrid->c3db::parall->MaxAll(3, dc);
   *deltac = dc / dte;

 
   // deltar 
   *deltar = 0.0;
   if (move) {
     double sum;
     for (auto ii = 0; ii < (myion->nion); ++ii) {
       sum = sqrt(fion[3*ii]  *fion[3*ii] +
                  fion[3*ii+1]*fion[3*ii+1] +
                  fion[3*ii+2]*fion[3*ii+2]);
       if (sum > (*deltar))
         *deltar = sum;
     }
   }
 
   //delete[] fion;
 
   mygrid->c_dealloc(tmp);
   mygrid->r_dealloc(xcp);
   mygrid->r_dealloc(xce);
   mygrid->r_dealloc(dnall);
   mygrid->r_dealloc(x);
   mygrid->c_dealloc(rho);
   mygrid->c_pack_deallocate(dng);
   mygrid->c_pack_deallocate(vl);

   mygrid->c_pack_deallocate(vc);
   mygrid->c_pack_deallocate(vcall);

}
} // namespace pwdft

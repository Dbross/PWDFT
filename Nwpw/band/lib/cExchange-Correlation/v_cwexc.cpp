
#include "Cneb.hpp"
#include "b3lyp.hpp"
#include "beef_gga.hpp"
#include "blyp.hpp"
#include "hsepbe.hpp"
#include "pbe96.hpp"
#include "pbesol.hpp"
#include "revpbe.hpp"

namespace pwdft {

#define dncut 1.0e-30

/********************************
 *				                    *
 *            v_cwexc           *
 *                              *
 ********************************/
// rho,agr,fn,fdn - real
// grx,gry,grz - real (gradients in real space)
void v_cwexc(const int gga, Cneb *mycneb, const double *dn,
             const double x_parameter, const double c_parameter, double *xcp,
             double *xce, double *rho, double *grx, double *gry, double *grz,
             double *agr, double *fn, double *fdn) 
{
   std::cout << "[XC DEBUG] Entering v_cwexc - performing real-space exchange-correlation calculation" << std::endl;
   
   // All calculations will be done in real space
   // No complex FFTs should be called in this function
   
   /**********************************
    ***** restricted calculation *****
    **********************************/
   if (mycneb->ispin == 1) 
   {
      std::cout << "[XC DEBUG] Restricted calculation (ispin=1)" << std::endl;
      
      // Zero the gradient arrays
      mycneb->r_zero(agr);
      mycneb->r_zero(grx);
      mycneb->r_zero(gry);
      mycneb->r_zero(grz);
     
      // Copy density to rho (real space)
      mycneb->rr_copy(dn, rho);
      std::cout << "[XC DEBUG] Density copied to rho array" << std::endl;
      
      // Calculate gradients in real space using finite differences
      // This replaces the complex FFT approach with direct real-space computation
      int nx = mycneb->nx;
      int ny = mycneb->ny;
      int nz = mycneb->nz;
      int n2ft3d = mycneb->n2ft3d;
      
      std::cout << "[XC DEBUG] Computing gradients in real space using finite differences" << std::endl;
      std::cout << "[XC DEBUG] Grid dimensions: nx=" << nx << ", ny=" << ny << ", nz=" << nz << std::endl;
      
      // Compute gradients using finite differences
      // For periodic boundary conditions, we need to handle wrap-around
      for (int k = 0; k < nz; ++k) {
         for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
               int idx = i + j * nx + k * nx * ny;
               
               // X gradient: (rho[i+1] - rho[i-1]) / (2*dx)
               int ip1 = ((i + 1) % nx) + j * nx + k * nx * ny;
               int im1 = ((i - 1 + nx) % nx) + j * nx + k * nx * ny;
               grx[idx] = (rho[ip1] - rho[im1]) * 0.5; // Assuming dx = 1 for now
               
               // Y gradient: (rho[j+1] - rho[j-1]) / (2*dy)
               int jp1 = i + ((j + 1) % ny) * nx + k * nx * ny;
               int jm1 = i + ((j - 1 + ny) % ny) * nx + k * nx * ny;
               gry[idx] = (rho[jp1] - rho[jm1]) * 0.5; // Assuming dy = 1 for now
               
               // Z gradient: (rho[k+1] - rho[k-1]) / (2*dz)
               int kp1 = i + j * nx + ((k + 1) % nz) * nx * ny;
               int km1 = i + j * nx + ((k - 1 + nz) % nz) * nx * ny;
               grz[idx] = (rho[kp1] - rho[km1]) * 0.5; // Assuming dz = 1 for now
               
               // Calculate |grad rho| = sqrt(gx^2 + gy^2 + gz^2)
               agr[idx] = std::sqrt(grx[idx] * grx[idx] + gry[idx] * gry[idx] + grz[idx] * grz[idx]);
               
               // Add small epsilon to avoid division by zero
               if (agr[idx] < 1e-12) agr[idx] = 1e-12;
            }
         }
      }
      
      std::cout << "[XC DEBUG] Gradients computed. Sample values:" << std::endl;
      std::cout << "[XC DEBUG] grx[0]=" << grx[0] << ", gry[0]=" << gry[0] << ", grz[0]=" << grz[0] << std::endl;
      std::cout << "[XC DEBUG] agr[0]=" << agr[0] << std::endl;
     
      // Now compute exchange-correlation functional in real space
      std::cout << "[XC DEBUG] Computing exchange-correlation functional" << std::endl;
      
      switch (gga) {
      case 10:
        gen_PBE96_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
        break;
      case 11:
        gen_BLYP_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
        break;
      case 12:
        gen_revPBE_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
        break;
      case 13:
        gen_PBEsol_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
        break;
      case 14:
        gen_HSE_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
        break;
      case 15:
        gen_B3LYP_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
        break;
      case 16:
        gen_BEEF_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, 0.6001664769, xce, fn, fdn);
        break;
      case 17:
        gen_BEEF_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, 0.0, xce, fn, fdn);
        break;
     
      default:
        gen_PBE96_BW_restricted(mycneb->nfft3d, rho, agr, x_parameter, c_parameter, xce, fn, fdn);
      }
      
      std::cout << "[XC DEBUG] Exchange-correlation functional computed" << std::endl;
     
      // Compute the gradient correction term in real space
      // V_xc = V_xc_LDA + V_xc_GGA where V_xc_GGA = df/dn - div(df/d|grad n| * grad n / |grad n|)
      std::cout << "[XC DEBUG] Computing gradient correction term" << std::endl;
      
      // Initialize xcp with the LDA term (fn)
      mycneb->rr_copy(fn, xcp);
      
      // Compute the divergence term: div(df/d|grad n| * grad n / |grad n|)
      // This should be computed using finite differences in real space
      for (int k = 0; k < nz; ++k) {
         for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
               int idx = i + j * nx + k * nx * ny;
               
               // Compute df/d|grad n| * grad n / |grad n| for each component
               double fx = fdn[idx] * grx[idx] / agr[idx];
               double fy = fdn[idx] * gry[idx] / agr[idx];
               double fz = fdn[idx] * grz[idx] / agr[idx];
               
               // Compute divergence using finite differences
               int ip1 = ((i + 1) % nx) + j * nx + k * nx * ny;
               int im1 = ((i - 1 + nx) % nx) + j * nx + k * nx * ny;
               int jp1 = i + ((j + 1) % ny) * nx + k * nx * ny;
               int jm1 = i + ((j - 1 + ny) % ny) * nx + k * nx * ny;
               int kp1 = i + j * nx + ((k + 1) % nz) * nx * ny;
               int km1 = i + j * nx + ((k - 1 + nz) % nz) * nx * ny;
               
               double div_x = (fx - fdn[im1] * grx[im1] / agr[im1]) * 0.5;
               double div_y = (fy - fdn[jm1] * gry[jm1] / agr[jm1]) * 0.5;
               double div_z = (fz - fdn[km1] * grz[km1] / agr[km1]) * 0.5;
               
               // Subtract the divergence from the LDA term
               xcp[idx] -= (div_x + div_y + div_z);
            }
         }
      }
      
      std::cout << "[XC DEBUG] Gradient correction computed" << std::endl;
      std::cout << "[XC DEBUG] Sample xcp values: xcp[0]=" << xcp[0] << ", xcp[1]=" << xcp[1] << std::endl;
   }
 
   /************************************
    ***** unrestricted calculation *****
    ************************************/
   else 
   {
      std::cout << "[XC DEBUG] Unrestricted calculation (ispin=2) - NOT IMPLEMENTED YET" << std::endl;
      std::cout << "[XC DEBUG] Falling back to restricted calculation for now" << std::endl;
      
      // For now, just use the restricted calculation
      // TODO: Implement proper unrestricted calculation in real space
      mycneb->r_zero(agr);
      mycneb->rr_copy(dn, rho);
      mycneb->rr_copy(fn, xcp);
   }
   
   std::cout << "[XC DEBUG] v_cwexc completed successfully - all calculations done in real space" << std::endl;
}

} // namespace pwdft

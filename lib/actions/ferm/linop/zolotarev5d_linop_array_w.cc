/* $Id: zolotarev5d_linop_array_w.cc,v 1.3 2004-07-28 02:38:02 edwards Exp $
/*! \file
 *  \brief Unpreconditioned extended-Overlap (5D) (Naryanan&Neuberger) linear operator
 */

#include "chromabase.h"
#include "linearop.h"

#include "actions/ferm/linop/zolotarev5d_linop_array_w.h"


//! Apply the operator onto a source vector
/*!
 * The operator acts on the entire lattice
 *
 * \param psi 	  Pseudofermion field     	       (Read)
 * \param isign   Flag ( PLUS | MINUS )   	       (Read)
 */
void
Zolotarev5DLinOpArray::operator() (multi1d<LatticeFermion>& chi,
				   const multi1d<LatticeFermion>& psi, 
				   enum PlusMinus isign) const
{
  START_CODE();


  int G5 = Ns*Ns - 1;

  // This is the upper limit for the index of the 5th dimension, i.e.
  //  it runs from 0 to TwoN. (altogether TwoN + 1 numbers. This
  // makes sense since N5 is meant to be odd)
  int TwoN = N5 - 1;

  // This is our scaling: we evaluate 
  //  ( 1 + m_q ) / ( 1 - m_q ) gamma5 + eps
  //
  Real mass = ( Real(1) + m_q ) / (Real(1) - m_q);

  // The sign of the diagonal terms.
  int Hsign;
  
  if( N5%2 == 0 ) {
    Hsign = 1;
  }
  else {
    Hsign = -1;
  }

  // Run through all the pseudofermion fields:
  //   chi(0) = beta(0)*H*psi(0) + alpha(0)*psi(1)
  //   chi(n) = alpha(n-1)*psi(n-1)
  //              + (-)^n*beta(n)*H*psi(n) + alpha(n)*psi(n+1)
  //   chi(TwoN) = alpha(TwoN-1)*psi(TwoN-1)
  //                      + (-)^TwoN*beta(TwoN)*H*psi(TwoN)         
    
  LatticeFermion tmp1, tmp2;
  Real pmscale;

  for(int n = 0; n < TwoN; ++n) {
    (*M)(tmp1, psi[n], PLUS);      // tmp1 = M psi[n]
    tmp2 = Gamma(G5)*tmp1;        // tmp2 = gamma_5 M psi

    // Scale factor and sign for the diagonal term proportional to H
    // The scale factor should be chosen in conszolotarev5d_w.m such
    //  that scale_fac * gamma5 * M has eigenvalues between -1 and 1 
    Hsign = -Hsign;
    pmscale = beta[n]*Hsign*scale_fac;
    
    chi[n] = pmscale*tmp2;
    chi[n] += alpha[n] * psi[n+1];
    
    if( n > 0 ) {
      chi[n] += alpha[n-1]*psi[n-1];
    }
  }

  // Last Component
  // chi[N] = mass*gamma5*psi[N] + alpha[N-1]*psi[N-1]
  tmp1 = Gamma(G5)*psi[TwoN];
  chi[TwoN] = mass*tmp1;;
  chi[TwoN] += alpha[TwoN-1]*psi[ TwoN -1 ];

  // Project out eigenvectors from Source if desired 
  //
  // (What does this do then? Urs????)
  LatticeFermion psi_proj;
  psi_proj = zero;

  if(  NEig > 0 ) {
    Complex cconsts;
    for(int i = 0 ; i < NEig; i++) {
      
      // Project out eigenvectors in psi_{2N} */
      cconsts = innerProduct(EigVec[i],psi[TwoN]);
      psi_proj -= EigVec[i]*cconsts;

      // The vectors are added into chi_2N
      chi[TwoN] += cconsts*EigValFunc[i]*EigVec[i];

      // Project out the eigenvectors from psi_{2N-1} and subtract from chi[N]
      cconsts = innerProduct(EigVec[i],psi[TwoN-1]);
      chi[TwoN] -= alpha[TwoN-1]*cconsts*EigVec[i];
    }

    // Subtract out projected psi_{N} from chi_{N-1} */
    chi[TwoN-1] += psi_proj * alpha[TwoN-1];
  }
                            
  // Complete the last component
  // chi(N) = chi(N) + beta_{N}*gamma_5*M*psi_{N}
  // The other two contributions
  //     chi[N] = mass * gamma[5]*psi[N] + alpha[N-1] * psi[N-1]
  // have been already calculated above 
   
  //  The contribution beta_{N}*gamma_5*M*psi_{N} is
  //   caluclated only if betaa_{N} != 0 */


  if( toBool( beta[TwoN] != Real(0) ) ) {
    
    // Complete psi_proj
    psi_proj += psi[TwoN];

    (*M)(tmp1, psi_proj, PLUS);
    pmscale = beta[TwoN]*scale_fac;
    tmp2 = Gamma(G5)*tmp1;
    chi[TwoN] += pmscale*tmp2;

  }

  END_CODE();
}


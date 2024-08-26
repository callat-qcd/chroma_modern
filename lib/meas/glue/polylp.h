// -*- C++ -*-
/*! \file
 *  \brief Calculate the global normalized sum of the Polyakov loop
 */

#ifndef __polylp_h__
#define __polylp_h__

namespace Chroma 
{

  //! Compute Polyakov loop
  /*!
   * \ingroup glue
   *
   * \param u          gauge field (Read)
   * \param poly_loop  Polyakov loop average in direction mu (Write) 
   * \param mu         direction of Polyakov loop (Read)
   */

#if ! defined (QDP_IS_QDPJIT2)
  void polylp(const multi1d<LatticeColorMatrixF3>& u, DComplex& poly_loop, int mu);
  void polylp(const multi1d<LatticeColorMatrixD3>& u, DComplex& poly_loop, int mu);
#else
  void polylp(const multi1d<LatticeColorMatrix>& u, DComplex& poly_loop, int mu);
#endif
  
  //! Compute Polyakov loop
  /*!
   * \ingroup glue
   *
   * \param u          gauge field (Read)
   * \param poly_loop  Polyakov loop average (Write) 
   */

#if ! defined (QDP_IS_QDPJIT2)
  void polylp(const multi1d<LatticeColorMatrixF3>& u, multi1d<DComplex>& poly_loop);
  void polylp(const multi1d<LatticeColorMatrixD3>& u, multi1d<DComplex>& poly_loop);
#else
  void polylp(const multi1d<LatticeColorMatrix>& u, multi1d<DComplex>& poly_loop);
#endif
  
}  // end namespace Chroma

#endif

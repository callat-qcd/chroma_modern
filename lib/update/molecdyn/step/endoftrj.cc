// $Id: endoftrj.cc,v 1.3 2004-07-28 02:38:05 edwards Exp $

#error "NOT FULLY CONVERTED - NEED TO MOVE AlgETrj into params of Integ. functor"

#include "chromabase.h"

using namespace QDP;

//! Test for end of a trajectory
/*!
 * \ingroup molecdyn
 *
 * \param tau0   desired length of trajectory ( Read )
 * \param t      integration time so far ( Read )
 * \return EndP  flag indicating end of trajectory ( Write ) 
 */

bool EndOfTrj(const Real& tau0, const Real& t)
{
  Real r;
  
  START_CODE();
  
  switch (AlgETrj)
  {
  case FIXED_LENGTH:
    EndP = ( toBool(t > 0.5*(tau0+dt)) ) ? true : false;
    break;

  case EXPONENTIAL_LENGTH:
    random(r);
    EndP = ( toBool(r < dt/max(tau0-dt,dt)) ) ? true : false;
    break;

  default:
    QDP_error_exit("unknown algorithm termination", AlgETrj);
  }
  
  END_CODE();
}

#ifndef MC_INTRA_BOND_TENSOR_AUTO_CORR_CPP
#define MC_INTRA_BOND_TENSOR_AUTO_CORR_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <mcMd/diagnostics/mcSystem/McIntraBondTensorAutoCorr.h>
#include <mcMd/potentials/bond/BondPotential.h>
#include <mcMd/diagnostics/system/IntraBondTensorAutoCorr_inc.h>

namespace McMd
{

   using namespace Util;
  
   /*
   * Constructor.
   */
   McIntraBondTensorAutoCorr::McIntraBondTensorAutoCorr(McSystem &system)
    : IntraBondTensorAutoCorr<McSystem>(system)
   {}

   /*
   * Destructor.
   */
   McIntraBondTensorAutoCorr::~McIntraBondTensorAutoCorr()
   {}

}
#endif

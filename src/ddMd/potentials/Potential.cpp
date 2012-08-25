#ifndef DDMD_POTENTIAL_CPP
#define DDMD_POTENTIAL_CPP

#include "Potential.h"

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

namespace DdMd
{

   using namespace Util;

   /*
   * Constructor.
   */
   Potential::Potential()
    : stress_(),
      energy_(),
      reverseUpdateFlag_(false)
   {}

   /*
   * Destructor.
   */
   Potential::~Potential()
   {}

   /*
   * Set flag to identify if reverse communication is enabled.
   */
   void Potential::setReverseUpdateFlag(bool reverseUpdateFlag)
   { reverseUpdateFlag_ = reverseUpdateFlag; }

   /*
   * Set a value for the total energy.
   */
   double Potential::energy() const
   {  return energy_.value(); }

   /*
   * Set a value for the total energy.
   */
   void Potential::setEnergy(double energy)
   {  energy_.set(energy); }

   /*
   * Mark the energy as unknown. 
   */
   void Potential::unsetEnergy()
   {  energy_.unset(); }

   /*
   * Is the energy set?
   */
   bool Potential::isEnergySet() const
   {  return energy_.isSet(); }

   /*
   * Set a value for the total stress.
   */
   Tensor Potential::stress() const
   {  return stress_.value(); }

   /*
   * Set a value for the pressure.
   */
   double Potential::pressure() const
   {  
      double p = 0.0;
      for (int i = 0; i < Dimension; ++i) {
         p += stress_.value()(i, i);
      }
      p /= 3.0;
      return p;
   }

   /*
   * Set a value for the total stress.
   */
   void Potential::setStress(const Tensor& stress)
   {  stress_.set(stress); }

   /*
   * Mark the stress as unknown. 
   */
   void Potential::unsetStress()
   {  stress_.unset(); }

   /*
   * Is the stress set?
   */
   bool Potential::isStressSet() const
   {  return stress_.isSet(); }

   #ifdef UTIL_MPI
   /*
   * Is the potential in a valid internal state?
   */
   bool Potential::isValid(MPI::Intracomm& communicator) const
   {
      energy_.isValid(communicator);
      stress_.isValid(communicator);
      return true;
   }
   #endif

}
#endif

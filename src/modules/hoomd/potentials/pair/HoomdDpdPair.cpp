#ifndef MCMD_NOPAIR
#ifndef HOOMD_DPD_PAIR_CPP
#define HOOMD_DPD_PAIR_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "HoomdDpdPair.h"


namespace McMd
{
   char classNameHoomdDpd[] = "HoomdDpdPair";

   /**
   * Default constructor.
   */
   HoomdDpdPair::HoomdDpdPair()   
    : HoomdPair< EvaluatorPairDPDThermo, gpu_compute_dpdthermo_forces,
         classNameHoomdDpd >()
   {
   }

   /**
   * Copy constructor
   */
   HoomdDpdPair::HoomdDpdPair(const HoomdDpdPair& other)
    : HoomdPair< EvaluatorPairDPDThermo, gpu_compute_dpdthermo_forces,
         classNameHoomdDpd >(other)
   {
      for (int i = 0; i < nAtomType_; ++i) {
         for (int j = 0; j < nAtomType_; ++j) {
            epsilon_[i][j] = other.epsilon_[i][j];
         }
      }
   }

   /**
   * read parameters from file
   */
   void HoomdDpdPair::readParam(std::istream &in)
   {
      double sigma[MaxAtomType][MaxAtomType];

      // Read parameters
      readCArray2D<double> (
                  in, "epsilon", epsilon_[0], nAtomType_, nAtomType_);
      readCArray2D<double> (
                  in, "sigma",   sigma[0], nAtomType_, nAtomType_);

      // calculate maxPairCutoff and assign parameters
      maxPairCutoff_ = 0.0;
      for (int i = 0; i < nAtomType_; ++i)
         for (int j = 0; j < nAtomType_; ++j) {
            params_[i][j].x = epsilon_[i][j]; // A parameter
            params_[i][j].y = 0;              // gamma parameter
            cutoff_[i][j] = sigma[i][j];
            if (cutoff_[i][j] > maxPairCutoff_)
               maxPairCutoff_ = cutoff_[i][j];
            cutoffSq_[i][j] = cutoff_[i][j] * cutoff_[i][j];
         }
   }

   void HoomdDpdPair::setEpsilon(int i, int j, double epsilon)
   {

      // Preconditions
//      if (!isInitialized_) {
//         UTIL_THROW("Cannot modify epsilon before DPDPair is initialized");
//      }
      if (i < 0 || i >= nAtomType_) {
         UTIL_THROW("Invalid atom type index i");
      }
      if (j < 0 || j >= nAtomType_) {
         UTIL_THROW("Invalid atom type index j");
      }

      epsilon_[i][j] = epsilon;
      params_[i][j].x = epsilon;

      // Symmetrize
      if (j != i) {
         epsilon_[j][i] = epsilon_[i][j];
         params_[j][i] = params_[i][j];
      }
   }

   /* 
   * Get pair interaction strength.
   */
   double HoomdDpdPair::epsilon(int i, int j) const
   {
      assert(i >= 0 && i < nAtomType_);
      assert(j >= 0 && j < nAtomType_);
      return epsilon_[i][j];
   }

}

#endif
#endif
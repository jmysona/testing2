#ifndef DDMD_PAIR_POTENTIAL_CPP
#define DDMD_PAIR_POTENTIAL_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "PairPotential.h"
#include <ddMd/simulation/Simulation.h>
#include <ddMd/storage/AtomStorage.h>
#include <ddMd/storage/AtomIterator.h>
#include <ddMd/storage/GhostIterator.h>
#include <ddMd/neighbor/PairIterator.h>
#include <ddMd/communicate/Domain.h>
#include <util/space/Vector.h>
#include <util/global.h>

namespace DdMd
{
   using namespace Util;

   /*
   * Default constructor (for unit testing).
   */
   PairPotential::PairPotential()
    : skin_(0.0),
      cutoff_(0.0),
      boundaryPtr_(0),
      domainPtr_(0),
      storagePtr_(0),
      pairCapacity_(0)
   {} 

   /*
   * Constructor.
   */
   PairPotential::PairPotential(Simulation& simulation)
    : skin_(0.0),
      cutoff_(0.0),
      boundaryPtr_(&simulation.boundary()),
      domainPtr_(&simulation.domain()),
      storagePtr_(&simulation.atomStorage()),
      pairCapacity_(0)
   {}

   /*
   * Associate with related objects. (for unit testing).
   */
   void PairPotential::associate(Domain& domain, Boundary& boundary, 
                                 AtomStorage& storage)
   {
      domainPtr_ = &domain;
      boundaryPtr_ = &boundary;
      storagePtr_ = &storage;
   } 

   /*
   * Destructor.
   */
   PairPotential::~PairPotential()
   {}

   void PairPotential::readPairListParam(std::istream& in)
   {
      read<double>(in, "skin", skin_);
      read<int>(in, "pairCapacity", pairCapacity_);
      read<Boundary>(in, "maxBoundary", maxBoundary_);

      cutoff_ = maxPairCutoff() + skin_;
      int atomCapacity = storage().atomCapacity();
                       + storage().ghostCapacity();

      // Set upper and lower bound of the processor domain.
      boundary().setLengths(maxBoundary_.lengths());
      Vector lower;
      Vector upper;
      for (int i = 0; i < Dimension; ++i) {
         lower[i] = domain().domainBound(i, 0);
         upper[i] = domain().domainBound(i, 1);
      }

      cellList_.allocate(atomCapacity, lower, upper, cutoff_);
      pairList_.allocate(atomCapacity, pairCapacity_, cutoff_);
   }

   /*
   * Allocate memory for the cell list.
   */
   void PairPotential::setParam(const Vector& lower, const Vector& upper, 
                                double skin, int pairCapacity)
   {
      skin_ = skin;
      pairCapacity_ = pairCapacity;

      cutoff_ = maxPairCutoff() + skin;
      int atomCapacity = storage().atomCapacity();
                       + storage().ghostCapacity();

      cellList_.allocate(atomCapacity, lower, upper, cutoff_);
      pairList_.allocate(atomCapacity, pairCapacity_, cutoff_);
   }

   /*
   * Build the cell list (i.e., fill with atoms).
   */
   void PairPotential::findNeighbors(const Vector& lower, const Vector& upper)
   {
      cellList_.makeGrid(lower, upper, cutoff_);
      cellList_.clear();
     
      // Add all atoms to the cell list. 
      AtomIterator atomIter;
      storage().begin(atomIter);
      for ( ; !atomIter.atEnd(); ++atomIter) {
         cellList_.placeAtom(*atomIter);
      }

      // Add all ghosts to the cell list. 
      GhostIterator ghostIter;
      storage().begin(ghostIter);
      for ( ; !ghostIter.atEnd(); ++ghostIter) {
         cellList_.placeAtom(*ghostIter);
      }

      cellList_.build();
      assert(cellList_.isValid());

      pairList_.build(cellList_);
   }

   /*
   * Build the cell list (i.e., fill with atoms).
   */
   void PairPotential::findNeighbors()
   {
      // Make the cell list grid.
      Vector lower;
      Vector upper;
      for (int i = 0; i < Dimension; ++i) {
         lower[i] = domain().domainBound(i, 0);
         upper[i] = domain().domainBound(i, 1);
      }
      findNeighbors(lower, upper);
   }

   #if 0
   /*
   * Set forces on all local atoms to zero.
   */
   void PairPotential::zeroForces()
   {
      AtomIterator atomIter;
      storage().begin(atomIter); 
      for( ; !atomIter.atEnd(); ++atomIter){
         atomIter->force().zero();
      }
   }

   /*
   * Set forces on all local atoms to zero.
   */
   void PairPotential::calculateForces()
   {
      zeroForces();
      addForces();
   }
   #endif

}
#endif

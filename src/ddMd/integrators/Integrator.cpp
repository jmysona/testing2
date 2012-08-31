#ifndef DDMD_INTEGRATOR_CPP
#define DDMD_INTEGRATOR_CPP

#include "Integrator.h"
#include <ddMd/simulation/Simulation.h>
#include <ddMd/storage/AtomStorage.h>
#include <ddMd/storage/AtomIterator.h>
#include <ddMd/communicate/Exchanger.h>
#include <ddMd/potentials/pair/PairPotential.h>
#include <ddMd/potentials/bond/BondPotential.h>
#ifdef INTER_ANGLE
#include <ddMd/potentials/angle/AnglePotential.h>
#endif
#ifdef INTER_DIHEDRAL
#include <ddMd/potentials/dihedral/DihedralPotential.h>
#endif
#ifdef INTER_EXTERNAL
#include <ddMd/potentials/external/ExternalPotential.h>
#endif

#include <util/format/Dbl.h>
#include <util/format/Int.h>
#include <util/format/Bool.h>
#include <util/global.h>


/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

namespace DdMd
{

   class Simulation;

   /*
   * Constructor.
   */
   Integrator::Integrator(Simulation& simulation)
     : SimulationAccess(simulation),
       timer_(Integrator::NTime)
   {}

   /*
   * Destructor.
   */
   Integrator::~Integrator()
   {}

   void Integrator::setupAtoms()
   {
      atomStorage().clearSnapshot();
      exchanger().exchange();
      pairPotential().buildCellList();
      if (!UTIL_ORTHOGONAL) {
         atomStorage().transformGenToCart(boundary());
      }
      atomStorage().makeSnapshot();
      pairPotential().buildPairList();
      simulation().computeForces();
   }

   /*
   * Compute forces for all atoms.
   */
   void Integrator::computeForces()
   {
      simulation().zeroForces();
      pairPotential().computeForces();
      timer_.stamp(PAIR_FORCE);
      bondPotential().computeForces();
      timer_.stamp(BOND_FORCE);
      #ifdef INTER_ANGLE
      if (nAngleType()) {
         anglePotential().computeForces();
         timer_.stamp(ANGLE_FORCE);
      }
      #endif
      #ifdef INTER_DIHEDRAL
      if (nDihedralType()) {
         dihedralPotential().computeForces();
         timer_.stamp(DIHEDRAL_FORCE);
      }
      #endif
      #ifdef INTER_EXTERNAL
      if (hasExternal()) {
         externalPotential().computeForces();
      }
      #endif

      // Reverse communication (if any)
      if (reverseUpdateFlag()) {
         exchanger().reverseUpdate();
      }

      // Send signal indicating change in atomic forces
      simulation().forceSignal().notify();
   }

   /*
   * Compute forces for all local atoms and virial, with timing.
   */
   void Integrator::computeForcesAndVirial()
   {
      simulation().zeroForces();
      pairPotential().computeForcesAndStress(domain().communicator());
      timer_.stamp(PAIR_FORCE);
      bondPotential().computeForcesAndStress(domain().communicator());
      timer_.stamp(BOND_FORCE);
      #ifdef INTER_ANGLE
      if (nAngleType()) {
         anglePotential().computeForcesAndStress(domain().communicator());
         timer_.stamp(ANGLE_FORCE);
      }
      #endif
      #ifdef INTER_DIHEDRAL
      if (nDihedralType()) {
         dihedralPotential().computeForcesAndStress(domain().communicator());
         timer_.stamp(DIHEDRAL_FORCE);
      }
      #endif
      #ifdef INTER_EXTERNAL
      if (hasExternal()) {
         externalPotential().computeForces();
      }
      #endif

      // Reverse communication (if any)
      if (reverseUpdateFlag()) {
         exchanger().reverseUpdate();
      }

      // Send signal indicating change in atomic forces
      simulation().forceSignal().notify();
   }

   /*
   * Return time per processor for last run.
   */
   double Integrator::time() const
   {  return timer_.time(); }

   /*
   * Return number of time steps in last run.
   */
   int Integrator::nStep() const
   {  return nStep_; }

   /*
   * Output statistics.
   */
   void Integrator::outputStatistics(std::ostream& out)
   {
      if (domain().isMaster()) {

         double time = timer().time();
         int nAtomTot = atomStorage().nAtomTotal();
         int nProc = 1;
         #ifdef UTIL_MPI
         nProc = domain().communicator().Get_size();
         #endif

         // Output total time for the run
         out << std::endl;
         out << "Time Statistics" << std::endl;
         out << "nStep                " << nStep_ << std::endl;
         out << "run time             " << time << " sec" << std::endl;
         out << "time / nStep         " << time/double(nStep_) 
             << " sec" << std::endl;

         double ratio = double(nProc)/double(nStep_*nAtomTot);

         double diagnosticT = timer().time(DIAGNOSTIC);
         double integrate1T = timer().time(INTEGRATE1);
         double checkT =  timer().time(CHECK);
         double transformFT = timer().time(TRANSFORM_F);
         double exchangeT = timer().time(EXCHANGE);
         double cellListT = timer().time(CELLLIST);
         double transformRT = timer().time(TRANSFORM_R);
         double pairListT = timer().time(PAIRLIST);
         double updateT = timer().time(UPDATE);
         double pairForceT = timer().time(PAIR_FORCE);
         double bondForceT = timer().time(BOND_FORCE);
         double integrate2T = timer().time(INTEGRATE2);

         out << std::endl;
         out << "time * nproc / (nStep*nAtom):" << std::endl;
         out << "Total                " << Dbl(time*ratio, 12, 6)
             << " sec   " << std::endl;
         out << "Diagnostics          " << Dbl(diagnosticT*ratio, 12, 6)
             << " sec   " << Dbl(diagnosticT/time, 12, 6, true) << std::endl;
         out << "Integrate1           " << Dbl(integrate1T*ratio, 12, 6) 
             << " sec   " << Dbl(integrate1T/time, 12, 6, true) << std::endl;
         out << "Check                " << Dbl(checkT*ratio, 12, 6)
             << " sec   " << Dbl(checkT/time, 12, 6, true) << std::endl;
         if (!UTIL_ORTHOGONAL) 
         out << "Transform (forward)  " << Dbl(transformFT*ratio, 12, 6)
             << " sec   " << Dbl(transformFT/time, 12, 6, true) << std::endl;
         out << "Exchange             " << Dbl(exchangeT*ratio, 12, 6)
             << " sec   " << Dbl(exchangeT/time, 12, 6, true) << std::endl;
         out << "CellList             " << Dbl(cellListT*ratio, 12, 6)
             << " sec   " << Dbl(cellListT/time, 12, 6, true) << std::endl;
         if (!UTIL_ORTHOGONAL) 
         out << "Transform (reverse)  " << Dbl(transformRT*ratio, 12, 6)
             << " sec   " << Dbl(transformRT/time, 12, 6, true) << std::endl;
         out << "PairList             " << Dbl(pairListT*ratio, 12, 6)
             << " sec   " << Dbl(pairListT/time, 12, 6, true) << std::endl;
         out << "Update               " << Dbl(updateT*ratio, 12, 6)
             << " sec   " << Dbl(updateT/time, 12, 6, true) << std::endl;
         out << "Pair Forces          " << Dbl(pairForceT*ratio, 12, 6)
             << " sec   " << Dbl(pairForceT/time, 12 , 6, true) << std::endl;
         out << "Bond Forces          " << Dbl(bondForceT*ratio, 12, 6)
             << " sec   " << Dbl(bondForceT/time, 12 , 6, true) << std::endl;
         out << "Integrate2           " << Dbl(integrate2T*ratio, 12, 6) 
             << " sec   " << Dbl(integrate2T/time, 12, 6, true) << std::endl;
         out << std::endl;

         int buildCounter = pairPotential().pairList().buildCounter(); 
         out << "buildCounter             " 
                     << Int(buildCounter, 10)
                     << std::endl;
         out << "steps / build            "
                     << double(nStep_)/double(buildCounter)
                     << std::endl;
         out << std::endl;

      }

   }

}
#endif

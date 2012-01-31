#ifndef MC_INTRA_BOND_STRESS_AUTO_CORR_H
#define MC_INTRA_BOND_STRESS_AUTO_CORR_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <mcMd/diagnostics/system/IntraBondStressAutoCorr.h> // base template
#include <mcMd/mcSimulation/McSystem.h>                      // parameter

namespace McMd
{

   using namespace Util;


   /**
   * Autocorrelation for bond stress of a molecule.
   *
   * \ingroup Diagnostic_Module
   */
   class McIntraBondStressAutoCorr : public IntraBondStressAutoCorr<McSystem>
   {
   
   public:
  
      /**
      * Constructor.
      *
      * \param system reference to parent System
      */
      McIntraBondStressAutoCorr(McSystem &system);

      /**
      * Destructor.
      */
      virtual ~McIntraBondStressAutoCorr();
  
      using IntraBondStressAutoCorr<McSystem>::readParam;
      using IntraBondStressAutoCorr<McSystem>::initialize;
      using IntraBondStressAutoCorr<McSystem>::sample;
      using IntraBondStressAutoCorr<McSystem>::output;

      /**
      * Serialize to/from an archive. 
      *
      * \param ar      saving or loading archive
      * \param version archive version id
      */
      template <class Archive>
      void serialize(Archive& ar, const unsigned int version);

   };

   /*
   * Serialize to/from an archive. 
   */
   template <class Archive>
   void 
   McIntraBondStressAutoCorr::serialize(Archive& ar, const unsigned int version)
   {  IntraBondStressAutoCorr<McSystem>::serialize(ar, version); }

}
#endif
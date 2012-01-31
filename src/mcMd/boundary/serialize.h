#ifndef BOUNDARY_SERIALIZE_H
#define BOUNDARY_SERIALIZE_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "OrthoRegion.h"  
#include "OrthorhombicBoundary.h"  

namespace McMd
{

   using namespace Util;

   /*
   * Serialize an OrthoRegion to/from an archive.
   */
   template <class Archive>
   void OrthoRegion::serialize(Archive& ar, const unsigned int version)
   {
      ar & minima_;
      ar & maxima_;
      ar & lengths_;
      ar & halfLengths_;
      ar & volume_;
   }

   /*
   * Serialize an OrthorhombicBoundary to/from an archive.
   */
   template <class Archive>
   void 
   OrthorhombicBoundary::serialize(Archive& ar, const unsigned int version)
   {
      OrthoRegion::serialize(ar, version);
      serializeEnum(ar, lattice_, version);
      reset();
      if (Archive::is_loading()) {
         isValid();
      }
   }

}
#endif
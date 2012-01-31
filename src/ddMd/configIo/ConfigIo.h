#ifndef CONFIG_IO_H
#define CONFIG_IO_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <util/param/ParamComposite.h>           // base class
#include <ddMd/communicate/AtomDistributor.h>        // member 

//#include <ddMd/system/System.h>                  // member
//#include <ddMd/storage/AtomStorage.h>                // member 
//#include <ddMd/communicate/Domain.h>             // member 
//#include <ddMd/communicate/Buffer.h>             // member 
#include <ddMd/boundary/Boundary.h>              // typedef

namespace DdMd
{

   class System;
   class AtomStorage;
   class Domain;
   class Buffer;

   using namespace Util;

   /**
   * 
   */
   class ConfigIo  : public ParamComposite
   {

   public:

      /**
      * Constructor.
      */
      ConfigIo(System& system, Buffer& buffer);

      /**
      * Read cache size and allocate memory.
      */
      virtual void readParam(std::istream& in);

      /**
      * Read configuration file.
      *
      * This routine opens and reads a file on the master,
      * and distributes atom data among the processors.
      *
      * \param filename name of configuration file.
      */
      virtual void readConfig(std::string filename);

      /**
      * Write configuration file.
      *
      * This routine opens and writes a file on the master,
      * collecting atom data from all processors.
      *
      * \param filename name of output configuration file.
      */
      virtual void writeConfig(std::string filename);

   protected:

      /**
      * Get the AtomDistributor by reference.
      */
      AtomDistributor& distributor();

      /**
      * Get AtomStorage by reference.
      */
      System& system();
   
      /**
      * Get AtomStorage by reference.
      */
      AtomStorage& atomStorage();
   
      /**
      * Get the Domain by reference.
      */
      Domain& domain();

      /**
      * Get Boundary by reference.
      */
      Boundary& boundary();
   
   private:

      AtomDistributor  distributor_;

      System*      systemPtr_;

      AtomStorage*     storagePtr_;

      Domain*      domainPtr_;

      Boundary*    boundaryPtr_;

      int          cacheCapacity_;

   };

   // Inline method definitions

   inline AtomDistributor& ConfigIo::distributor()
   { return distributor_; }

   inline System& ConfigIo::system()
   { return *systemPtr_; }

   inline AtomStorage& ConfigIo::atomStorage()
   { return *storagePtr_; }

   inline Domain& ConfigIo::domain()
   { return *domainPtr_; }

   inline Boundary& ConfigIo::boundary()
   { return *boundaryPtr_; }

}
#endif
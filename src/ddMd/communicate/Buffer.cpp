#ifndef BUFFER_CPP
#define BUFFER_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "Buffer.h"
#include "Domain.h"
#include <ddMd/chemistry/Atom.h>
#include <ddMd/chemistry/Group.h>
#include <util/mpi/MpiLogger.h>

namespace DdMd
{
   using namespace Util;

   /*
   * Constructor.
   */
   Buffer::Buffer()
    : Util::ParamComposite(),
      #ifdef UTIL_MPI
      sendBufferBegin_(0),
      recvBufferBegin_(0),
      sendBufferEnd_(0),
      recvBufferEnd_(0),
      sendBlockBegin_(0),
      recvBlockBegin_(0),
      sendPtr_(0),
      recvPtr_(0),
      bufferCapacity_(-1),
      sendSize_(0),
      recvSize_(0),
      #endif
      atomCapacity_(-1),
      ghostCapacity_(-1),
      isInitialized_(false)
   {}

   /*
   * Destructor.
   */
   Buffer::~Buffer()
   {}

   /*
   * Allocate send and recv buffers.
   */
   void Buffer::allocate(int atomCapacity, int ghostCapacity)
   {
      //Preconditions
      if (atomCapacity < 0) {
         UTIL_THROW("Negative atomCapacity");
      }
      if (ghostCapacity < 0) {
         UTIL_THROW("Negative ghostCapacity");
      }

      atomCapacity_  = atomCapacity;
      ghostCapacity_ = ghostCapacity;

      #ifdef UTIL_MPI
      // Do actual allocation
      allocate();
      #endif

      isInitialized_ = true;
   }

   /*
   * Read capacities, and allocate buffers.
   */
   void Buffer::readParam(std::istream& in)
   {

      // Read parameters
      readBegin(in, "Buffer");
      read<int>(in, "atomCapacity",  atomCapacity_);
      read<int>(in, "ghostCapacity", ghostCapacity_);
      readEnd(in);

      //Preconditions
      if (atomCapacity_ < 0) {
         UTIL_THROW("Negative atomCapacity");
      }
      if (ghostCapacity_ < 0) {
         UTIL_THROW("Negative ghostCapacity");
      }

      #ifdef UTIL_MPI
      // Do actual allocation
      allocate();
      #endif

      isInitialized_ = true;
   }

   /*
   * Maximum number of atoms for which space is available.
   */
   int Buffer::atomCapacity() const
   {  return atomCapacity_; }

   /*
   * Maximum number of ghost atoms for which space is available.
   */
   int Buffer::ghostCapacity() const
   {  return ghostCapacity_; }

   /*
   * Has this buffer been initialized?
   */
   bool Buffer::isInitialized() const
   {  return isInitialized_; }

   #ifdef UTIL_MPI

   /*
   * Allocate send and recv buffers (private method).
   *
   * This method uses values of atomCapacity_ and ghostCapacity_ that 
   * must have been set previously. It is called by allocate(int, int) 
   * and readParam() to do the actual allocation. 
   */
   void Buffer::allocate()
   {

      // Preconditions
      if (atomCapacity_ <= 0) {
         UTIL_THROW("atomCapacity_ must be positive");
      }
      if (ghostCapacity_ <= 0) {
         UTIL_THROW("ghostCapacity_ must be positive");
      }
      if (isAllocated()) {
         UTIL_THROW("Buffer cannot be re-allocated");
      }

      // Capacity in bytes send and receive buffers. This is maximum of
      // the buffer space required by the local atoms and ghost atoms.
      bufferCapacity_  = ((atomCapacity_ * localAtomSize_) >
                          (ghostCapacity_ * ghostAtomSize_)) 
                         ? (atomCapacity_ * localAtomSize_)   
                         : (ghostCapacity_ * ghostAtomSize_);

      if (bufferCapacity_ <= 0) {
         UTIL_THROW("Zero or negative buffer capacity");
      }

      // Leave space for a 4 byte header
      bufferCapacity_ += 4 * sizeof(int);

      // Allocate memory for the send buffer 
      sendBufferBegin_ = new char[bufferCapacity_];
      if (sendBufferBegin_ == 0) {
         UTIL_THROW("Error: memory not allocated for send buffer");
      }
      sendBufferEnd_ = sendBufferBegin_ + bufferCapacity_;

      // Allocate memory for the receive buffer
      recvBufferBegin_ = new char[bufferCapacity_];
      if (recvBufferBegin_ == 0) {
         UTIL_THROW("Error: memory not allocated for recv buffer");
      }
      recvBufferEnd_ = recvBufferBegin_ + bufferCapacity_;

      recvPtr_ = recvBufferBegin_;

   }

   /*
   * Clear the send buffer prior to packing, and set the sendType.
   */
   void Buffer::clearSendBuffer()
   { 
      sendPtr_ = sendBufferBegin_;
      sendSize_ = 0;
      sendType_ = NONE;
      sendBlockBegin_ = 0;
   }

   /*
   * Clear the send buffer prior to packing, and set the sendType.
   */
   void Buffer::beginSendBlock(BlockDataType sendType)
   {
      if (sendSize_ != 0) {
         UTIL_THROW("Error: previous send block not finalized");
      }

      // Set number of atoms currently in send block to zero.
      sendSize_ = 0;

      // Data type to be sent.
      sendType_ = sendType;

      // Mark beginning of block.
      sendBlockBegin_ = sendPtr_;

      // Increment sendPtr_ to leave space for 4 integers.
      int* sendBuffPtr = (int *)sendPtr_;
      sendBuffPtr += 4;
      sendPtr_ = (char *)sendBuffPtr;

   }

   /*
   * Finalize data block in buffer. Pack prefix data.
   */
   void Buffer::endSendBlock(bool isComplete)
   {
      // Add passport to the beginning of the block:
      // Pack sendSize_, sendType_ and isComplete.
      int* sendBuffPtr = (int *)sendBlockBegin_;
      *sendBuffPtr = sendSize_;
      ++sendBuffPtr;
      *sendBuffPtr = (int) sendType_;
      ++sendBuffPtr;
      *sendBuffPtr = (int) isComplete;

      // Clear variables associated with the sent block.
      sendBlockBegin_ = 0;
      sendSize_ = 0;
      sendType_ = NONE;

   }

   /*
   * Begin receiving block. Extract prefix data. 
   */
   bool Buffer::beginRecvBlock()
   {
      // Precondition
      if (recvSize_ != 0) {
         UTIL_THROW("Error: Previous receive block not completely unpacked");
      }

      // Extract the number packed atoms and type from the receive buffer
      int* recvBuffPtr = (int *)recvPtr_;
      recvSize_  = *recvBuffPtr;
      recvType_  = *(recvBuffPtr + 1);
      bool isComplete = (bool) *(recvBuffPtr + 2);
      recvBuffPtr += 4;

      // Set recvPtr to beginning of first item to be unpacked
      recvPtr_ = (char *)recvBuffPtr;

      return isComplete;
   }

   /*
   * Pack a local Atom for exchange of ownership.
   */
   void Buffer::packAtom(Atom& atom)
   {
      // Preconditions
      if (sendType_ != ATOM) {
         UTIL_THROW("Send type is not ATOM");
      }
      if (sendSize_ >= atomCapacity_) {
         UTIL_THROW("Attempt to overpack send buffer");
      }

      pack<int>(atom.typeId());
      pack<int>(atom.id());
      pack<Vector>(atom.position());
      pack<Vector>(atom.velocity());

      //Increment number of atoms in send buffer by 1
      ++sendSize_;

   }

   /*
   * Receive ownership of an Atom.
   */
   void Buffer::unpackAtom(Atom& atom)
   {
      // Preconditions
      if (recvType_ != (int)ATOM) {
         UTIL_THROW("Receive type is not ATOM");
      }
      if (recvSize_ <= 0) {
         UTIL_THROW("Attempt to unpack empty receive buffer");
      }

      int i;
      unpack(i);
      atom.setTypeId(i);
      unpack(i);
      atom.setId(i);
      unpack<Vector>(atom.position());
      unpack<Vector>(atom.velocity());

      // Decrement number of atoms in recv buffer by 1
      recvSize_--;

   }

   /*
   * Pack data required for a ghost Atom for sending.
   */
   void Buffer::packGhost(Atom& atom)
   {
      // Preconditions
      if (sendType_ != GHOST) {
         UTIL_THROW("Send type is not GHOST");
      }
      if (sendSize_ >= ghostCapacity_) {
         UTIL_THROW("Attempt to overpack send buffer");
      }

      Vector pos;
      pack<int>(atom.typeId());
      pack<int>(atom.id());
      pack<Vector>(atom.position());

      //Increment number of atoms in send buffer by 1
      sendSize_++;

   }

   /**
   * Unpack data required for a ghost Atom.
   */
   void Buffer::unpackGhost(Atom& atom)
   {
      // Preconditions
      if (recvType_ != (int)GHOST) {
         UTIL_THROW("Receive type is not GHOST");
      }
      if (recvSize_ <= 0) {
         UTIL_THROW("Attempt to unpack empty receive buffer");
      }

      int i;
      unpack(i);
      atom.setTypeId(i);
      unpack(i);
      atom.setId(i);
      unpack<Vector>(atom.position());
      
      //Decrement number of atoms in recv buffer to be unpacked by 1
      recvSize_--;
   }

   /*
   * Pack a Group.
   */
   template <int N>
   void Buffer::packGroup(const Group<N>& group)
   {
      pack<int>(group.typeId());
      pack<int>(group.id());
      for (int j = 0; j < N; ++j) {
         pack<int>(group.atomId(j));
      }

      //Increment number of groups in send buffer by 1
      ++sendSize_;

   }

   /*
   * Unpack a Group.
   */
   template <int N>
   void Buffer::unpackGroup(Group<N>& group)
   {
      int i, j;
      unpack(i);
      group.setTypeId(i);
      unpack(i);
      group.setId(i);
      for (j = 0; j < N; ++j) {
         unpack(i);
         group.setAtomId(j, i);
      }

      // Decrement number of groups in recv buffer by 1
      recvSize_--;

   }

   /*
   * Pack data required for a ghost Atom for sending.
   */
   void Buffer::packBond(Bond& bond)
   {
      // Preconditions
      if (sendType_ != BOND) {
         UTIL_THROW("Send type is not BOND");
      }
      packGroup<2>(bond);
   }

   /**
   * Unpack data required for a ghost Atom.
   */
   void Buffer::unpackBond(Bond& bond)
   {
      if (recvType_ != (int)BOND) {
         UTIL_THROW("Receive type is not BOND");
      }
      unpackGroup<2>(bond);
   }

   /*
   * Send and receive buffer.
   */
   void Buffer::sendRecv(MPI::Intracomm& comm, int source, int dest)
   {

      MPI::Request request[2];
      int* sendBuffPtr = 0;
      int* recvBuffPtr = 0;
      int  sendBytes = 0;
      int  myRank    = comm.Get_rank();
      int  comm_size = comm.Get_size();

      // Preconditions
      if (dest > comm_size - 1 || dest < 0) {
         UTIL_THROW("Destination rank out of bounds");
      }
      if (source > comm_size - 1 || source < 0) {
         UTIL_THROW("Source rank out of bounds");
      }
      if (dest == myRank) {
         UTIL_THROW("Destination and my rank are identical");
      }
      if (source == myRank) {
         UTIL_THROW("Source and my rank are identical");
      }

      // Start nonblocking receive.
      request[0] = comm.Irecv(recvBufferBegin_, bufferCapacity_ , 
                              MPI::CHAR, source, 5);

      // Start nonblock send.
      sendBytes = sendPtr_ - sendBufferBegin_;
      request[1] = comm.Isend(sendBufferBegin_, sendBytes , MPI::CHAR, dest, 5);

      // Wait for completion of receive buffer.
      request[0].Wait();
      recvPtr_ = recvBufferBegin_;

      // Wait for completion of send
      request[1].Wait();

      #if 0
      MpiLogger logger;
      logger.begin();
      std::cout << " My rank                 " << myRank << std::endl;
      std::cout << " Receiving from          " << source << std::endl;
      std::cout << " Send number of bytes  = " << sendBytes << std::endl;
      std::cout << " Sent number of atoms  = " << sendSize_ << std::endl;
      std::cout << " Received number atoms = " << recvSize_ << std::endl;
      logger.end();
      #endif

   }

   /*
   * Send a buffer.
   */
   void Buffer::send(MPI::Intracomm& comm, int dest)
   {
      MPI::Request request;
      int* sendBuffPtr;
      int  sendBytes = 0;
      int  comm_size = comm.Get_size();
      int  myRank = comm.Get_rank();

      // Preconditions
      if (dest > comm_size - 1 || dest < 0) {
         UTIL_THROW("Destination rank out of bounds");
      }
      if (dest == myRank) {
         UTIL_THROW("Source and desination identical");
      }

      sendBytes = sendPtr_ - sendBufferBegin_;
      request = comm.Isend(sendBufferBegin_, sendBytes, MPI::CHAR, dest, 5);
      request.Wait();

   }

   /*
   * Receive a buffer.
   */
   void Buffer::recv(MPI::Intracomm& comm, int source)
   {
      MPI::Request request;
      int* recvBuffPtr;
      int  myRank     = comm.Get_rank();
      int  comm_size  = comm.Get_size();

      // Preconditons
      if (source > comm_size - 1 || source < 0) {
         UTIL_THROW("Source rank out of bounds");
      }
      if (source == myRank) {
         UTIL_THROW("Source and desination identical");
      }

      recvType_ = NONE;
      request = comm.Irecv(recvBufferBegin_, bufferCapacity_, MPI::CHAR, source, 5);
      request.Wait();
      recvPtr_ = recvBufferBegin_;

   }

   /*
   * Broadcast a buffer.
   */
   void Buffer::bcast(MPI::Intracomm& comm, int source)
   {
      MPI::Request request;
      int  comm_size = comm.Get_size();
      int  myRank = comm.Get_rank();

      if (myRank == source) {
         int sendBytes = sendPtr_ - sendBufferBegin_;
         comm.Bcast(sendBufferBegin_, sendBytes, 
                    MPI::CHAR, source);
         sendPtr_ = sendBufferBegin_;
         sendType_ = NONE;
      } else {
         comm.Bcast(recvBufferBegin_, bufferCapacity_, 
                    MPI::CHAR, source);
         recvType_ = NONE;
         recvPtr_ = recvBufferBegin_;
      }

   }

   /*
   * Number of items currently in data send block. 
   */
   int Buffer::sendSize() const
   {  return sendSize_; }

   /*
   * Number of unread items currently in data receive block. 
   */
   int Buffer::recvSize() const
   {  return recvSize_; }

   /*
   * Has this buffer been allocated?
   */
   bool Buffer::isAllocated() const
   {  return (bufferCapacity_ > 0); }

   /*
   * Pack an object of type T into send buffer.
   */
   template <typename T>
   void Buffer::pack(const T& data)
   {
      if (sendPtr_ + sizeof(data) > sendBufferEnd_) {
         UTIL_THROW("Attempted write past end of send buffer");
      }
      T* ptr = (T *)sendPtr_;
      *ptr = data;
      ++ptr;
      sendPtr_ = (char *)ptr;
   }

   /*
   * Unpack an object of type T from recvBuffer.
   */
   template <typename T>
   void Buffer::unpack(T& data)
   {
      if (recvPtr_ + sizeof(data) > recvBufferEnd_) {
         UTIL_THROW("Attempted read past end of recv buffer");
      }
      T* ptr = (T *)recvPtr_;
      data = *ptr;
      ++ptr;
      recvPtr_ = (char *)ptr;
   }
   #endif
}
#endif
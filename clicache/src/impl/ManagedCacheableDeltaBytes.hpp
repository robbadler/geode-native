/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "../geode_defs.hpp"
#include <vcclr.h>
#include "begin_native.hpp"
#include "SerializationRegistry.hpp"
#include <geode/Cache.hpp>
#include <geode/Delta.hpp>
#include <geode/DataOutput.hpp>
#include "end_native.hpp"

#include "../Log.hpp"
#include "../DataOutput.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      interface class IGeodeSerializable;
      interface class IGeodeDelta;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


namespace apache
{
  namespace geode
  {
    namespace client
    {


      /// <summary>
      /// Wraps the managed <see cref="Apache.Geode.Client.IGeodeDelta" />
      /// object and implements the native <c>apache::geode::client::CacheableKey</c> interface.
      /// </summary>
      class ManagedCacheableDeltaBytesGeneric
        : public CacheableKey, public Delta
      {
      public:

        /// <summary>
        /// Constructor to initialize with the provided managed object.
        /// </summary>
        /// <param name="managedDeltaptr">
        /// The managed object.
        /// </param>
        inline ManagedCacheableDeltaBytesGeneric(
          Apache::Geode::Client::IGeodeDelta^ managedDeltaptr, bool storeBytes)
          :Delta(nullptr), m_domainId(System::Threading::Thread::GetDomainID()),
          m_classId(0),
          m_bytes(NULL),
          m_size(0),
          m_hasDelta(false),
          m_hashCode(0)
        {
          if (storeBytes)
            m_hasDelta = managedDeltaptr->HasDelta();
          Apache::Geode::Client::IGeodeSerializable^ managedptr =
            dynamic_cast <Apache::Geode::Client::IGeodeSerializable^> (managedDeltaptr);
          if (managedptr != nullptr)
          {
            m_classId = managedptr->ClassId;
            Apache::Geode::Client::Log::Finer("ManagedCacheableDeltaBytes::Constructor: current AppDomain ID: " + System::Threading::Thread::GetDomainID() + " for object: " + System::Convert::ToString((uint64_t) this) + " with its AppDomain ID: " + m_domainId);
            Apache::Geode::Client::Log::Finer("ManagedCacheableDeltaBytes::Constructor: class ID " + managedptr->ClassId + " : " + managedptr->ToString() + " storeBytes:" + storeBytes);
            if (storeBytes)
            {
              auto dataOut = m_cache->createDataOutput();
              Apache::Geode::Client::DataOutput mg_output(dataOut.get(), true);
              managedptr->ToData(%mg_output);

              //move cursor
              //dataOut.advanceCursor(mg_output.BufferLength);
              mg_output.WriteBytesToUMDataOutput();

              m_bytes = dataOut->getBufferCopy();
              m_size = dataOut->getBufferLength();
              m_hashCode = managedptr->GetHashCode();
              Apache::Geode::Client::Log::Finer("ManagedCacheableDeltaBytes::Constructor objectSize = " + m_size + " m_hashCode = " + m_hashCode);
            }
          }
        }
        
        /// <summary>
        /// serialize this object
        /// </summary>
        virtual void toData(apache::geode::client::DataOutput& output) const;

        /// <summary>
        /// deserialize this object, typical implementation should return
        /// the 'this' pointer.
        /// </summary>
        virtual apache::geode::client::Serializable* fromData(apache::geode::client::DataInput& input);

        virtual void toDelta(apache::geode::client::DataOutput& output) const;

        virtual void fromDelta(apache::geode::client::DataInput& input);

        /// <summary>
        /// return the size of this object in bytes
        /// </summary>
        virtual System::UInt32 objectSize() const;

        /// <summary>
        /// return the classId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        virtual System::Int32 classId() const;

        /// <summary>
        /// return the typeId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        virtual int8_t typeId() const;

        /// <summary>
        /// return the Data Serialization Fixed ID type.
        /// This is used to determine what instance type to create
        /// and deserialize into.
        ///
        /// Note that this should not be overridden by custom implementations
        /// and is reserved only for builtin types.
        /// </summary>
        virtual int8_t DSFID() const;

        virtual bool hasDelta();

        virtual apache::geode::client::DeltaPtr clone();

        /// <summary>
        /// return the hashcode for this key.
        /// </summary>
        virtual System::Int32 hashcode() const;

        /// <summary>
        /// return true if this key matches other CacheableKey
        /// </summary>
        virtual bool operator == (const CacheableKey& other) const;

        /// <summary>
        /// return true if this key matches other ManagedCacheableDeltaBytesGeneric
        /// </summary>
        virtual bool operator == (const ManagedCacheableDeltaBytesGeneric& other) const;

        /// <summary>
        /// Copy the string form of a key into a char* buffer for logging purposes.
        /// implementations should only generate a string as long as maxLength chars,
        /// and return the number of chars written. buffer is expected to be large 
        /// enough to hold at least maxLength chars.
        /// The default implementation renders the classname and instance address.
        /// </summary>
        virtual size_t logString(char* buffer, size_t maxLength) const;

        /// <summary>
        /// Returns the wrapped managed object reference.
        /// </summary>
        inline Apache::Geode::Client::IGeodeDelta^ ptr() const
        {
          return getManagedObject();
        }

        inline ~ManagedCacheableDeltaBytesGeneric()
        {
          Apache::Geode::Client::Log::Finer("ManagedCacheableDeltaBytes::Destructor current AppDomain ID: " + System::Threading::Thread::GetDomainID() + " for object: " + System::Convert::ToString((uint64_t) this) + " with its AppDomain ID: " + m_domainId);
          GF_SAFE_DELETE(m_bytes);
        }

      private:
        Apache::Geode::Client::IGeodeDelta^ getManagedObject() const;
        /// <summary>
        /// Using gcroot to hold the managed delegate pointer (since it cannot be stored directly).
        /// Note: not using auto_gcroot since it will result in 'Dispose' of the IGeodeDelta
        /// to be called which is not what is desired when this object is destroyed. Normally this
        /// managed object may be created by the user and will be handled automatically by the GC.
        /// </summary>
        //gcroot<Apache::Geode::Client::IGeodeDelta^> m_managedptr;
        //gcroot<Apache::Geode::Client::IGeodeSerializable^> m_managedSerializableptr;

        int m_domainId;
        UInt32 m_classId;
        System::Byte * m_bytes;
        System::UInt32 m_size;
        System::UInt32 m_hashCode;
        bool m_hasDelta;

        // Disable the copy and assignment constructors
        ManagedCacheableDeltaBytesGeneric(const ManagedCacheableDeltaBytesGeneric&);
        ManagedCacheableDeltaBytesGeneric& operator = (const ManagedCacheableDeltaBytesGeneric&);
      };

    }
  }
}

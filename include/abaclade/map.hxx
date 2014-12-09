﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_MAP_HXX
#define _ABACLADE_MAP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/range.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::map_impl

namespace abc {
namespace detail {

//! Non-template implementation class for abc::map.
class ABACLADE_SYM map_impl {
protected:
   typedef void (* destruct_key_value_fn)(void * pKey, void * pValue);
   typedef bool (* keys_equal_fn)(map_impl const * pmapi, void const * pKey1, void const * pKey2);
   typedef void (* move_key_value_to_bucket_fn)(
      map_impl * pmapi, void * pKey, void * pValue, std::size_t iBucket
   );

public:
   /*! Constructor.

   @param m
      Source object.
   */
   map_impl();
   map_impl(map_impl && m);

   //! Destructor.
   ~map_impl();

   /*! Assignment operator.

   @param m
      Source object.
   */
   map_impl & operator=(map_impl && m);

   /*! Returns the maximum number of key/value pairs the map can currently hold.

   @return
      Current size of the allocated storage, in elements.
   */
   std::size_t capacity() const {
      return m_cBuckets;
   }

   /*! Returns the count of elements in the map.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return m_cUsedBuckets;
   }

protected:
   /*! Finds the first (non-empty) bucket whose contents can be moved to the specified bucket.

   @param iEmptyBucket
      Index of the empty bucket, which is also the last bucket of the neighborhood to scan.
   @return
      Index of the first bucket whose contents can be moved, or smc_iNullIndex if none of the
      occupied buckets contain keys from the neighborhood ending at iEmptyBucket.
   */
   std::size_t find_bucket_movable_to_empty(std::size_t iEmptyBucket) const;

   /*! Returns the index of the bucket matching the specified key, or locates an empty bucket and
   returns its index after moving it in the key’s neighborhood.

   @param cbKey
      Size of a key, in bytes.
   @param cbValue
      Size of a value, in bytes.
   @param pfnKeysEqual
      Pointer to a function that returns true if two keys compare as equal.
   @param pfnMoveKeyValueToBucket
      Pointer to a function that move-constructs the key and value of a bucket using the provided
      pointers.
   @param pKey
      Pointer to the key to lookup.
   @param iKeyHash
      Hash of key.
   @return
      Index of the bucket for the specified key. If key is not already in the map and no empty
      bucket can be moved in key’s neighborhood, the returned index is smc_iNullIndex.
   */
   std::size_t get_existing_or_empty_bucket_for_key(
      std::size_t cbKey, std::size_t cbValue, keys_equal_fn pfnKeysEqual,
      move_key_value_to_bucket_fn pfnMoveKeyValueToBucket, void const * pKey, std::size_t iKeyHash
   );

   /*! Returns the neighborhood index (index of the first bucket in a neighborhood) for the given
   hash.

   @param iHash
      Hash to get the neighborhood index for.
   @return
      Index of the first bucket in the neighborhood.
   */
   std::size_t hash_neighborhood_index(std::size_t iHash) const {
      return iHash & (m_cBuckets - 1);
   }

   /*! Returns the bucket index ranges for the neighborhood of the given hash.

   @param iHash
      Hash to return the neighborhood of.
   @return
      Calculated range for the neighborhood bucket index.
   */
   std::tuple<std::size_t, std::size_t> hash_neighborhood_range(std::size_t iHash) const {
      std::size_t iNhBegin = hash_neighborhood_index(iHash);
      std::size_t iNhEnd = iNhBegin + m_cNeighborhoodBuckets;
      // Wrap the end index back in the table.
      iNhEnd &= m_cBuckets - 1;
      return std::make_tuple(iNhBegin, iNhEnd);
   }


   /*! Returns the current neighborhood size.

   @return
      Current neighborhood size.
   */
   std::size_t neighborhood_size() const {
      return m_cNeighborhoodBuckets;
   }

private:
   /*! Looks for an empty bucket in the specified bucket range.

   @param iNhBegin
      Beginning of the neighborhood bucket index range.
   @param iNhEnd
      End of the neighborhood bucket index range.
   @return
      Index of the first empty bucket found, or smc_iNullIndex if no empty buckets were found.
   */
   std::size_t find_empty_bucket(std::size_t iNhBegin, std::size_t iNhEnd) const;

   /*! Looks for a specific key or an unused bucket in the map.

   @param cbKey
      Size of a key, in bytes.
   @param pKey
      Pointer to the key to lookup.
   @param iKeyHash
      Hash of *pKey.
   @param pfnKeysEqual
      Pointer to a function that returns true if two keys compare as equal.
   @param iNhBegin
      Beginning of the neighborhood bucket index range.
   @param iNhEnd
      End of the neighborhood bucket index range.
   @return
      Index of the bucket at which the key could be found, or index of the first empty bucket found,
      or smc_iNullIndex if neither could be found.
   */
   std::size_t lookup_key_or_find_empty_bucket(
      std::size_t cbKey, void const * pKey, std::size_t iKeyHash, keys_equal_fn pfnKeysEqual,
      std::size_t iNhBegin, std::size_t iNhEnd
   ) const;

protected:
   //! Array containing the hash of each key.
   std::unique_ptr<std::size_t[]> m_piHashes;
   //! Array of keys.
   std::unique_ptr<std::max_align_t[]> m_pKeys;
   //! Array of buckets.
   std::unique_ptr<std::max_align_t[]> m_pValues;
   //! Count of total buckets. Always a power of two.
   std::size_t m_cBuckets;
   //! Count of elements / occupied buckets.
   std::size_t m_cUsedBuckets;
   /*! Neighborhood size. The map will try to keep this to smc_cIdealNeighborhoodBuckets, but the
   actual value may be smaller if the table is too small, or larger if the hash function results in
   too many collisions. In the worst case, this will be the same as m_cBuckets. */
   std::size_t m_cNeighborhoodBuckets;
   //! Minimum bucket count. Must be a power of 2.
   static std::size_t const smc_cBucketsMin = 8;
   //! Special hash value used to indicate that a bucket is empty.
   static std::size_t const smc_iEmptyBucketHash = 0;
   //! Hash table growth factor. Must be a power of 2.
   static std::size_t const smc_iGrowthFactor = 4;
   //! Neighborhood size.
   static std::size_t const smc_cIdealNeighborhoodBuckets = sizeof(std::size_t) * CHAR_BIT;
   //! Special index returned by several methods to indicate a logical “null index”.
   static std::size_t const smc_iNullIndex = numeric::max<std::size_t>::value;
   /*! Hash value substituted when the hash function returns 0; this is so we can use 0 (aliased by
   smc_iEmptyBucketHash) as a special value. This specific value is merely the largest prime number
   that will fit in 2^16, which is the (future, if ever) minimum word size supported by Abaclade. */
   static std::size_t const smc_iZeroHash = 65521;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::map

namespace abc {

//! Key/value map using a simplified hopscotch hashing collision resolution algorithm.
template <
   typename TKey,
   typename TValue,
   typename THasher = std::hash<TKey>,
   typename TKeyEqual = std::equal_to<TKey>
>
class map : public detail::map_impl, private THasher, private TKeyEqual {
public:
   //! Hash generator for TKey.
   typedef THasher hasher;
   //! Functor that can compare two TKey instances for equality.
   typedef TKeyEqual key_equal;

   //! Iterator type.
   class iterator {
   public:
      /*! Constructor.

      @param pmap
         Pointer to the map owning the iterated objects.
      @param iBucket
         Index of the current bucket.
      */
      iterator(map const * pmap, std::size_t iBucket) :
         mc_pmap(pmap),
         m_iBucket(iBucket) {
      }

   private:
      //! Pointer to the map to iterate over.
      map const * const mc_pmap;
      //! Current bucket index.
      std::size_t m_iBucket;
   };

public:
   /*! Constructor.

   @param m
      Source object.
   */
   map() {
   }
   map(map && m) :
      detail::map_impl(std::move(m)) {
   }

   //! Destructor.
   ~map() {
      clear();
   }

   /*! Assignment operator.

   @param m
      Source object.
   */
   map & operator=(map && m) {
      detail::map_impl::operator=(std::move(m));
      return *this;
   }

   /*! Element lookup operator.

   @param key
      Key to lookup.
   @return
      Value corresponding to key. If key is not in the map, an exception will be thrown.
   */
   TValue & operator[](TKey const & key) const {
      std::size_t iBucket = lookup_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      return *value_ptr(iBucket);
   }

   /*! Adds a key/value pair to the map, overwriting the value if key is already associated to one.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Pair containing an iterator to the newly added key/value, and a bool value that is true if the
      key/value pair was just added, or false if the key already existed in the map and the
      corresponding value was overwritten.
   */
   std::pair<iterator, bool> add(TKey key, TValue value) {
      std::size_t iKeyHash = calculate_and_adjust_hash(key), iBucket;
      if (!m_cBuckets) {
         grow_table();
      }
      /* Repeatedly resize the table until we’re able to find an empty bucket for the new element.
      This should really only happen at most once. */
      for (;;) {
         iBucket = get_existing_or_empty_bucket_for_key(
            sizeof(TKey), sizeof(TValue), &keys_equal, &move_key_value_to_bucket, &key, iKeyHash
         );
         if (iBucket != smc_iNullIndex) {
            break;
         }
         grow_table();
      }

      std::size_t * piHash = &m_piHashes[iBucket];
      bool bNew = (*piHash == smc_iEmptyBucketHash);
      if (bNew) {
         // The bucket is currently empty, so initialize it with hash/key/value.
         move_key_value_to_bucket(this, &key, &value, iBucket);
         *piHash = iKeyHash;
      } else {
         // The bucket already has a value, so overwrite that with the value argument.
         *value_ptr(iBucket) = std::move(value);
      }
      ++m_cUsedBuckets;
      return std::make_pair(iterator(this, iBucket), bNew);
   }

   /*! Returns a forward iterator set to the first key/value pair.

   @return
      Forward iterator to the first key/value pair.
   */
   iterator begin() {
      return iterator(this, 0);
   }

   //! Removes all elements from the map.
   void clear() {
      std::size_t * piHash = m_piHashes.get(), * piHashesEnd = piHash + m_cBuckets;
      TKey   * pkey   = key_ptr  (0);
      TValue * pvalue = value_ptr(0);
      for (; piHash < piHashesEnd; ++piHash, ++pkey, ++pvalue) {
         if (*piHash != smc_iEmptyBucketHash) {
            *piHash = smc_iEmptyBucketHash;
            destruct_key_value(pkey, pvalue);
         }
      }
      m_cUsedBuckets = 0;
   }

   /*! Returns a forward iterator set beyond the last key/value pair.

   @return
      Forward iterator to the first key/value pair.
   */
   iterator end() {
      return iterator(this, m_cBuckets);
   }

   /*! Removes a key/value pair given the key, which must be in the map.

   @param key
      Key associated to the value to remove.
   */
   void remove(TKey const & key) {
      std::size_t iBucket = lookup_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      // Mark the bucket as empty and destruct the corresponding key and value.
      --m_cUsedBuckets;
      m_piHashes[iBucket] = smc_iEmptyBucketHash;
      destruct_key_value(key_ptr(iBucket), value_ptr(iBucket));
   }

private:
   /*! Calculates, adjusts and returns the hash value for the specified key.

   @param key
      Key to calculate a hash value for.
   @return
      Hash value of key.
   */
   std::size_t calculate_and_adjust_hash(TKey const & key) const {
      std::size_t iHash = hasher::operator()(key);
      return iHash == smc_iEmptyBucketHash ? smc_iZeroHash : iHash;
   }

   /*! Destructs a key and a value.

   @param pKey
      Pointer to the key to destruct.
   @param pValue
      Pointer to the value to destruct.
   */
   static void destruct_key_value(void * pKey, void * pValue) {
      static_cast<TKey   *>(pKey  )->~TKey  ();
      static_cast<TValue *>(pValue)->~TValue();
   }

   /*! Enlarges the hash table by a factor of smc_iGrowthFactor. The contents of each bucket are
   moved from the old arrays to new temporary ones, and the two array sets are then swapped.

   The bucket contents transfer work is done by reusing functions that obtain the arrays to operate
   on via member variables. In the assumption that transferring the contents of a bucket won’t throw
   because it only involves move-constructions and destructions, we optimistically update the member
   variables as soon as all memory allocations are done; if anything were to go wrong after that,
   we’d have no guaranteed-safe way of recovering from a half-transferred scenario anyway. */
   void grow_table() {
      // The “old” names of these four variables will make sense in a moment…
      std::size_t cOldBuckets = m_cBuckets ? m_cBuckets * smc_iGrowthFactor : smc_cBucketsMin;
      std::unique_ptr<std::size_t[]> piOldHashes(new std::size_t[cOldBuckets]);
      std::unique_ptr<std::max_align_t[]> pOldKeys(
         new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TKey) * cOldBuckets)]
      );
      std::unique_ptr<std::max_align_t[]> pOldValues(
         new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TValue) * cOldBuckets)]
      );
      // At this point we’re safe from exceptions, so we can update the member variables.
      std::swap(m_cBuckets, cOldBuckets);
      std::swap(m_piHashes, piOldHashes);
      std::swap(m_pKeys,    pOldKeys);
      std::swap(m_pValues,  pOldValues);
      // Now the names of these variables make sense :)

      /* Recalculate the neighborhood size. The (missing) “else” to this “if” is for when the actual
      neighborhood size is greater than the ideal, which can happen when dealing with a subpar hash
      function that resulted in more collisions than smc_cIdealNeighborhoodBuckets. In that
      scenario, the table size increase doesn’t change anything, since the fix has already been
      applied to m_cNeighborhoodBuckets, which we won’t change here. */
      if (m_cNeighborhoodBuckets < smc_cIdealNeighborhoodBuckets) {
         if (m_cBuckets < smc_cIdealNeighborhoodBuckets) {
            /* m_cNeighborhoodBuckets has not yet reached smc_cIdealNeighborhoodBuckets, but it
            can’t exceed m_cBuckets, so set it to the latter. */
            m_cNeighborhoodBuckets = m_cBuckets;
         } else {
            // Fix m_cNeighborhoodBuckets to its ideal value.
            m_cNeighborhoodBuckets = smc_cIdealNeighborhoodBuckets;
         }
      }

      // Initialize piNewHashes[i] with smc_iEmptyBucketHash.
      memory::clear(m_piHashes.get(), m_cBuckets);
      // Re-insert each hash/key/value triplet to move it from the old arrays to the new ones.
      std::size_t * piOldHash = piOldHashes.get(), * piOldHashesEnd = piOldHash + cOldBuckets;
      TKey   * pOldKey   = reinterpret_cast<TKey   *>(pOldKeys  .get());
      TValue * pOldValue = reinterpret_cast<TValue *>(pOldValues.get());
      for (; piOldHash < piOldHashesEnd; ++piOldHash, ++pOldKey, ++pOldValue) {
         if (*piOldHash != smc_iEmptyBucketHash) {
            std::size_t iNewBucket = get_existing_or_empty_bucket_for_key(
               sizeof(TKey), sizeof(TValue), &keys_equal, &move_key_value_to_bucket,
               pOldKey, *piOldHash
            );
            ABC_ASSERT(
               iNewBucket != smc_iNullIndex,
               ABC_SL("failed to find empty bucket while growing hash table")
            );

            // Move hash/key/value to the new bucket.
            move_key_value_to_bucket(this, pOldKey, pOldValue, iNewBucket);
            m_piHashes[iNewBucket] = *piOldHash;
            destruct_key_value(pOldKey, pOldValue);
         }
      }
   }

   /*! Looks for a specific key in the map.

   @param key
      Key to lookup.
   @return
      Index of the bucket at which the key could be found, or smc_iNullIndex if the key could not be
      found.
   */
   std::size_t lookup_key(TKey const & key) const {
      std::size_t iKeyHash = calculate_and_adjust_hash(key);
      if (m_cBuckets == 0) {
         // The key cannot possibly be in the map.
         return smc_iNullIndex;
      }
      std::size_t iNhBegin, iNhEnd;
      std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);

      std::size_t const * piHash      = m_piHashes.get() + iNhBegin,
                        * piHashNhEnd = m_piHashes.get() + iNhEnd,
                        * piHashesEnd = m_piHashes.get() + m_cBuckets;
      /* iNhBegin - iNhEnd may be a wrapping range, so we can only test for inequality and rely on
      the wrap-around logic at the end of the loop body. Also, we need to iterate at least once,
      otherwise we won’t enter the loop at all if the start condition is the same as the end
      condition, which is the case for m_cNeighborhoodBuckets == m_cBuckets. */
      do {
         /* Multiple calculations of the second condition should be rare enough (exact key match or
         hash collision) to make recalculating the offset from m_pKeys cheaper than keeping a cursor
         over m_pKeys running in parallel to piHash. */
         if (*piHash == iKeyHash) {
            std::size_t iBucket = static_cast<std::size_t>(piHash - m_piHashes.get());
            if (key_equal::operator()(*key_ptr(iBucket), key)) {
               return iBucket;
            }
         }

         // Move on to the next bucket, wrapping around to the first one if needed.
         if (++piHash == piHashesEnd) {
            piHash = m_piHashes.get();
         }
      } while (piHash != piHashNhEnd);
      return smc_iNullIndex;
   }

   /*! Returns a pointer to the key in the specified bucket index.

   @param i
      Bucket index.
   @return
      Pointer to the key.
   */
   TKey * key_ptr(std::size_t i) const {
      return reinterpret_cast<TKey *>(m_pKeys.get()) + i;
   }

   /*! Compares two keys for equality. Static helper used by detail::map_impl.

   @param pThis
      Pointer to *this.
   @param pKey1
      Pointer to the first key to compare.
   @param pKey2
      Pointer to the second key to compare.
   @return
      true if the two keys compare as equal, or false otherwise.
   */
   static bool keys_equal(map_impl const * pmapi, void const * pKey1, void const * pKey2) {
      map const * pmap = static_cast<map const *>(pmapi);
      return pmap->key_equal::operator()(
         *static_cast<TKey const *>(pKey1), *static_cast<TKey const *>(pKey2)
      );
   }

   /*! Moves a key and a value to the specified bucket.

   @param pThis
      Pointer to *this.
   @param pKey
      Pointer to the key to move to the destination bucket.
   @param pValue
      Pointer to the value to move to the destination bucket.
   @param iBucket
      Index of the destination bucket.
   */
   static void move_key_value_to_bucket(
      map_impl * pmapi, void * pKey, void * pValue, std::size_t iBucket
   ) {
      map * pmap = static_cast<map *>(pmapi);
      new(pmap->  key_ptr(iBucket)) TKey  (std::move(*static_cast<TKey   *>(pKey  )));
      new(pmap->value_ptr(iBucket)) TValue(std::move(*static_cast<TValue *>(pValue)));
   }

   /*! Returns a pointer to the value in the specified bucket index.

   @param i
      Bucket index.
   @return
      Pointer to the value.
   */
   TValue * value_ptr(std::size_t i) const {
      return reinterpret_cast<TValue *>(m_pValues.get()) + i;
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_MAP_HXX

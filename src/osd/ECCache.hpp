// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2016 Western Digital Corporation.
 * Author: Tomy Cheru <tomy.cheru@sandisk.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#ifndef ECCACHE_HPP_INCLUDED
#define ECCACHE_HPP_INCLUDED

#include <iostream>
//#include <string>
#include <set>
#include "include/buffer.h"
#include "common/hobject.h"
#include "common/Mutex.h"
#include "common/Cond.h"

class ecc_extent
{
  private:
    unsigned eoff;
    bufferlist edata;

  public:
    bool operator < (const ecc_extent& r) const
    {
      return eoff < r.eoff;
    }

    bool operator == (const ecc_extent& r) const
    {
      return eoff == r.eoff;
    }

    ecc_extent(unsigned o = 0,bufferlist d = {} )
    {
      eoff = o;
      edata.claim(d) ;
    };

    ~ecc_extent(){};

    unsigned offset() const
    {
      return eoff;
    }

    unsigned length() const
    {
      return edata.length();
    }

    bufferlist data() const
    {
      return edata;
    }

    unsigned start() const
    {
      return eoff;
    }

    unsigned end() const
    {
      return eoff + edata.length();
    }
};

typedef std::set<ecc_extent> extents;
typedef std::set<extents> extents_set;

class ecc_object
{
  private:
    bool evictable;
    std::string key;
    extents e;

  public:
    bool operator < (const class ecc_object& r) const
    {
      return key < r.key;
    }

    bool operator == (const class ecc_object& r) const
    {
      return key == r.key;
    }

    ecc_object(std::string k = "", bool eflag = false, extents ein = {})
    {
      evictable = eflag;
      key.assign(k);
      e = ein;
    }

    ~ecc_object(){}

    std::string get_key()
    {
      return key;
    }

    void evict()
    {
      evictable = true;
    }

    bool is_evictable()
    {
      return evictable;
    }

    int put_extent(unsigned o, bufferlist d)
    {
      unsigned l(d.length());
      unsigned nso(o), neo(o+l-1);
      bufferlist nd;

      extents::iterator low = e.upper_bound(o);
      extents::iterator up = e.upper_bound(o+l-1);

      if((!e.empty()) && low != e.begin()){
	--low;
	nso = (low->end() >= nso) ? low->start() : nso;
	nd.substr_of(low->data(), 0, o-nso);
      }
      bufferlist tmp;
      tmp.substr_of(d, 0, l);
      nd.append(tmp);
      if((!e.empty()) && up != e.begin()){
	--up;
	neo = (up->end() > neo) ? up->end() : neo;
	bufferlist tmp1;
	tmp1.substr_of(up->data(), (up->length() - (neo - (o+l-1))), (neo - (o+l-1)));
	nd.append(tmp1);
	++up;
      }
      e.erase(low, up);
      ecc_extent tmp_extent(nso, nd);
      e.insert(tmp_extent);
      return (l);
    }

    int get_extent( unsigned o, unsigned l, extents *out)
    {
      bufferlist tmp;
      unsigned t, r = l;

      for(extents::iterator i = e.begin(); i != e.end(); i++){
	if(i->end() < o) continue;
	if(i->start() < o){
	  t = ((i->length()) - (o-(i->start())));
	  t = t > r ? r : t;
	  tmp.substr_of(i->data(), o-(i->start()), t);
	  ecc_extent tmp_extent(o, tmp);
	  out->insert(tmp_extent);
	  r -=t;
	  continue;
	}
	if(i->end() > (o+l)){
	  t = (i->length() - (i->end() - (o+l)));
	  t = t > r ? r : t;
	  tmp.substr_of(i->data(), 0, t);
	  ecc_extent tmp_extent(i->offset(), tmp);
	  out->insert(tmp_extent);
	  r -=t;
	  break;
	}
	t = i->length();
	t = t > r ? r : t;
	tmp.substr_of(i->data(), 0, t);
	ecc_extent tmp_extent(i->offset(), tmp);
	out->insert(tmp_extent);
	r -=t;
      }
      return l-r;
    }
};

typedef std::set<ecc_object> Cache;

class ECCache
{
  private:
    Mutex lock;
    uint64_t current_cap;
    uint64_t cap;
    uint64_t high_threshold;
    uint64_t low_threashold;
    Cache cache;

  public:
    ECCache(uint64_t c = 104857600, uint64_t ht = 98304000, uint64_t lt = 52428800) : 
      lock("ECCache::lock"), current_cap(0), cap(c), high_threshold(ht), low_threashold(lt) {
      cache.clear();
    }

    ~ECCache(){}

    int set_evict(std::string key)
    {
      //Mutex::Locker l(lock);
      Cache::iterator Cit = cache.find(key);
      ecc_object TmpCacheObj = *Cit;
      cache.erase(key);
      TmpCacheObj.evict();
      cache.insert(TmpCacheObj);
      return 0;
    }

    void trim()
    {
      //Mutex::Locker l(lock);
      ecc_object TmpCacheObj;
      for(Cache::iterator i = cache.begin(); i!=cache.end(); i++){
	TmpCacheObj = *i;
	if(TmpCacheObj.is_evictable()){
	  //        current_cap += i->bl.length();
	  cache.erase(TmpCacheObj.get_key());
	  //        if(current_cap <= low_threashold) break;
	}
      }
    }

    int put(std::string key, unsigned off, bufferlist data)
    {
      lock.Lock();
      uint64_t ret = 0;
      if((current_cap + data.length()) > high_threshold){
	trim();
      }

      Cache::iterator Cit = cache.find(key);
      if(Cit != cache.end()){
	ecc_object TmpCacheObj = *Cit;
	cache.erase(key);
	ret = TmpCacheObj.put_extent(off, data);
	cache.insert(TmpCacheObj);
	current_cap += ret;
	lock.Unlock();
      } else {
	ecc_object TmpCacheObj(key, false, {});
	cache.insert(TmpCacheObj);
	lock.Unlock();
	put(key, off, data);

      }
      return ret;
    }

    int get(std::string key, unsigned off, unsigned len, extents *out)
    {
      //Mutex::Locker l(lock);
      uint64_t ret = 0;
      Cache::iterator Cit = cache.find(key);
      if(Cit != cache.end()){
	ecc_object TmpCacheObj = *Cit;
	ret = TmpCacheObj.get_extent(off, len, out);
      }
      return ret;
    }
};
#endif // ECCACHE_HPP_INCLUDED

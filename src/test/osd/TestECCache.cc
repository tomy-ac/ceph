// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2016 Western Digital Corporation.
 *
 * Author: Tomy Cheru <tomy.cheru@sandisk.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 */

#include <errno.h>
#include <gtest/gtest.h>
#include <osd/ECCache.hpp>
#include <include/buffer.h>

using namespace std;

TEST(ecsc, basic_insert) {
  ECCache ecsc(1000, 1000);

  bufferlist bl;
  char d[91] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
  bl.append(d, 90);

  bufferlist tmp;

  tmp.substr_of(bl, 30, 10);

  bufferlist key;
  key.substr_of(bl, 0, 10);
  ecsc.put(key.c_str(), 30, tmp);

  extents out;
  uint64_t ret = ecsc.get(key.c_str(), 30, 10, &out);
  EXPECT_NE(ret, (unsigned)0);
  if(ret !=0) EXPECT_EQ(tmp, out.begin()->data());
}

/*
 * Local Variables:
 * compile-command: "cd ../.. ; make -j4 &&
 *   make unittest_osd_eccache &&
 *   valgrind --tool=memcheck --leak-check=full \
 *      ./unittest_osd_eccache
 *   "
 * End:
 */


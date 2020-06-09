/*
 * Copyright 2020 Rene Sugar. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @file pypph.cpp
 * @author Rene Sugar <rene.sugar@gmail.com>
 * @brief Creates an order-preserving perfect hash function for a list of strings
 */

#include "pph.h"

#include <cstdint>
#include <cfloat>
#include <random>
#include <ctime>
#include <limits>
#include <vector>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <string>

#include "pybind11/stl.h"
#include "pybind11/iostream.h"

namespace py = pybind11;

class PphKeyFunctions {
public:
  PphKeyFunctions();
  ~PphKeyFunctions();

  std::vector<std::string> getKeys();

  std::string name(const std::string& uuid);

private:
  std::vector<std::string>  m_uuids;
};

class PphRandomNumber {
public:
  PphRandomNumber() : random_gen(clock()), dist(0, UINT64_MAX) {
  }

  ~PphRandomNumber() {
  }

  uint64_t next();

private:
  pph::XorShift1024Star random_gen;

  std::uniform_int_distribution<uint64_t> dist;
};

class PphHashTable {
public:
  PphHashTable();

  ~PphHashTable();

  std::vector<std::string> getKeys();

  std::string& getUuid();

  void setUuid(std::string& uuid);

  double getLoadingFactor();

  void setLoadingFactor(double value);

  uint64_t getTimeout();

  void setTimeout(uint64_t value);

  uint64_t getSeed();

  void setSeed(uint64_t value);

  uint64_t getMultiplier();

  void setMultiplier(uint64_t value);

  uint64_t getAdjustment();

  void setAdjustment(uint64_t value);

  bool contains(std::string& key);

  py::object getitem(std::string& key);

  void setitem(std::string& key, py::object value);

  void delitem(std::string& key);

  bool load(py::object pystream);

  bool save(py::object pystream);

  // Call initialize() before calling getitem()
  bool initialize();

private:
  pph::Table *              m_table;
  std::vector<std::string>  m_keys;
  std::vector<uint64_t>     m_index_values;
  std::vector<py::object>   m_values;

  std::string               m_uuid;
  bool                      m_use_loading_factor;
  double                    m_loading_factor;
  uint64_t                  m_timeout;
  uint64_t                  m_seed;
  uint64_t                  m_multiplier;
  uint64_t                  m_adjustment;
  bool                      m_initialized;
};
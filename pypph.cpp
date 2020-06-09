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

#include "pypph.h"

PphKeyFunctions::PphKeyFunctions() {
  m_uuids.push_back("F80F007A-26C3-4BD0-A481-24EE9AE94D01"); // "crc64"
  m_uuids.push_back("BCC54D42-34F0-43FF-88EB-59C7B47EE210"); // "djb_hash"
  m_uuids.push_back("87333E59-7C1A-4613-9C6F-81F1BB1F6AED"); // "fnv64a_hash"
  m_uuids.push_back("3AC2A805-6771-4189-8C62-5F41297126FE"); // "oat_hash"
  m_uuids.push_back("A647F03D-A02E-477F-9635-420F3BCEB394"); // "spookyV2_hash"
}

PphKeyFunctions::~PphKeyFunctions() {
}

std::vector<std::string> PphKeyFunctions::getKeys() {
  return this->m_uuids;
}

std::string PphKeyFunctions::name(const std::string& uuid) {
  if (uuid == "F80F007A-26C3-4BD0-A481-24EE9AE94D01") {
    return "crc64";
  } else if (uuid == "BCC54D42-34F0-43FF-88EB-59C7B47EE210") {
    return "djb_hash";
  } else if (uuid == "87333E59-7C1A-4613-9C6F-81F1BB1F6AED") {
    return "fnv64a_hash";
  } else if (uuid == "3AC2A805-6771-4189-8C62-5F41297126FE") {
    return "oat_hash";
  } else if (uuid == "A647F03D-A02E-477F-9635-420F3BCEB394") {
    return "spookyV2_hash";
  }

  return "unknown";
}

uint64_t PphRandomNumber::next() {
  return dist(random_gen);
}

PphHashTable::PphHashTable() {
  m_table              = new pph::Table();
  m_uuid               = "BCC54D42-34F0-43FF-88EB-59C7B47EE210"; // djb_hash
  m_use_loading_factor = false;
  m_loading_factor     = pph::DEFAULT_LOADING_FACTOR;
  m_timeout            = pph::DEFAULT_TIMEOUT;
  m_seed               = 0;
  m_multiplier         = pph::HASH_MULTIPLIER;
  m_adjustment         = 0;
  m_initialized        = false;
}

PphHashTable::~PphHashTable() {
  delete this->m_table;
}

std::vector<std::string> PphHashTable::getKeys() {
  // return list of keys (wrap std::vector in Python list)
  return this->m_keys;
}

std::string& PphHashTable::getUuid() {
  return this->m_uuid;
}

void PphHashTable::setUuid(std::string& uuid) {
  this->m_uuid = uuid;
}

double PphHashTable::getLoadingFactor() {
  return this->m_loading_factor;
}

void PphHashTable::setLoadingFactor(double value) {
  this->m_loading_factor = value;
}

uint64_t PphHashTable::getTimeout() {
  return this->m_timeout;
}

void PphHashTable::setTimeout(uint64_t value) {
  this->m_timeout = value;
}

uint64_t PphHashTable::getSeed() {
  return this->m_seed;
}

void PphHashTable::setSeed(uint64_t value) {
  this->m_seed = value;
}

uint64_t PphHashTable::getMultiplier() {
  return this->m_multiplier;
}

void PphHashTable::setMultiplier(uint64_t value) {
  this->m_multiplier = value;
}

uint64_t PphHashTable::getAdjustment() {
  return this->m_adjustment;
}

void PphHashTable::setAdjustment(uint64_t value) {
  this->m_adjustment = value;
}

bool PphHashTable::contains(std::string& key) {
  int val = this->m_table->find_val(key);
  if (this->m_table->notfound_val(val)) {
    return false;
  }
  return true;
}

py::object PphHashTable::getitem(std::string& key) {
  if (this->m_initialized == false) {
    throw pybind11::key_error();
  }

  uint64_t val = this->m_table->find_val(key);
  if (this->m_table->notfound_val(val)) {
    // https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html
    throw pybind11::key_error();
  }
  return this->m_values[val];
}

void PphHashTable::setitem(std::string& key, py::object value) {
  py::gil_scoped_acquire gil;
  if (this->m_initialized == true) {
    throw pybind11::value_error();
  }

  m_keys.push_back(key);
  m_values.push_back(value);
}

void PphHashTable::delitem(std::string& key) {
  // Hash table is loaded and then built; values do not change
  throw pybind11::key_error();
}

bool PphHashTable::load(py::object pystream) {
  if (this->m_initialized == true) {
    // Cannot reuse hash table to create a new one once it has been initialized.
    return false;
  }
  this->m_initialized = true;
  if (!(py::hasattr(pystream, "readinto") && py::hasattr(pystream, "write") && py::hasattr(pystream, "flush"))) {
      throw py::type_error("PphHashTable::load(pystream): incompatible function argument:  `pystream` must be a file-like object, but `"
                          + (std::string)(py::repr(pystream)) + "` provided");
  }
  py::detail::ipythonbuf buf(pystream);
  std::istream cpp_stream(&buf);
  bool status = m_table->unserialize(cpp_stream);
  if (status == true) {
    uint64_t val = 0;

    this->m_keys.reserve(m_table->keys().size());
    this->m_keys.resize(m_table->keys().size());

    this->m_index_values.reserve(m_table->keys().size());
    this->m_index_values.resize(m_table->keys().size());

    this->m_values.reserve(m_table->keys().size());
    this->m_values.resize(m_table->keys().size());

    for (uint64_t i = 0; i < m_table->keys().size(); i++) {
      val = m_table->find_val(m_table->keys()[i]);

      if (m_table->notfound_val(val)) {
        return false;
      }
      m_keys.at(val) = m_table->keys()[i];
      m_index_values.at(val) = val;
      // Only index values are stored in the hash table
      m_values.at(val) = py::int_(val);
    }    
  }
  return status;
}

bool PphHashTable::save(py::object pystream) {
  if (this->m_initialized == false) {
    // Cannot save hash table that has not been initialized.
    return false;
  }
  if (!(py::hasattr(pystream, "write") && py::hasattr(pystream, "flush"))) {
      throw py::type_error("PphHashTable::load(pystream): incompatible function argument:  `pystream` must be a file-like object, but `"
                          + (std::string)(py::repr(pystream)) + "` provided");
  }
  // https://pybind11.readthedocs.io/en/stable/reference.html#redirecting-c-streams
  // py::scoped_ostream_redirect output{std::cout, pystream};
  // return m_table->serialize(std::cout);
  py::detail::pythonbuf buf(pystream);
  std::ostream cpp_stream(&buf);
  return m_table->serialize(cpp_stream);
}

// Call initialize() before calling getitem()
bool PphHashTable::initialize() {
  py::gil_scoped_acquire gil;
  if (this->m_initialized == true) {
    // Cannot reuse hash table to create a new one once it has been initialized.
    return false;
  }
  this->m_initialized = true;
  pph::keyfunc_t keyfunc = pph::uuid_to_keyfunc(this->m_uuid);
  this->m_table->setup(m_keys.size(),
                      this->m_use_loading_factor,
                      this->m_loading_factor,
                      this->m_timeout,
                      this->m_seed,
                      this->m_multiplier,
                      this->m_adjustment,
                      keyfunc);
  this->m_table->set_uuid(this->m_uuid);
  this->m_index_values.reserve(m_keys.size());
  this->m_index_values.resize(m_keys.size());
  std::iota(std::begin(this->m_index_values), std::end(this->m_index_values), 0);
  return this->m_table->load(this->m_keys, this->m_index_values);
}


PYBIND11_MODULE(pph, m) {
  m.doc() = "Practical Perfect Hashing module";

  py::class_<PphKeyFunctions>(m, "PphKeyFunctions")
    .def(py::init<>())
    .def_property("keys", &PphKeyFunctions::getKeys, nullptr)
    .def("name", &PphKeyFunctions::name)
    ;

  py::class_<PphRandomNumber>(m, "PphRandomNumber")
    .def(py::init<>())
    .def("next", &PphRandomNumber::next)
    ;

  py::class_<PphHashTable>(m, "PphHashTable")
    .def(py::init<>())
    .def_property("keys", &PphHashTable::getKeys, nullptr)
    .def_property("key_function_uuid", &PphHashTable::getUuid, &PphHashTable::setUuid)
    .def_property("loading_factor", &PphHashTable::getLoadingFactor, &PphHashTable::setLoadingFactor)
    .def_property("timeout", &PphHashTable::getTimeout, &PphHashTable::setTimeout)
    .def_property("seed", &PphHashTable::getSeed, &PphHashTable::setSeed)
    .def_property("multiplier", &PphHashTable::getMultiplier, &PphHashTable::setMultiplier)
    .def_property("adjustment", &PphHashTable::getAdjustment, &PphHashTable::setAdjustment)
    .def("__contains__", &PphHashTable::contains)
    .def("__getitem__", &PphHashTable::getitem)
    .def("__setitem__", &PphHashTable::setitem)
    .def("__delitem__", &PphHashTable::delitem)
    .def("load", &PphHashTable::load)
    .def("save", &PphHashTable::save)
    .def("initialize", &PphHashTable::initialize)
    ;

  m.attr("__version__") = py::make_tuple(0, 1, 0, "alpha", 0);
  m.attr("__version__") = "0.1.0";
  m.attr("__author__")  = "Rene Sugar";
  m.attr("__email__")   = "rene.sugar@gmail.com";
}
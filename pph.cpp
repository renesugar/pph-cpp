/*
 * Copyright 2017 Rene Sugar. All rights reserved.
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
 * @file pph.cpp
 * @author Rene Sugar <rene.sugar@gmail.com>
 * @brief Creates an order-preserving perfect hash function for a list of strings
 */

#include "pph.h"

#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

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

#define STR(x) #x
#define STR_(x) STR(x)
#define STR__LINE__ STR_(__LINE__)

#include "pphrelease.h"
//#define PPH_RELEASE "1.0.0"

int main(int argc, const char** argv) {
  int retval = 0;

  // File options
  std::vector<std::string> input_files;
  std::string              config_file("hash.conf");
  std::string              table_filename("table.hash");
  std::string              output_filename("output.hash");

  std::ifstream            table_file;
  std::ifstream            input_file;
  std::ofstream            output_file;

  uint64_t                 n          = 0;
  uint64_t                 count      = 0;
  uint64_t                 multiplier = pph::HASH_MULTIPLIER;
  uint64_t                 adjustment = 0;
  uint64_t                 timeout    = 60000;
  uint64_t                 seed       = 0;
  uint64_t                 skip       = 0;
  uint64_t                 rows       = 0;

  std::string              uuid       = "BCC54D42-34F0-43FF-88EB-59C7B47EE210";
  double                   p          = 0.97;
  bool                     use_p      = false;

  pph::Table               table;

  std::vector<std::string> keys;
  std::vector<uint64_t>    values;

  namespace po = boost::program_options;

  pph::XorShift1024Star random_gen(clock());

  std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
  seed = dist(random_gen);

  po::options_description desc("Options");
  desc.add_options()("help,h", "Print help messages");
  desc.add_options()("version,v", "Print release version number");
  desc.add_options()("index", "Print index for each key");
  desc.add_options()("config", po::value<std::string>(&config_file), "Path to pph.conf");
  desc.add_options()("input,i", po::value<std::vector<std::string>>(), "Path to data file(s)");
  desc.add_options()("output,o", po::value<std::string>(&output_filename)->required()->default_value("output"), "Path to table output file");
  desc.add_options()("verify", po::value<std::string>(&table_filename), "Path to table file to verify");

  // Declare a group of options that will be allowed both on command line and in the config file
  po::options_description config("Configuration");
  config.add_options()("uuid,U", po::value<std::string>(&uuid), "UUID of key hash function");
  config.add_options()("p,P", po::value<double>(&p), "Loading factor");
  config.add_options()("timeout,T",
                       po::value<uint64_t>(&timeout)->default_value(timeout)->implicit_value(60000),
                       "Timeout for how long to attempt creating a hash table");
  config.add_options()("seed,S",
                       po::value<uint64_t>(&seed)->default_value(seed),
                       "Seed for random number generator used to create a hash table");
  config.add_options()("multiplier,M",
                       po::value<uint64_t>(&multiplier)->default_value(multiplier)->implicit_value(pph::HASH_MULTIPLIER),
                       "Multiplier for key hash function");
  config.add_options()("adjustment,A",
                       po::value<uint64_t>(&adjustment)->default_value(adjustment)->implicit_value(0),
                       "Adjustment for key hash functions");
  config.add_options()("skip,S",
                       po::value<uint64_t>(&skip)->default_value(skip)->implicit_value(0),
                       "Number of rows to skip in input file");
  config.add_options()("rows,R",
                       po::value<uint64_t>(&skip)->default_value(skip)->implicit_value(0),
                       "Number of rows to read in input file");

  po::positional_options_description positionalOptions;
  positionalOptions.add("input", -1);

  po::options_description desc_all("All options");
  desc_all.add(desc).add(config);

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).options(desc_all).positional(positionalOptions).run(), vm);
    po::notify(vm);

    if (vm.count("config")) {
      std::ifstream settings_file(config_file);
      po::store(po::parse_config_file(settings_file, desc_all, true), vm);
      po::notify(vm);
      settings_file.close();
    }

    if (vm.count("help")) {
      std::cout << "Usage: pph <input file(s)> [--config <config file>] [--verify <table file>] " << std::endl;
      std::cout << "           [--output <output file>] [--version|-v] [--timeout <timeout>]" << std::endl;
      std::cout << "           [--uuid <uuid>] [--multiplier <multiplier>] [--adjustment <adjustment>]" << std::endl;
      std::cout << std::endl
      << std::endl;
      std::cout << desc
      << std::endl
      << std::endl;
      std::cout << config << std::endl;
      return 0;
    }

    if (vm.count("version")) {
      std::cout << "pph version: " << PPH_RELEASE << std::endl;
      return 0;
    }

    if (vm.count("verify")) {
      // open the hash table file
      table_file.open(table_filename, std::ifstream::in);

      // default to reading from std::cin
      std::istream table_stream(std::cin.rdbuf());

      // read from file if it exists
      if (table_file) {
        table_stream.rdbuf(table_file.rdbuf());
      }

      // unserialize the hash function

      table.unserialize(table_stream);

      // Test generated table

      try {
        uint64_t val = 0;

        for (int i = 0; i < table.keys().size(); i++) {
          val = table.find_val(table.keys()[i]);

          if (table.notfound_val(val)) {
            std::cerr << "Error verifying key '" << table.keys()[i] << "' at index " << i << std::endl;
            return -1;
          }
        }
      } catch (const std::exception& e) {
        std::cerr << "Testing hash function error: " << e.what() << std::endl;
        return -1;
      }

      std::cout << "Hash function verified; loaded from " << table_filename << std::endl;

      // close the table file

      table_file.close();

      return 0;
    }

    if (vm.count("p")) {
      use_p = true;
    }

    if (vm.count("input")) {
      std::cerr << "Input files are: " << std::endl;

      std::vector<std::string> temp = vm["input"].as<std::vector<std::string>>();

      for (int i = 0; i < temp.size(); i++) {
        input_files.push_back(temp[i]);

        std::cerr << temp[i] << std::endl;

        const auto in_file = boost::filesystem::path(temp[i]);
        if (!boost::filesystem::exists(in_file)) {
          std::cerr << std::endl << "Input file '" << temp[i] << "' does not exist." << std::endl;
          return 1;
        }
      }
    }
  } catch (boost::program_options::error& e) {
    std::cerr << "Usage Error: " << e.what() << std::endl;
    return 1;
  }

  // open the output file
  output_file.open(output_filename, std::ofstream::out);

  // default to writing to std::cout
  std::ostream output_stream(std::cout.rdbuf());

  // write to file if it exists
  if (output_file) {
    output_stream.rdbuf(output_file.rdbuf());
  }

  //try
  {
    std::string line("");

    // Read keys from all input files

    count = 0;

    for (int i = 0; i < input_files.size(); i++) {
      std::ifstream input_file(input_files[i]);

      while (std::getline(input_file, line)) {
        line = pph::trim(line);

        if (line.empty())
          break;

        if (n < skip) {
          n++;
          continue;
        }

        // line number
        n++;

        keys.push_back(line);

        values.push_back(count);

        // row count
        count++;

        // check if row limit (if there is one) has been reached
        if ((rows > 0) && (count >= rows))
          break;
      }

      input_file.close();
    }
  } /*catch (const std::exception& e) {
    std::cerr << "Error reading keys from input file: " << e.what() << std::endl;
    return -1;
  }*/

  // setup the table for hash function generation

  table.setup(count, use_p, p, timeout, seed, multiplier, adjustment);

  table.set_uuid(uuid);

  // print index

  if (vm.count("index")) {
    try {
      table.print_index(keys);
      return 0;
    } catch (const std::exception& e) {
      std::cerr << "Printing index error: " << e.what() << std::endl;
      return -1;
    }
  }

  // load the table and generate the hash function

  try {
    bool status = table.load(keys, values);
    if (status == false) {
      std::cerr << "Loading table failed."<< std::endl;
      retval = -1;
      goto finish;
    }
  } catch (const std::exception& e) {
    std::cerr << "Loading table error: " << e.what() << std::endl;
    return -1;
  }
  // Test generated table

  try {
    uint64_t val = 0;

    for (int i = 0; i < keys.size(); i++) {
      val = table.find_val(keys[i]);

      if (table.notfound_val(val)) {
        std::cerr << "Error verifying key '" << keys[i] << "' at index " << i << std::endl;
        retval = -1;
        goto finish;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Testing hash function error: " << e.what() << std::endl;
    retval = -1;
    goto finish;
  }

  std::cout << "Hash function generated and verified; written to " << output_filename << std::endl;

finish:

  // serialize the hash function

  table.serialize(output_stream);

  // finish writing hash function to file

  output_file.close();

  return retval;
}

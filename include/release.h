/*
 * Copyright 2017 Rene Sugar
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

// clang-format off
/**
 * @file		release.h
 * @author	Rene Sugar <rene.sugar@gmail.com>
 * @brief		Defines the release number string
 *
 * Copyright (c) 2017 Rene Sugar  All rights reserved.
 **/
#ifndef RELEASE_H
#define RELEASE_H

#include <string>

static const int32_t PPH_VERSION_MAJOR{@PPH_VERSION_MAJOR@};
static const int32_t PPH_VERSION_MINOR{@PPH_VERSION_MINOR@};
static const int32_t PPH_VERSION_PATCH{@PPH_VERSION_PATCH@};
static const int32_t PPH_VERSION{
  @PPH_VERSION_MAJOR@ * 1000000 + @PPH_VERSION_MINOR@ * 1000 + @PPH_VERSION_PATCH@
};

static const std::string PPH_VERSION_EXTRA{"@PPH_VERSION_EXTRA@"};
static const std::string PPH_VERSION_RAW{"@PPH_VERSION_RAW@"};
static const std::string PPH_BUILD_DATE{"@PPH_BUILD_DATE@"};
static const std::string PPH_GIT_HASH{"@PPH_GIT_HASH@"};

static const std::string PPH_RELEASE{"@PPH_VERSION_RAW@-@PPH_BUILD_DATE@-@PPH_GIT_HASH@"};

#endif  // RELEASE_H

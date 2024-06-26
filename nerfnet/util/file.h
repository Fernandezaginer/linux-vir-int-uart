/*
 * Copyright 2021 Andrew Rossignol andrew.rossignol@gmail.com
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

#ifndef NERFNET_UTIL_FILE_H_
#define NERFNET_UTIL_FILE_H_

#include <google/protobuf/message.h>
#include <string>

namespace nerfnet {

// Read the contents of the supplied file into a string.
// Returns true if successful, false otherwise.
bool ReadFileToString(const std::string& filename, std::string* contents);

// Reads the contents of the supplied file into a protobuf message.
// Returns true if successful, false otherwise.
bool ReadTextProtoFileToMessage(const std::string& filename,
    google::protobuf::Message* message);

}  // namespace nerfnet

#endif  // NERFNET_UTIL_FILE_H_

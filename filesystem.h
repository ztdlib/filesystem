//    BSD 3-Clause License
//
//    Copyright (c) 2020, ztd team
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this
//       list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//
//    3. Neither the name of the copyright holder nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#if __APPLE__ || __unix__
    #include "stdio.h"
#elif _WIN32
    #include <Windows.h>
    #include <stdint.h>
    // Include for Windows
#endif

namespace ztd {
namespace fs {

#if __APPLE__ || __unix // Compile unix-like version
class file {
public:
    void open() {} // Open file
    void close() {} // Close file
    size_t getSize(){ return 0; } // Get file size
    void rename() {} // Rename file
    void insert() {} // Insert data at specified location
    void cut() {} // Remove data at specified location
    void remove() {} // Delete the file
    void write() {} // Write to the file (pass a pointer)
    template <class data> void write() {} // Write to the file (use the template)
    void append() {} // Append data at the end of the file (pass a pointer)
    template <class data> void append() {} // Append data at the end of the file (use the template)
    void* read() { return 0; } // Read data (manually specify size)
    template <class data> data* read() { return 0; } // Read data (get size from template)
};


#elif _WIN32 || defined(WIN32)  // Compile Windows version
class file {
public:
    void open() {} // Open file
    void close() {} // Close file
    size_t getSize(){ return 0; } // Get file size
    void rename() {} // Rename file
    void insert() {} // Insert data at specified location
    void cut() {} // Remove data at specified location
    void remove() {} // Delete the file
    void write() {} // Write to the file (pass a pointer)
    template <class data> void write() {} // Write to the file (use the template)
    void append() {} // Append data at the end of the file (pass a pointer)
    template <class data> void append() {} // Append data at the end of the file (use the template)
    void* read() { return 0; } // Read data (manually specify size)
    template <class data> data* read() { return 0; } // Read data (get size from template)
};


#endif

}
}

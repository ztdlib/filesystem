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


#define ztd_fsREAD  0x01     //00000001 Read Mode
#define ztd_fsWRITE 0x02     //00000010 Write Mode
#define ztd_fsAPPEND 0x04
#define ztd_fserrEXISTS 0x1F
#define ztd_fserrOPTION 0x2F
#define ztd_fserrOPEN 0x3F
#define ztd_fserrNOTEXISTS 0x4F
#define ztd_fscodesetMAXRAMCACHESIZE ((1024 * 1024) * 250)

#if __APPLE__ || __unix__
    #include "stdio.h"
#elif _WIN32
    #include <stdio.h>
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
class File {
private:

    FILE* m_filePointer = 0;
    
    //Each byte enables
    uint8_t m_openMode = 0;
        struct FileProperties {
        public:
            bool exists : 1;     // The file does exist
            bool readable : 1;   // Read access to the file is granted
            bool writable : 1;   // Write access to the file is granted
            bool executable : 1; // The file can be executed
        } fileProperties;
       
        bool m_isFileOpen : 1;

public:
   const char* m_filePath = 0;
public:

    File(const char* filePath, uint8_t openMode = 0) {

        
        *((uint8_t*)(&this->fileProperties)) = 0;
        m_isFileOpen = false;

        this->m_filePath = filePath;

        if (openMode) {
            this->open(openMode);
        }
     
    }

    ~File() {
        if (this->m_filePointer) {
            this->close();
        }
    }

    static void create(const char* filePath) {
        DWORD g_fileAtt = GetFileAttributes(filePath);
        if (!(g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY))) {
            CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else {
            throw ztd_fserrEXISTS;
        }
    }

    void create() {  
        DWORD g_fileAtt = GetFileAttributes(m_filePath);
        if (!(g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY))) {
            CreateFile(m_filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        } else {
            throw ztd_fserrEXISTS;
        }
    }

    ztd::fs::File::FileProperties getProperties() {
        DWORD g_fileAtt = GetFileAttributes(m_filePath);
        //Checks if file exists
        if (g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY)) {
            this->fileProperties.exists = true;
        }
        //Checks if file is readonly
        if (g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY) && (g_fileAtt & FILE_ATTRIBUTE_READONLY)) {
            this->fileProperties.readable = true;
            this->fileProperties.writable = false;
        }
        //Checks if file is writable and ofcourse readable
        if (g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY) && !(g_fileAtt & FILE_ATTRIBUTE_READONLY)) {
            this->fileProperties.readable = true;
            this->fileProperties.writable = true;
        }
        //Files on windows are always executables but have different Group Policies; this needs to be implemented
        this->fileProperties.executable = true;
        return fileProperties;
    }


    static ztd::fs::File::FileProperties getProperties(const char* tempFilePath) {
        DWORD g_fileAtt = GetFileAttributes(tempFilePath);
        FileProperties tempProperties;
        //Checks if file exists
        if (g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY)) {
            tempProperties.exists = true;
        }
        //Checks if file is readonly
        if (g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY) && (g_fileAtt & FILE_ATTRIBUTE_READONLY)) {
            tempProperties.readable = true;
            tempProperties.writable = false;
        }
        //Checks if file is writable and ofcourse readable
        if (g_fileAtt != INVALID_FILE_ATTRIBUTES && !(g_fileAtt & FILE_ATTRIBUTE_DIRECTORY) && !(g_fileAtt & FILE_ATTRIBUTE_READONLY)) {
            tempProperties.readable = true;
            tempProperties.writable = true;
        }
        //Files on windows are always executables but have different Group Policies; this needs to be implemented
        tempProperties.executable = true;
        return tempProperties;
    }

    void open(uint8_t settings) {   // Open file
        if (!(this->m_isFileOpen) && getProperties().exists) {
            if (settings > 0x04) {
                throw ztd_fserrOPTION;
            }
            if (settings == 0x04) {
                if (getProperties().exists) {
                    this->m_filePointer = fopen(m_filePath, "ab");
                }
            }
            if (settings == 0x03) {
                if (getProperties().exists && getProperties().readable && getProperties().writable) {
                    this->m_filePointer = fopen(m_filePath, "rb+");
                }
            }
            if (settings == 0x02) {
                if (getProperties().exists && getProperties().writable) {
                    this->m_filePointer = fopen(m_filePath, "rb+");
                }
            }
            if (settings == 0x01) {
                if (getProperties().exists && getProperties().readable) {
                    this->m_filePointer = fopen(m_filePath, "rb");
                }
            }
        }
        else {
            throw ztd_fserrOPEN;
        }
    } 
    void close(){ // Close file
        if (this->m_filePointer) {
            fclose(m_filePointer);
        }
        else {
            throw ztd_fserrOPTION;
        }
    } 

    static size_t getSize(const char* getSizeFile) {
        if (getProperties(getSizeFile).exists) {
            FILE* tempFile_ptr = fopen(getSizeFile, "r");
            fseek(tempFile_ptr, 0, SEEK_END);
            size_t fSize = ftell(tempFile_ptr);
            return fSize;
        }
        else {
            throw ztd_fserrNOTEXISTS;
        }
    }

    size_t getSize(){  // Get file size
       if (!(this->m_filePointer)) {
           open(ztd_fsREAD);
       }
       else {
           ztd_fserrOPTION;
       }
       fseek(this->m_filePointer, 0, SEEK_END);
       size_t fSize = ftell(this->m_filePointer);
       return fSize;
    }



    
    static void renameFile(const char* oldFileName, const char* newFileName) { //Rename file static method
        if (rename(oldFileName, newFileName)) {}
        else { throw ztd_fserrNOTEXISTS; }
    }
    void renameFile(const char* newFileName) { //Rename file method
        rename(this->m_filePath, newFileName);
    }
    
    static void deleteFile(const char* deleteFileName) { //Delete file statich method
        if (remove(deleteFileName)) {}
        else { throw ztd_fserrNOTEXISTS; }
    }
    
    void deleteFile(){ // Delete file method
        if (!(this->m_filePointer)) {
            remove(this->m_filePath);
        }
        else {
            close();
            remove(this->m_filePath);
        }
            
    }

    void resize(size_t newSize) {
        LARGE_INTEGER size;
        size.QuadPart = 0;
        HANDLE fh = CreateFile(m_filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh == INVALID_HANDLE_VALUE) {
            throw ztd_fserrNOTEXISTS;
            return;
        }
        GetFileSizeEx(fh, &size);
        size.QuadPart = newSize;
        SetFilePointerEx(fh, size, NULL, FILE_BEGIN);
        if (SetEndOfFile(fh) == 0) {
            throw ztd_fserrOPTION;
            return;
        }
        CloseHandle(fh);
    }

    static void resize(const char* filePath, size_t newSize) {
        LARGE_INTEGER size;
        size.QuadPart = 0;
        HANDLE fh = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh == INVALID_HANDLE_VALUE) {
            throw ztd_fserrNOTEXISTS;
            return;
        }
        GetFileSizeEx(fh, &size);
        size.QuadPart = newSize;
        SetFilePointerEx(fh, size, NULL, FILE_BEGIN);
        if (SetEndOfFile(fh) == 0) {
            throw ztd_fserrOPTION;
            return;
        }
        CloseHandle(fh);
    }

  

    void write(void* sourceStream, size_t streamSize, size_t fileOffset = 0) {  // Write to the file (pass a pointer)
        if (this->m_filePointer) {
            fseek(this->m_filePointer, fileOffset, SEEK_SET);
            fwrite(sourceStream, 1, streamSize, this->m_filePointer);
        }
        else {
            throw ztd_fserrNOTEXISTS;
        }
    } 
    template <class data> void write(data* sourceData, size_t fileOffset = 0) {   // Write to the file (use the template)
        this->write(sourceData, sizeof(data), fileOffset);
    } 

    void* read(size_t streamSize, size_t fileOffset = 0) { // Read data (manually specify size)
        if (this->m_filePointer) {
            void* buffer = malloc(streamSize);
            fseek(this->m_filePointer, fileOffset, SEEK_SET);
            fread(buffer, 1, streamSize, this->m_filePointer);
            return buffer;
        }
        else {
            throw ztd_fserrNOTEXISTS;
        }
    } 

    template <class data> data* read(size_t fileOffset = 0) { // Read data (get size from template)
        data* cache = malloc(sizeof(data));
        cache = this->read(sizeof(data), fileOffset);
        return cache;
      
    } 

    void insert(void* sourceStream, size_t streamSize, size_t fileOffset = 0, const char* diskCachePath = 0, size_t maxRamCacheSize = 0) {    // Insert data at specified location
        if (!diskCachePath) {
            void* buffer = malloc(this->getSize() - fileOffset); // Allocate memory for other data
            buffer = this->read(this->getSize() - fileOffset, fileOffset);  // Read from the entry point to the end of the file and save it to the buffer
            this->write(sourceStream, streamSize, fileOffset); // Write the new data
            this->write(buffer, this->getSize() - fileOffset, this->getSize() - fileOffset + streamSize - 1); //Extends the file using the rest of the data (the buffer)
        }
        else {
               //Disk cache ability not working for now sorry ;w;                                                                                                                                                                    
        }
    
    }

    template <class data> void insert(data* sourceStream, size_t streamSize, size_t fileOffset = 0, const char* diskCachePath = 0, size_t maxRamCacheSize = 0){
        this->insert(sourceStream, sizeof(data), fileOffset, diskCachePath, maxRamCacheSize);
    }

    void cut(size_t size, size_t fileOffset = 0, const char* diskCachePath = 0, size_t maxRamCacheSize = 0){ // Remove data at specified location
        if (!diskCachePath) {
            size_t cachedSize = this->getSize();
            void* buffer = this->read(cachedSize - fileOffset - size, fileOffset + size);
            this->resize(m_filePath, fileOffset);
            this->write(buffer, cachedSize - fileOffset - size, fileOffset);
        }
        else {
            //TODO: Add support for diskCaching
        }
    }

};

#endif

}
}

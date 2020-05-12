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


// Settings Macro Definition

#define ztd_fsREAD 0x01 // Open and read (file must exist)
#define ztd_fsWRITE 0x02 // Create new file for writing (if existing, erase old content)
#define ztd_fsAPPEND 0x04 // Append data at the end of the existing file (cannot be used with other options, in such case this is ignored)
#define ztd_fsRAMCACHE 0x05 // Use RAM as cache for insert() method
#define ztd_fsDISKCACHE 0x06 // Use Disk as cache for the insert() method

// Errors Macro Definition

#define ztd_fserrEXISTS 0x1F // Error, could not create the file (it already exists)
#define ztd_fserrOPTION 0x2F // Error, options are invalid
#define ztd_fserrOPEN 0x3F // Error, the file is already open
#define ztd_fserrNOTEXISTS 0x4F // Error, the file does not exists
#define ztd_fserrGENERIC 0x5F // Generic error, check the documentation for more info
#define ztd_fserrCLOSED 0x6F // Error, the file is already closed

#if __APPLE__ || __unix__
    #include <stdio.h>
    #include <stdint.h>
    #include <unistd.h>
    #include <stdlib.h>
#elif __WIN32
    #include <stdio.h>
    #include <stdint.h>
    #include <Windows.h>
#endif

namespace ztd {
namespace fs {

#if __APPLE__ || __unix // Compile unix-like version
class File {
private: // Private Members
    
    FILE* m_filePointer = 0; // Pointer to the file

    
    /*
        Utility bool (is the file open?)
     */
    bool m_isFileOpen: 1;
    
public: // Public Members
    const char* m_filePath = 0; // Contains full path to the file
    
    /*
        fileProperties gets populated with info from the getProperties method
        contains info about the file
     */
    struct FileProperties {
    public:
        bool exists: 1; // The file does exist
        bool readable: 1; // Read access to the file is granted
        bool writable: 1; // Write access to the file is granted
        bool executable: 1; // The file can be executed
    } fileProperties;
    
public: // Methods
    
    /*
        Contructor, set to 0 all the members (in case the compiler does not by default)
        Opens the file if requested with the correct settings, (a call to the file::open member)
     */
    // Status: Working
    File(const char* filePath, uint8_t openMode = 0) {

        // Set properties in the class to 0 (some compilers may not do this by default or if using an optimization flag)
        *((uint8_t*)(&this->fileProperties)) = 0;
        // m_filePath does not need to be set to 0 since it gets written over in the next instruction always.
        this->m_isFileOpen = false; // Set it to false, if it gets open, the open function will take care of it
        
        // Save the path as a public member for easy access in the future by the user
        this->m_filePath = filePath;
        
        // Executing operations on constructor if required.
        if (openMode) { // Open file using the appropriate settings
            this->open(openMode);
        }

    }
    
    ~File() {
        if (!this->m_isFileOpen()) {
            this->close();
        }
    }
    
    /*
        Creates an empty file, if the file already exist throw an erro (ztd_fserrEXISTS 0x1F)
     */
    // Status: Working
    void create() { // Create file
        if (access(this->m_filePath, F_OK)) { // File does not exist, proceed to creation.
            this->m_filePointer = fopen(m_filePath, "w");
            fclose(this->m_filePointer);
        } else {
            throw ztd_fserrEXISTS; // File already exist, throw error. (0x1F)
        }
    }
    static void create(const char* filePath) { // Create a blank file, does not require allocating object
        if (access(filePath, F_OK)) { // File does not exist, proceed to creation.
            FILE* filePointer = fopen(filePath, "w");
            fclose(filePointer);
        } else {
            throw ztd_fserrEXISTS; // File already exist, throw error. (0x1F)
        }
    }
    
    /*
        Check for all the various access types, then update the output with the availables accesses.
     */
    // Status: Working
    ztd::fs::File::FileProperties getProperties() { // Check file access
        if (!access(this->m_filePath, F_OK)) { // Check if the file exists
            this->fileProperties.exists = true;
        }
        if (!access(this->m_filePath, R_OK)) { // Check if the file is readable
            this->fileProperties.readable = true;
        }
        if (!access(this->m_filePath, W_OK)) { // Check if the file is writable
            this->fileProperties.writable = true;
        }
        if (!access(this->m_filePath, X_OK)) { // Check if the file is executable
            this->fileProperties.executable = true;
        }
        return this->fileProperties;
    }
    static ztd::fs::File::FileProperties getProperties(const char* filePath) {
        FileProperties cacheFileProperties;
        if (!access(filePath, F_OK)) { // Check if the file exists
            cacheFileProperties.exists = true;
        }
        if (!access(filePath, R_OK)) { // Check if the file is readable
            cacheFileProperties.readable = true;
        }
        if (!access(filePath, W_OK)) { // Check if the file is writable
            cacheFileProperties.writable = true;
        }
        if (!access(filePath, X_OK)) { // Check if the file is executable
            cacheFileProperties.executable = true;
        }
        return cacheFileProperties;
    }
    
    /*
        Opens the file given the appropriate settings, (checks if the file is already open)
     */
    // Status: Working
    void open(uint8_t settings) { // Open existing file
        
        if (!this->m_isFileOpen && this->getProperties().exists) {
            if (settings > 0x04) { // All option form 0x05 above are invalid, throw the appropriate error, all the functions below 0x05 (0x05 excluded) are valid
                throw ztd_fserrOPTION;
            } else if (settings == 0x04) { // Open in append mode
                if (this->getProperties().exists) { // If the file exists, open it in append mode
                    this->m_filePointer = fopen(this->m_filePath, "ab");
                }
            } else if (settings == 0x03) { // Open in read & write mode
                this->m_filePointer = fopen(this->m_filePath, "rb+");
            } else if (settings == 0x01) { // Open in read mode
                this->m_filePointer = fopen(this->m_filePath, "rb");
            } else if (settings == 0x02) { // Open in read & write mode
                this->m_filePointer = fopen(this->m_filePath, "rb+");
            }
            this->m_isFileOpen = 1;
        } else {
            throw ztd_fserrGENERIC; // Either the file is already open or either it does not exist.
        }
    }
    
    /*
        Closes the file
     */
    // Status: Working
    void close() { // Close file
        if (this->m_isFileOpen) { // If the file is open, close it.
            fclose(this->m_filePointer);
            this->m_isFileOpen = 0;
        } else { // If the file is not open, throw an error.
            throw ztd_fserrCLOSED;
        }
    }
    
    /*
        Get the file size.
    */
    size_t getSize() { // Get file size
        size_t size = 0;
        if (this->fileProperties.exists) {
            if (!this->m_isFileOpen) { // If file is closed, open it, get the size and close it.
                this->open(ztd_fsREAD);
                fseek(this->m_filePointer, 0, SEEK_END);
                size = ftell(this->m_filePointer);
                this->close();
            } else { // If the file is opened, get the size and return it.
                fseek(this->m_filePointer, 0, SEEK_END);
                size = ftell(this->m_filePointer);
            }
        } else { // Error, the file does not exists, cannot get file size
            throw ztd_fserrNOTEXISTS;
        }
        return size;
    }
    static size_t getSize(const char* filePath) {
        size_t size = 0;
        if (ztd::fs::File::getProperties(filePath).exists) { // Get file size, does not check if the file is open (if it is not allocated, it should not be open...)
            FILE* filePointer = fopen(filePath, "r");
            fseek(filePointer, 0, SEEK_END);
            size = ftell(filePointer);
            fclose(filePointer);
        } else { // Error, the file does not exists, cannot get file size
            throw ztd_fserrNOTEXISTS;
        }
        return size;
    }
    
    /*
        Change the file name
     */
    static void renameFile(const char* oldFileName, const char* newFileName) { // Static version, does not require name allocation, but does require the full path
        rename(oldFileName, newFileName);
    }
    void renameFile(const char* newFileName) { // Uses the member path, but does require the object to be allocated
        rename(this->m_filePath, newFileName);
    }
    
    /*
        Delete a file.
     */
    static void deleteFile(const char* filePath) { // Does not check if the file is open or not
        remove(filePath);
    }
    void deleteFile() { // Checks if the file is open.
        if (!this->m_isFileOpen) {
            remove(this->m_filePath);
        } else {
            this->close();
            remove(this->m_filePath);
        }
    }
    
    /*
        Shrink is used to truncate a file to a specific size
     */
    static void resize(const char* filePath, size_t newSize) {
        truncate(filePath, newSize);
    }
    void resize(size_t newSize) {
        truncate(this->m_filePath, newSize); // Now sure what ftruncate requires, just going to use truncate().
    }
    
    /*
        Write data to a file at specified locaiton (by default the first byte), will overwrite data.
     */
    void write(void* sourceStream, size_t streamSize, size_t fileOffset = 0) { // Write to the file (pass a pointer)
        if (this->m_isFileOpen) {
            fseek(this->m_filePointer, fileOffset, SEEK_SET);
            fwrite(sourceStream, 1, streamSize, this->m_filePointer);
        } else {
            throw ztd_fserrCLOSED;
        }
    }
    template <class data> void write(data* sourceData, size_t fileOffset = 0) { // Write to the file (use the template)
        this->write(sourceData, sizeof(data), fileOffset);
    }
    
    
    void* read(size_t streamSize, size_t fileOffset = 0) { // Read data (manually specify size)
        if (this->m_isFileOpen) {
            void* buffer = malloc(streamSize);
            fseek(this->m_filePointer, fileOffset, SEEK_SET);
            fread(buffer, 1, streamSize, this->m_filePointer);
            return buffer;
        } else {
            throw ztd_fserrCLOSED;
        }
    }
    template <class data> data* read(size_t fileOffset = 0) { // Wrapper using a template for user convenience
        data* cache = malloc(sizeof(data));
        cache = this->read(sizeof(data), fileOffset);
        return cache;
    }
    
    /*
        Insert data at a specific location in the file, the rest of the data gets moved forward (extending the file size)
        The method provides 2 modes, a RAM cache and a file cache mode (this way if there is a lot of data to move, the ram won't get filled up)
    */
    void insert(void* sourceStream, size_t streamSize, size_t fileOffset = 0, const char* diskCachePath = 0, size_t maxRamCacheSize = 0) { // Insert data at specified location
        size_t sizeCache = this->getSize() - fileOffset; // Using this variable as cache, wastes something like 2 bytes but saves a lot of function calls and size checking. Better this way.
        if (!diskCachePath) {
            void* buffer = this->read(sizeCache, fileOffset); // Read from the entry point to the file end
            this->write(sourceStream, streamSize, fileOffset); // Write the new data
            this->write(buffer, sizeCache, sizeCache + streamSize - 1); // Extend the file using the rest of the data
        } else {
            //  TODO: This code currently does not work, need to fix it. (probably something with the fseek, but need to write a better version anyway)
            //  Just a function to log this to the user
            printf("[ZTD/FILESYSTEM WARNING:]   This option is currently not available, please check for ztd/filesystem updates.");
//            FILE* cacheFile = fopen(diskCachePath, "ab");
//            void* buffer = 0; // Buffer for each cycle iteration, no need to re allocate the RAM, read() does that on its own.
//            for (size_t i = 0; i < (sizeCache / maxRamCacheSize) - 1; ++i) {
//                buffer = this->read(maxRamCacheSize, fileOffset + (i * maxRamCacheSize));
//                fwrite(buffer, 1, maxRamCacheSize, cacheFile); // File opened in append mode, no need to modify fseek.
//            }
//            buffer = this->read(sizeCache - (maxRamCacheSize * (sizeCache / maxRamCacheSize)), this->getSize() - sizeCache - (maxRamCacheSize * (sizeCache / maxRamCacheSize)));
//            fwrite(buffer, 1, sizeCache - (maxRamCacheSize * (sizeCache / maxRamCacheSize)), cacheFile);
//            // Moving data to the cacheFile is done
//            // Proceed to adding the user given data at the correct spot onto the original file.
//            this->write(sourceStream, streamSize, fileOffset);
//            // Now add the cached data (on the file) back to this file at the correct location.
//            for (size_t i = 0; i < (sizeCache / maxRamCacheSize); ++i) {
//                fseek(cacheFile, (i * maxRamCacheSize), SEEK_SET); // Set the file pointer to the current file block
//                fread(buffer, 1, maxRamCacheSize, cacheFile); // Read one block
//                this->write(buffer, maxRamCacheSize, sizeCache + streamSize + (i * maxRamCacheSize) - 1); // Write the cached data
//            } // Now only the last block is missing, make sure to copy the correct amount of data and paste it.
//            fread(buffer, 1, maxRamCacheSize, cacheFile);
//            this->write(buffer, sizeCache - (maxRamCacheSize * (sizeCache / maxRamCacheSize)), sizeCache + fileOffset + ((sizeCache / maxRamCacheSize) * maxRamCacheSize));
//            // All done, just delete the cache file
//            fclose(cacheFile);
//            //ztd::fs::File::deleteFile(diskCachePath);
        }
    }
    template <class data> void insert(data* sourceData, size_t fileOffset = 0, const char* diskCachePath = 0, size_t maxRamCacheSize = 0) {
        this->insert(sourceData, sizeof(data), fileOffset, diskCachePath, maxRamCacheSize); // Just call the non-template function
    }
    
    void cut(size_t size, size_t fileOffset = 0, const char* diskCachePath = 0, size_t maxRamCacheSize = 0) { // Remove data at specified location
        if (!diskCachePath) {
            size_t cachedSize = this->getSize(); // Necessary since the file size is going to change and this value needs to not change
            void* buffer = this->read(cachedSize - fileOffset - size, fileOffset + size);
            this->resize(fileOffset);
            this->write(buffer, cachedSize - fileOffset - size, fileOffset);
        } else {
            // TODO: Add support for diskCaching for the cut method.
            // Just a function to log this to the user
            printf("[ZTD/FILESYSTEM WARNING:]   This option is currently not available, please check for ztd/filesystem updates.");
        }
    }
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

// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================



// ******************************************************************************************************************************************
//
//          this class provides methods for saving and loading data to and from the SPIFFS (SPI Flash File System)
//          
// ******************************************************************************************************************************************


#ifndef ffs_H
#define ffs_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class FlashFS {
  private:
    String _path;
    bool _isDirectory();                                    // returns true, if path is a directory
    bool _mkdir(String path);                               // make directory. path shall end with "/"
    File* _file = nullptr;                                  // holds pointer to file, wehen opened
  
  public:
    FlashFS(String _path);                                  // Constructor. path is either file or folder
    bool begin();                                           // initiates the file system. Returns false if initiation failed
    void write_f(String text);                              // write String to file.
    void write_f(uint8_t* data, size_t len);                // write len bytes of binary data[] to file
    String read_f();                                        // read String from file.
    bool read_f(uint8_t (*bitmap)[14], int rows);           //

/*    void read_f(uint8_t* data, size_t len);                 // read len bytes from file and save in data[] */
    File* open_f(String mode);                              // open file
    bool close_f();                                         // close file
    String listFilesInJson();                               // list all files in the root directory in JSON format
    bool delete_f();                                        // delete a file
    bool rename_f(String newPath);                          // rename file or directory
    bool copy_f(String destPath);                           // copy file or folder (including content)
    bool move_f(String destPath);                           // move file or folder (including content)
};

#endif

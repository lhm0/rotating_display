// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================


#include "FlashFS.h"
#include <Arduino.h>
#include <LittleFS.h>


// Constructor

FlashFS::FlashFS(String path) {
  this->_path = path;
  if (_isDirectory()) {
    _mkdir(_path);              // creat path, if it does not exist yet
  }
}

// Initialisierungsroutine

bool FlashFS::begin() {
  bool initok = false;
  initok = LittleFS.begin();
  if (!(initok)) // Format LittleFS, of not formatted. - Try 1
  {
    Serial.println("LittleFS file system formated.");
    LittleFS.format();
    initok = LittleFS.begin();
  }
  if (!(initok)) // Format LittleFS. - Try 2
  {
    LittleFS.format();
    initok = LittleFS.begin();
  }
  if (initok) { Serial.println("LittleFS ist  OK"); } else { Serial.println("LittleFS ist nicht OK"); }
  return initok;
}


// Routine zum Lesen einer Datei
String FlashFS::read_f() {
  File file = LittleFS.open(_path.c_str(), "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }
  String text = file.readString();
  file.close();
  return text;
}

bool FlashFS::read_f(uint8_t (*bitmap)[14], int rows) {
  File file = LittleFS.open(_path.c_str(), "r");
  bool success = true;
  if (!file) {
    Serial.println("Failed to open file for reading");
    success = false;
    return success;
  } else {
    Serial.println("file open successful");
  }
  
  // Read the data into the bitmap array
  for (int i = 0; i < rows; ++i) {
    int bytesRead = file.read(bitmap[i], sizeof(bitmap[i]));
    
    // If there are less bytes than expected, fill with 0
    if (bytesRead < sizeof(bitmap[i])) {
      memset(&bitmap[i][bytesRead], 0, sizeof(bitmap[i]) - bytesRead);
    }
  }
  
  file.close();
  return success;
}

// Routine zum Schreiben einer Datei
void FlashFS::write_f(String text) {
  File file = LittleFS.open(_path.c_str(), "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(text);
  file.close();
}

void FlashFS::write_f(uint8_t* data, size_t len) {
  File file = LittleFS.open(_path.c_str(), "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.write(data, len);
  file.close();
}

File* FlashFS::open_f(String mode) {
  if (_isDirectory()) {
    return nullptr; // return null pointer to indicate error
  }
  _file = new File(LittleFS.open(_path.c_str(), mode.c_str()));
  return _file;
}

bool FlashFS::close_f() {
  if (_file) {
    _file->close();
    delete _file;
    _file = nullptr;
    return true; // successful close
  }
  return false; // file not open
}

String FlashFS::listFilesInJson() {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  if (!LittleFS.begin()) {
    Serial.println("An error occurred while mounting LittleFS");
    return "";
  }

  Dir dir = LittleFS.openDir(_path.c_str());
  while (dir.next()) {
    String fileName = dir.fileName();
    array.add(fileName);
  }

  String jsonString;
  serializeJson(doc, jsonString);

  return jsonString;
}

bool FlashFS::rename_f(String newPath) {
  bool success; 

  if (_isDirectory()) {
    String newFolder = newPath.substring(0, newPath.length() - 1);
    String oldFolder = _path.substring(0, _path.length() - 1);
    success = LittleFS.rename(oldFolder.c_str(), newFolder.c_str());
  } else {
    success = LittleFS.rename(_path.c_str(), newPath.c_str());
  }
  
  if (success) {
    _path = newPath;
  }
  return success;
}

bool FlashFS::delete_f() {
  if (_isDirectory()) {
    // remove all files and subdirectories recursively, before deleting the directory
    Dir dir = LittleFS.openDir(_path.c_str());
    while (dir.next()) {
      String fileName = dir.fileName();
      String filePath = _path + fileName;
      if (dir.isDirectory()) {
        // delete subdirectory
        filePath += "/";
        FlashFS* subDir = new FlashFS(filePath);
        subDir -> delete_f();
        delete subDir;
      } else {
        if (!LittleFS.remove(filePath.c_str())) {
          return false;
        }
      }
    }
    // now directory is empty. remove the directory
    return LittleFS.rmdir(_path.c_str());
  } else {
    // delete a single file
    return LittleFS.remove(_path.c_str());
  }
}

bool FlashFS::copy_f(String destPath) {
  String sourcePath = _path;
  if (_isDirectory()) {
    // create destination directory if it doesn't exist
    _mkdir(destPath);
    // copy all files and subdirectories recursively
    Dir dir = LittleFS.openDir(sourcePath.c_str());
    while (dir.next()) {
      String sourceFileName = dir.fileName();
      String sourceFilePath = sourcePath + sourceFileName;
      if (dir.isDirectory()) {
        sourceFilePath += "/";
        String subDirDestPath = destPath + sourceFileName + "/";
        FlashFS* subDir = new FlashFS(sourceFilePath);
        subDir -> copy_f(subDirDestPath);
        delete subDir;
      } else {
        String destFilePath = destPath + sourceFileName;
        File* srcFile = new File(LittleFS.open(sourceFilePath.c_str(), "r"));
        File* destFile = new File(LittleFS.open(destFilePath.c_str(), "w"));
        if (!srcFile || !destFile) {
          return false; // copy failed
        }
        while (srcFile->available()) {
          destFile->write(srcFile->read());
        }
        srcFile->close();
        destFile->close();
        delete srcFile;
        delete destFile;
      }
    }
    return true;
  } else {
    // determine destination path:
    // extract file name from sourcePath
    String fileName = "";
    int lastSlashIndex = sourcePath.lastIndexOf("/");
    if (lastSlashIndex >= 0) {
      fileName = sourcePath.substring(lastSlashIndex + 1);
    }
    destPath += fileName;

    // copy file to destination
    File* srcFile = new File(LittleFS.open(sourcePath.c_str(), "r"));
    File* destFile = new File(LittleFS.open(destPath.c_str(), "w"));
    if (!srcFile || !destFile) {
      return false; // copy failed
    }
    while (srcFile->available()) {
      destFile->write(srcFile->read());
    }
    srcFile->close();
    destFile->close();
    delete srcFile;
    delete destFile;
    return true;
  }
}

bool FlashFS::move_f(String destPath) {
  if (copy_f(destPath)) {
    delete_f();
    _path = destPath;
    return true;
  } else {
    return false; // move failed
  }
}

bool FlashFS::_isDirectory() {
    return _path.c_str()[_path.length()-1] == '/';
}

bool FlashFS::_mkdir(String path) {
  if (!LittleFS.exists(path.c_str())) {        // check if directory already exists
    if (path.endsWith("/")) {                  // check if path ends with "/"
      path.remove(path.length()-1);           // remove the last character
    }
    bool success = LittleFS.mkdir(path.c_str()); // make directory, if it does not exist, yet
      if (!success) {
        Serial.println("Failed to create directory");
      }
    return success;
  } else {
    return true;
  }
}
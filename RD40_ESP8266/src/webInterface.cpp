// =========================================================================================================
// Rotating Display RD40
// © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// You may use, adapt, share. If you share, "share alike".
// You may not use this software for commercial purposes.
// =========================================================================================================

#ifndef ASYNCWEBSERVER_REGEX
#define ASYNCWEBSERVER_REGEX
#endif

#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>

#include "my_ESP.h"
#include "my_BMP.h"
#include "webInterface.h"
#include "webinterface_data.h"
#include "FlashFS.h"
#include "RD_40.h"

// Static variable initialization
webInterface* webInterface::_instance = nullptr;

// =========================================================================================================
// Constructor
// =========================================================================================================

webInterface::webInterface() {}

// =========================================================================================================
// begin Method
// =========================================================================================================

void webInterface::begin(String ssid_) {
    _ssid = ssid_;

    _apiKey_f.begin();  // Initialize LittleFS (only once)

    apiKey = _apiKey_f.read_f();
    location = _location_f.read_f();
    country = _country_f.read_f();
    clockFacePath = _clockFacePath_f.read_f();
    logoPath = _logoPath_f.read_f();
    imagePath = _imagePath_f.read_f();

    _instance = this;
    _startServer();
}

// =========================================================================================================
// _startServer() and WebSocket Handlers
// =========================================================================================================

void webInterface::_startServer() {
    // Index Page
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("Index called");
        request->send(LittleFS, "/html/index.html", "text/html");
    });

    _server.on("/index.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/css/index.css", "text/css");
    });

    _server.on("/index.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/scripts/index.js", "text/javascript");
    });

    _server.on("/favicon.ico", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
    });

    _server.on("/getParam", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["mode"] = clockMode;
        doc["brightness"] = brightness;

        Serial.print("Mode uploaded: ");
        Serial.println(clockMode);

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/configDone", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/index.html", "text/html");
    });

    _server.on("/updateradiobutton", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("button")) {
            String button = request->getParam("button")->value();
            clockMode = button == "option1" ? 0 :
                        button == "option2" ? 1 :
                        button == "option3" ? 2 :
                        button == "option4" ? 3 :
                        button == "option5" ? 4 :
                        button == "option6" ? 5 : 6;
        } else {
            clockMode = 0;
        }
        request->send(200, "text/plain", "OK");
    });

    _server.on("/slider", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            _brightness_s = request->getParam("value")->value();
            brightness = _brightness_s.toInt();
            Serial.println("Slider value = " + _brightness_s);
        } else {
            Serial.println("No slider value sent");
        }
        request->send(200, "text/plain", "OK");
    });

    // Config Weather Page
    _server.on("/configWeather", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/configWeather.html", "text/html");
    });

    _server.on("/configWeather.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/css/configWeather.css", "text/css");
    });

    _server.on("/configWeather.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/scripts/configWeather.js", "text/javascript");
    });

    _server.on("/getWeatherParam", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["apiKey"] = _apiKey_f.read_f();
        doc["location"] = _location_f.read_f();
        doc["country"] = _country_f.read_f();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/getWeather", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["w_icon"] = _w_icon;
        doc["w_temp"] = _w_temp;
        doc["w_humi"] = _w_humi;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/uploadWeatherParam", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("value1") && request->hasParam("value2") && request->hasParam("value3")) {
            apiKey = request->getParam("value1")->value();
            location = request->getParam("value2")->value();
            country = request->getParam("value3")->value();

            Serial.print("apiKey: ");
            Serial.println(apiKey);
            Serial.print("location: ");
            Serial.println(location);
            Serial.print("country: ");
            Serial.println(country);

            _apiKey_f.write_f(apiKey);
            _location_f.write_f(location);
            _country_f.write_f(country);
        }
        request->send(200, "text/plain", "OK");
        updateWeather = true;
    });

    for (int i = 0; i < 9; i++) {
        String path = "/w_img" + String(i) + ".PNG";
        _server.on(path.c_str(), HTTP_GET, [i](AsyncWebServerRequest *request) {
            String webImage = "/html/images/w_img" + String(i) + ".png";
            request->send(LittleFS, webImage, "image/png");
        });
    }

    // Reset WiFi Page
    _server.on("/resetWifi", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/resetWifi.html", "text/html");
    });

    _server.on("/resetWifi.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/css/resetWifi.css", "text/css");
    });

    _server.on("/resetWifi.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/scripts/resetWifi.js", "text/javascript");
    });

    _server.on("/reset", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
        FlashFS* flashFs1 = new FlashFS("/variables/ssid.txt");
        flashFs1->delete_f();
        delete flashFs1;
        FlashFS* flashFs2 = new FlashFS("/variables/password.txt");
        flashFs2->delete_f();
        delete flashFs2;
    });

    _server.on("/getWifiParam", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["ssid"] = _ssid_f.read_f();
        doc["password"] = _password_f.read_f();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/uploadWifiParam", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("value1") && request->hasParam("value2")) {
            String _ssid = request->getParam("value1")->value();
            String _password = request->getParam("value2")->value();

            _ssid_f.write_f(_ssid);
            _password_f.write_f(_password);
        }
        request->send(200, "text/plain", "OK");
    });

    // File Manager Page
    _server.on("/fileManager", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/fileManager.html", "text/html");
    });

    _server.on("/fileManager.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/css/fileManager.css", "text/css");
    });

    _server.on("/fileManager.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/scripts/fileManager.js", "text/javascript");
    });

    _server.on("/filelist", HTTP_GET, [this](AsyncWebServerRequest *request) {
        _currentPath = request->getParam("path")->value();
        String json;
        {
            FlashFS flashFs(_currentPath);
            json = flashFs.listFilesInJson();
        }
        Serial.println("Path: " + _currentPath);
        Serial.println("JSON: " + json);
        request->send(200, "application/json", json);
    });

    _server.on("/deletefile", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String fileName = request->getParam("filename")->value();
        Serial.print("Delete file: ");
        Serial.println(fileName);

        FlashFS* flashFs = new FlashFS(fileName);
        flashFs->delete_f();
        delete flashFs;

        String json;
        {
            FlashFS flashFs("/");
            json = flashFs.listFilesInJson();
        }
        request->send(200, "application/json", json);
    });

    _server.on("/renamefile", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String oldFileName = request->getParam("oldfilename")->value();
        String newFileName = request->getParam("newfilename")->value();
        Serial.print("Rename file: ");
        Serial.print(oldFileName);
        Serial.print(" => ");
        Serial.println(newFileName);

        FlashFS* flashFs = new FlashFS(oldFileName);
        flashFs->rename_f(newFileName);
        delete flashFs;

        String json;
        {
            FlashFS flashFs("/");
            json = flashFs.listFilesInJson();
        }
        request->send(200, "application/json", json);
    });

    _server.on("/copyfile", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String sourcePath = request->getParam("source")->value();
        String destPath = request->getParam("destination")->value();
        int moveFlag = request->getParam("moveflag")->value().toInt();
        Serial.print("Copy source file: ");
        Serial.println(sourcePath);
        Serial.print("Copy destination folder: ");
        Serial.println(destPath);

        if (sourcePath.endsWith("/")) {  // If directory
            if (destPath.startsWith(sourcePath)) {
                return;
            }
            // Determine destination path
            String lastFolder = "";
            int secondLastSlashIndex = sourcePath.lastIndexOf("/", sourcePath.length() - 2);
            if (secondLastSlashIndex >= 0) {
                lastFolder = sourcePath.substring(secondLastSlashIndex + 1, sourcePath.length() - 1);
            }
            destPath += lastFolder + "/";
        }

        FlashFS* flashFs = new FlashFS(sourcePath);
        flashFs->copy_f(destPath);
        delete flashFs;

        if (moveFlag != 0) {
            FlashFS* flashFs = new FlashFS(sourcePath);
            flashFs->delete_f();
            delete flashFs;
        }

        String json;
        {
            FlashFS flashFs("/");
            json = flashFs.listFilesInJson();
        }
        request->send(200, "application/json", json);
    });

    _server.on("/mkdir", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String newDir = request->getParam("filename")->value();
        Serial.print("Create directory: ");
        Serial.println(newDir);

        FlashFS* flashFs = new FlashFS(newDir);
        delete flashFs;

        String json;
        {
            FlashFS flashFs("/");
            json = flashFs.listFilesInJson();
        }
        request->send(200, "application/json", json);
    });

    _server.on("/uploadfile", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("/uploadfile.....");
        request->send(200);
    }, _handleUpload);

    _server.on("/downloadfile", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("filename")) {
            String filename = request->getParam("filename")->value();
            Serial.print("Download file: ");
            Serial.println(filename);
            String contentType = "application/octet-stream";
            if (filename.endsWith(".txt")) {
                contentType = "text/plain";
            } else if (filename.endsWith(".js")) {
                contentType = "application/javascript";
            } else if (filename.endsWith(".css")) {
                contentType = "text/css";
            } else if (filename.endsWith(".html")) {
                contentType = "text/html";
            } else if (filename.endsWith(".gif")) {
                contentType = "image/gif";
            } else if (filename.endsWith(".jpeg") || filename.endsWith(".jpg")) {
                contentType = "image/jpeg";
            } else if (filename.endsWith(".tiff") || filename.endsWith(".tif")) {
                contentType = "image/tiff";
            } else if (filename.endsWith(".png")) {
                contentType = "image/png";
            } else if (filename.endsWith(".webp")) {
                contentType = "image/webp";
            } else if (filename.endsWith(".bmp")) {
                contentType = "image/bmp";
            }
            request->send(LittleFS, filename, contentType);
        }
    });

    _server.on("/fileSize", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("filename")) {
            String filename = request->getParam("filename")->value();
            Serial.print("File size request: ");
            Serial.println(filename);
            if (LittleFS.exists(filename)) {
                File file = LittleFS.open(filename, "r");
                String fileSize = String(file.size());
                Serial.print("File size: ");
                Serial.println(fileSize);
                request->send(200, "application/json", "{\"size\":" + fileSize + "}");
                file.close();
            } else {
                request->send(404);
            }
        }
    });

    _server.on("/diskspace", HTTP_GET, [this](AsyncWebServerRequest *request) {
        FSInfo fs_info;
        LittleFS.info(fs_info);
        String totalBytes = String(fs_info.totalBytes);
        String usedBytes = String(fs_info.usedBytes);
        String response = "{\"totalBytes\": " + totalBytes + ", \"usedBytes\": " + usedBytes + "}";
        request->send(200, "application/json", response);
    });

    // RD40 Image Manager Page
    _server.on("/rd40ImageManager", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/rd40ImageManager.html", "text/html");
    });

    _server.on("/rd40ImageManager.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/css/rd40ImageManager.css", "text/css");
    });

    _server.on("/rd40ImageManager.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/scripts/rd40ImageManager.js", "text/javascript");
    });

    _server.on("/selectclockface", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("filename")) {
            clockFacePath = request->getParam("filename")->value();
            _clockFacePath_f.write_f(clockFacePath);

            Serial.print("Selected clock face: ");
            Serial.println(clockFacePath);
            request->send(200, "text/plain", "File selection saved successfully.");
        } else {
            request->send(400, "text/plain", "Invalid request.");
        }
    });

    _server.on("/selectlogo", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("filename")) {
            logoPath = request->getParam("filename")->value();
            _logoPath_f.write_f(logoPath);

            Serial.print("Selected logo: ");
            Serial.println(logoPath);
            request->send(200, "text/plain", "File selection saved successfully.");
        } else {
            request->send(400, "text/plain", "Invalid request.");
        }
    });

    _server.on("/selectimage", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("filename")) {
            imagePath = request->getParam("filename")->value();
            _imagePath_f.write_f(imagePath);

            Serial.print("Selected image: ");
            Serial.println(imagePath);
            request->send(200, "text/plain", "File selection saved successfully.");
        } else {
            request->send(400, "text/plain", "Invalid request.");
        }
    });

    _server.on("/getRD40Names", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["clockface"] = _clockFacePath_f.read_f();
        doc["logo"] = _logoPath_f.read_f();
        doc["image"] = _imagePath_f.read_f();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Time Zone Page
    _server.on("/timeZone", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/timeZone.html", "text/html");
    });

    _server.on("/timeZone.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/css/timeZone.css", "text/css");
    });

    _server.on("/timeZone.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/html/scripts/timeZone.js", "text/javascript");
    });

    _server.on("/timeZoneData", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String part;
        if (request->hasParam("part")) {
            part = request->getParam("part")->value();
        }

        Serial.print("Call /timeZoneData: ");
        Serial.println(part);

        JsonDocument doc;
        JsonArray timeZoneArray = doc.to<JsonArray>();

        int part_i = part.toInt() - 1;
        for (int i = part_i * 20; i < (20 + part_i * 17); i++) {
            char location[50];
            char timeZone[50];
            char timeDifference[50];

            for (byte k = 0; k < 50; k++) {
                location[k] = pgm_read_byte_near(timeZones[0][i] + k);
                timeZone[k] = pgm_read_byte_near(timeZones[1][i] + k);
                timeDifference[k] = pgm_read_byte_near(timeZones[2][i] + k);
            }

            char entry[150];
            strcpy(entry, location);
            strcat(entry, ": ");
            strcat(entry, timeZone);
            strcat(entry, " (");
            strcat(entry, timeDifference);
            strcat(entry, ")");

            JsonObject timeZoneObj = timeZoneArray.add<JsonObject>();
            timeZoneObj["entry"] = entry;
        }

        String jsonString;
        serializeJson(doc, jsonString);
        request->send(200, "application/json", jsonString);
    });

    _server.on("/timeZoneUpdate", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            String newTimeZone = request->getParam("value")->value();
            int newTimeZone_i = newTimeZone.toInt();
            Serial.println("New time zone = " + newTimeZone);

            char timeZone[50];
            for (byte k = 0; k < 50; k++) {
                timeZone[k] = pgm_read_byte_near(timeZones[3][newTimeZone_i - 1] + k);
            }

            String timeZone_s(timeZone);
            _timeZone_f.write_f(timeZone_s);

            my_ESP ESP_temp;
            ESP_temp.setMyTime();

            Serial.println(timeZone_s);
        } else {
            Serial.println("No time zone sent");
        }
        request->send(200, "text/plain", "OK");
    });

    _server.on("/server-time", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String jsonResponse = "{";
        jsonResponse += "\"hour\": " + String(myESP.Hour) + ",";
        jsonResponse += "\"minute\": " + String(myESP.Min) + ",";
        jsonResponse += "\"second\": " + String(myESP.Sec);
        jsonResponse += "}";

        request->send(200, "application/json", jsonResponse);
    });

    // Start Server
    _server.begin();
    Serial.println("Server started");
}

void webInterface::updateWI(int w_icon, String w_temp, String w_humi) {
    _w_icon = w_icon;
    _w_temp = w_temp;
    _w_humi = w_humi;

    _ws.textAll("weather data ready");
}

String webInterface::_currentPath = "";

void webInterface::_handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    String logMessage = "";
    String myPath = _currentPath + filename;

    if (!index) {
        logMessage = "Upload Start: " + myPath;
        Serial.println(logMessage);

        request->_tempFile = LittleFS.open(myPath, "w");
        if (!request->_tempFile) {
            Serial.println("Error: Could not open file");
        } else {
            Serial.println("File opened successfully");
        }
    }

    if (len) {
        request->_tempFile.write(data, len);
        logMessage = "Writing file: " + myPath + " index=" + String(index) + " len=" + String(len);
        Serial.println(logMessage);
    }

    if (final) {
        logMessage = "Upload Complete: " + myPath + ", size: " + String(index + len);
        Serial.println(logMessage);
        request->_tempFile.close();
        request->redirect("/");
    }
}
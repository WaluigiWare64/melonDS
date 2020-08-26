/*
    Copyright 2016-2020 Arisotura, WaluigiWare64
    This file is part of melonDS.
    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.
    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <iostream>
#include <string>
#include <cstring> 
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdio>

namespace Updater
{
using json = nlohmann::json;
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
std::string getCURL(const char* webURL, const char* authFlag="", bool fwrite=false, const char* fname="") 
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
  
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, webURL);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.77.1");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	
        curl_easy_setopt(curl, CURLOPT_PASSWORD, authFlag);
	    if (fwrite)
	    {
            fp = fopen(fname,"wb");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        } 
        else
        {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		
        }
    
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
	    {
            return "cURL Error.";
	    }
	    curl_easy_cleanup(curl);
	    if (fwrite) {fclose(fp); return "";} else {return readBuffer;}
    
     }
}
int checkForUpdates(std::string currentVer, const char* authFlag)
{
    #if defined(_WIN32)
        const char* platform = "melonDS-win-x86_64";
    #elif defined(__linux__) && !defined(__aarch64__)
	    const char* platform = "melonDS-ubuntu-x86_64";
    #elif defined(__linux__) && defined(__aarch64__)
	    const char* platform = "melonDS-ubuntu-aarch64";	
    #endif
	
    std::string latestArtifact, latestArtifactVer;
    json jsonArtifact = json::parse(getCURL("https://api.github.com/repos/Arisotura/melonDS/actions/artifacts", authFlag));
  
    for (const auto& obj : jsonArtifact["artifacts"]) 
    {
        if (obj["name"] == platform) 
        {
            latestArtifact = obj["archive_download_url"];
	        latestArtifactVer = obj["url"];
            latestArtifactVer = latestArtifactVer.substr(latestArtifactVer.find_last_of("/")+1);
        }
    }
    if (latestArtifactVer == currentVer)
    {
        return 0;
    }
    else
    {
    getCURL(latestArtifact.c_str(), authFlag, true, "melonDS.zip");
        return 1;
    }
}
int installUpdate()
{
    //todo
}
}

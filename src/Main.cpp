#include "imio.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#define access _access
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "vecmatquat.h"

std::vector<std::string> GetFilesInDirectory(const std::string &directory)
{
    std::vector<std::string> out;
#ifdef WIN32
    HANDLE dir;
    WIN32_FIND_DATA file_data;

    if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
        return out; /* No files found */

    do {
        const std::string file_name = file_data.cFileName;
        const std::string full_file_name = directory + "/" + file_name;
        const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (file_name[0] == '.')
            continue;

        if (is_directory)
            continue;

        out.push_back(full_file_name);
    } while (FindNextFile(dir, &file_data));

    FindClose(dir);
#else
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    dir = opendir(directory.c_str());
    while ((ent = readdir(dir)) != NULL) {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = directory + "/" + file_name;

        if (file_name[0] == '.')
            continue;

        if (stat(full_file_name.c_str(), &st) == -1)
            continue;

        const bool is_directory = (st.st_mode & S_IFDIR) != 0;

        if (is_directory)
            continue;

        out.push_back(full_file_name);
    }
    closedir(dir);
#endif
    return out;
} // GetFilesInDirectory

struct cross_t {
    int color;
    int len;
};


int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;

    std::string filePath = std::string(argv[1]) + '/';
    auto files = GetFilesInDirectory(filePath);
    cross_t crosses[] = { { 128, 20 }, { 255, 20 }, { 0, 20 } };
    for (const auto & file : files)
    {
        if (file.find("images_eye") != std::string::npos && file.find("png") != std::string::npos)
        {
            std::cout << file << std::endl;
            auto img2 = img::imread<uint8_t, 1>(file.c_str());
            auto img = img2.copy();
            int idx = 0;
            std::vector<std::vector<int2>> found_crosses(sizeof(crosses) / sizeof(cross_t));

            for (const auto & crs : crosses) {
                for (int y = 0; y < img.height - (2*crs.len + 1); y++) {
                    for (int x = crs.len + 1; x < img.width - (crs.len + 1); x++) {
                        
                        // search y
                        int vCnt = 0;
                        int hCnt = 0;
                        // require 2 corners
                        auto crnerCnt = 0;
                        crnerCnt += (img.ptr[(y)*img.width + (x)] == crs.color) ? 1 : 0;
                        crnerCnt += (img.ptr[(y+crs.len)*img.width + (x - crs.len)] == crs.color) ? 1 : 0;
                        crnerCnt += (img.ptr[(y+crs.len)*img.width + (x + crs.len)] == crs.color) ? 1 : 0;
                        crnerCnt += (img.ptr[(y+2*crs.len)*img.width + (x)] == crs.color) ? 1 : 0;
                        if (crnerCnt < 2)
                            continue;

                        for (int yi = 0; yi < 2 * crs.len + 1; yi++) {
                            auto px = img.ptr[(y + yi)*img.width + x];
                            if (px == crs.color || px ==4) {
                                vCnt++;
                            }
                        }
                        if (vCnt >= 2 * crs.len + 1 - 2){
                            // search x
                            for (int xi = 0; xi < 2 * crs.len + 1; xi++) {
                                auto px = img.ptr[(y + crs.len)*img.width + (x + xi - crs.len)];
                                if (px == crs.color || px == 4) {
                                    hCnt++;
                                }
                            }
                            if (hCnt >= 2 * crs.len + 1 - 2){
                                int vSameCnt  =0 ;
                                for (int yi = 0; yi < 2 * crs.len + 1; yi++) {
                                    if (img.ptr[(y + yi)*img.width + x + 1] == crs.color && img.ptr[(y + yi)*img.width + x - 1] == crs.color) {
                                        vSameCnt++;
                                    }
                                }
                                if (vSameCnt < crs.len ){
                                    found_crosses[idx].emplace_back( x + crs.len, y + crs.len );
                                    std::cout << crs.color << '\t' << x + crs.len << '\t' << y + crs.len << std::endl;

                                    // don't cares are 4
                                    for (int yi = 0; yi < 2 * crs.len + 1; yi++) {
                                        if (img.ptr[(y + yi)*img.width + x] == crs.color) {
                                            img.ptr[(y + yi)*img.width + x] = 4;
                                        }
                                    }
                                    for (int xi = 0; xi < 2 * crs.len + 1; xi++) {
                                        if (img.ptr[(y + crs.len)*img.width + (x + xi - crs.len)] == crs.color) {
                                            img.ptr[(y + crs.len)*img.width + (x + xi - crs.len)] = 4;
                                        }
                                    }

                                }
                            }
                        }
                    }
                }
                idx++;
            }
            bool hasTwo = true;
            for (const auto & crs : found_crosses) {
                if (crs.size() != 2)
                    hasTwo = false;
            }
            if(!hasTwo)
                continue;

            auto baseName = file.substr(0, file.size() - 4);
            std::ofstream logFile(baseName + ".log");
            for (size_t i = 0; i < found_crosses.size(); i++){
                std::sort(found_crosses[i].begin(), found_crosses[i].end(), [](int2 a, int2 b){ return a.x < b.x; });
                logFile << found_crosses[i][0].x << ',' << found_crosses[i][0].y << ',' << found_crosses[i][1].x << ',' << found_crosses[i][1].y;
                if (i != found_crosses.size() - 1)
                    logFile << '\n';
            }
        }
    }

    return 0;
}

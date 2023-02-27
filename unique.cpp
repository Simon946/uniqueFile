#include <iostream>
#include <dirent.h>
#include <string>
#include "sha1.cpp"
#include "hashtable.cpp"

HashTable<std::string> table;
const int FILESYSTEM_BLOCK_SIZE = 4096;
const int FILESYSTEM_DIRECTORY = 4;
const int FILESYSTEM_FILE = 8;

bool filesAreEqual(std::string fileName1, std::string fileName2){
    std::ifstream file1(fileName1.c_str(), std::ios::binary);
    std::ifstream file2(fileName2.c_str(), std::ios::binary);

    if(!file1.is_open() || !file2.is_open()){
        std::cout << "could not open files" << std::endl;
        return false;
    }
    std::string hash1 = SHA1::hash(file1);;
    std::string hash2 = SHA1::hash(file2);
    file1.close();
    file2.close();
    return hash1 == hash2;
}


void addToTable(std::string fullFileName){
    uint8_t* data = new uint8_t[FILESYSTEM_BLOCK_SIZE];
    std::ifstream file(fullFileName.c_str());

    if(!file.is_open()){
        std::cout << "failed to open file" << std::endl;
        return;
    }
    file.read((char*)data, FILESYSTEM_BLOCK_SIZE);
    size_t fileSize = file.gcount();
    file.close();

    std::string hashValue = SHA1::hash(data, fileSize);

    if(table.get(hashValue) != table.defaultValue){
        std::string existingFullFileName = table.get(hashValue);

        if(filesAreEqual(existingFullFileName, fullFileName)){
            std::cout << "duplicate file found: " << existingFullFileName << " is exactly the same as: " << fullFileName << std::endl;
        }
        else{
            std::cout << "found two files that start the same " << existingFullFileName << " and " << fullFileName << std::endl;
        }
    }
    else{
        table.insert(hashValue, fullFileName);
    }
    delete data;
}

size_t loadDir(std::string directoryName){
    DIR* dir = opendir(directoryName.c_str());
    size_t filesOpened = 0;

    if(dir == NULL){
        std::cerr << "Unable to open directory" << directoryName << std::endl;
        return 0;
    }
    struct dirent* currentFile = new struct dirent;

    do{
        currentFile = readdir(dir);

        if(currentFile != NULL){
                        
            std::string localFileName = currentFile->d_name;
            std::string fullFileName = directoryName + '/'  + localFileName;

            switch(currentFile->d_type){
                case FILESYSTEM_DIRECTORY:{
                    if(localFileName != "." && localFileName != ".."){
                        filesOpened += loadDir(fullFileName);
                    }
                    break;
                }
                case FILESYSTEM_FILE:{
                    filesOpened++;
                    addToTable(fullFileName);
                    break;
                }
                default:{
                    std::cerr << "thats a strange file type: " << (int)currentFile->d_type << std::endl;
                }
            }   
        }

    }while(currentFile != NULL);
    delete currentFile;
    return filesOpened;
}

void searchFile(std::string fileName){
    std::string fileHash = "";
    uint8_t* data = new uint8_t[FILESYSTEM_BLOCK_SIZE];
    std::ifstream file(fileName.c_str(), std::ios::binary);

    if(!file.is_open()){
        std::cout << "failed to open file" << std::endl;
        file.close();
        return;
    }
    file.read((char*)data, FILESYSTEM_BLOCK_SIZE);
    fileHash = SHA1::hash(data, file.gcount());
    delete data;
    file.close();

    if(table.get(fileHash) == table.defaultValue){
        std::cout << "that file is not in the directory" << std::endl;
        return;
    }
    std::string existingFileName = table.get(fileHash);
    
    if(filesAreEqual(fileName, existingFileName)){
        std::cout << "found an exact copy at: " << existingFileName << std::endl;
    }
    else{
        std::cout << "found a similar but different file at: " << existingFileName << std::endl;
    }
}

int main(){
    table.defaultValue = "/\\This file does not exist/\\";
    std::cout << "Enter a directory:";
    std::string directoryName = "a";
    std::cin >> directoryName;
    size_t filesOpened = loadDir(directoryName);
    std::cout << "opened: " << filesOpened << " files" << std::endl;
    std::cout << "Directory loaded" << std::endl;
    std::string fileName = "";
    do{
        std::cout << "Enter a file name: (or exit)";
        std::cin >> fileName;

        if(fileName != "exit"){
            std::cout << "Searching..." << std::endl;
            searchFile(fileName);
        }

    }while(fileName != "exit");

    //table.print();
    return 0;
}
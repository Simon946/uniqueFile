#ifndef SHA1cpp
#define SHA1cpp

#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <stdint.h>

namespace SHA1{
    const size_t BLOCK_INTS = 16;  // number of 32bit integers per SHA1 block
    const size_t BLOCK_BYTES = BLOCK_INTS * 4;
    const size_t EXTENDED_BLOCK_INTS = 80;

    uint32_t reverseEndian(uint32_t value){
        uint32_t result = value & 0xFF;
        for(int i = 0; i < 3; i++){
            result <<= 8;
            value >>= 8;
            result |= value & 0xFF;
        }   
        return result; 
    }

    uint32_t leftRotate(uint32_t value, int bits){
        return (value << bits) | (value >> (32 - bits));
    }

    class SHA1Hash{
        public:
            std::string getHash();
            void loadBlocks(uint8_t* data, size_t size);
            void hashBlock(uint32_t* block);
            void fromStream(std::istream& stream);
            uint32_t hashValue[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
    };

    std::string hash(std::istream& stream){
        SHA1Hash hasher;
        hasher.fromStream(stream);
        return hasher.getHash();
    }

    std::string hash(uint8_t* data, size_t size){
        SHA1Hash hasher;
        hasher.loadBlocks(data, size);
        return hasher.getHash();
    }

    std::string hash(std::string string){
        uint8_t* data = new uint8_t[string.length()];

        for(int i = 0; i < string.length(); i++){
            data[i] = string.at(i);
        }
        std::string result = hash(data, string.length());
        delete data;
        return result;
    }

    std::string SHA1Hash::getHash(){
        std::ostringstream result;

        for (size_t i = 0; i < 5; i++){
            result << std::hex << std::setfill('0') << std::setw(8);
            result << hashValue[i] << ' ';
        }
        return result.str();
    }

    void SHA1Hash::fromStream(std::istream& stream){

        if(!stream.good()){
            std::cerr << "stream is not good" << std::endl;
            return;
        }
        if(stream.fail()){
            std::cout << "stream failed before starting" << std::endl;
            return;
        }       

        uint32_t* block = new uint32_t[BLOCK_INTS];
        uint64_t streamSize = 0;
        bool lastBlock = false;

        while(!lastBlock){
            stream.read((char*)block, BLOCK_BYTES);

            size_t bytesRead = stream.gcount();
            streamSize += bytesRead;

            if(bytesRead != BLOCK_BYTES){
                lastBlock = true;
                uint8_t* bytes = (uint8_t*)block;
                bytes[bytesRead] = 0x80;//append 1 bit to mark end of data
                bytesRead++;

                for(int i = bytesRead; i < BLOCK_BYTES; i++){
                    bytes[i] = 0;//write padding
                }
                for(int i = 0; i < BLOCK_INTS; i++){
                    block[i] = reverseEndian(block[i]);
                } 
                if(bytesRead < BLOCK_BYTES - 2){
                    streamSize *= 8; //to get number of bits in data
                    block[BLOCK_INTS -1] = (uint32_t)streamSize;
                    block[BLOCK_INTS -2] = (uint64_t)(streamSize >> 32);
                    hashBlock(block);
                    break;
                }
                else{
                    hashBlock(block);
                    uint32_t* paddingBlock = new uint32_t[BLOCK_INTS];
                    
                    for(int i  = 0; i < BLOCK_INTS; i++){
                        paddingBlock[i] = 0;
                    }

                    streamSize *= 8; //to get number of bits in data
                    paddingBlock[BLOCK_INTS -1] = (uint32_t)streamSize;
                    paddingBlock[BLOCK_INTS -2] = (uint64_t)(streamSize >> 32);
                    hashBlock(paddingBlock);
                    delete paddingBlock;
                    break;
                }
            }
            for(int i = 0; i < BLOCK_INTS; i++){
                block[i] = reverseEndian(block[i]);
            }
            hashBlock(block);
        }
        delete block;
    }

    void SHA1Hash::loadBlocks(uint8_t* data, size_t size){
        uint32_t* block = new uint32_t[BLOCK_INTS];
        uint64_t streamSize = 0;
        bool lastBlock = false;

        while(!lastBlock){

            size_t bytesRead = (streamSize + BLOCK_BYTES <= size)? BLOCK_BYTES : size - streamSize;
            
            uint8_t* tmp = (uint8_t*)block;

            for(int i = 0; i < bytesRead; i++){
                tmp[i] = data[streamSize + i];
            }

            streamSize += bytesRead;
            
            if(bytesRead != BLOCK_BYTES){
                lastBlock = true;
                uint8_t* bytes = (uint8_t*)block;
                bytes[bytesRead] = 0x80;//append 1 bit to mark end of data
                bytesRead++;

                for(int i = bytesRead; i < BLOCK_BYTES; i++){
                    bytes[i] = 0;//write padding
                }
                for(int i = 0; i < BLOCK_INTS; i++){
                    block[i] = reverseEndian(block[i]);
                }
                
        
                if(bytesRead < BLOCK_BYTES - 2){
                    streamSize *= 8; //to get number of bits in data
                    block[BLOCK_INTS -1] = (uint32_t)streamSize;
                    block[BLOCK_INTS -2] = (uint64_t)(streamSize >> 32);
                    
                    hashBlock(block);
                    break;
                }
                else{
                    hashBlock(block);
                    uint32_t* paddingBlock = new uint32_t[BLOCK_INTS];
                    
                    for(int i  = 0; i < BLOCK_INTS; i++){
                        paddingBlock[i] = 0;
                    }

                    streamSize *= 8; //to get number of bits in data
                    paddingBlock[BLOCK_INTS -1] = (uint32_t)streamSize;
                    paddingBlock[BLOCK_INTS -2] = (uint64_t)(streamSize >> 32);
                    hashBlock(paddingBlock);
                    delete paddingBlock;
                    break;
                }
            }
            for(int i = 0; i < BLOCK_INTS; i++){
                block[i] = reverseEndian(block[i]);
            }
            hashBlock(block);
        }
        delete block;
    }

    void printBlock(uint32_t* block){
        std::cout << "Printing block: ";
        for(int i = 0; i < BLOCK_INTS; i++){
            std::cout << std::hex << block[i] << ' ';
        }
        std::cout << std::endl;
    }

    void SHA1Hash::hashBlock(uint32_t* block){     
        uint32_t* extendedBlock = new uint32_t[EXTENDED_BLOCK_INTS];
        //printBlock(block);

        for(int i = 0; i < EXTENDED_BLOCK_INTS; i++){

            if(i < 16){
                extendedBlock[i] = block[i];
            }
            else{
                extendedBlock[i] = leftRotate(extendedBlock[i - 3] ^ extendedBlock[i - 8] ^ extendedBlock[i - 14] ^ extendedBlock[i - 16], 1);
            }
        }
        uint32_t a = hashValue[0];
        uint32_t b = hashValue[1];
        uint32_t c = hashValue[2];
        uint32_t d = hashValue[3];
        uint32_t e = hashValue[4];
        uint32_t f = 0;
        uint32_t k = 0;

        for(int i = 0; i < EXTENDED_BLOCK_INTS; i++){
            
            if(i >= 0 && i < 20){
                f = (b & c) ^ ((~b) & d);
                k = 0x5A827999;
            }
            if(i >= 20 && i < 40){
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            if(i >= 40 && i < 60){
                f = (b & c) ^ (b & d) ^ (c & d);
                k = 0x8F1BBCDC;
            }
            if(i >= 60 && i < 80){
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = leftRotate(a, 5) + f + e + k + extendedBlock[i];
            e = d;
            d = c;
            c = leftRotate(b, 30);
            b = a;
            a = temp;
        }
        hashValue[0] += a;
        hashValue[1] += b;
        hashValue[2] += c;
        hashValue[3] += d;
        hashValue[4] += e;
        delete extendedBlock;
    }
}
#endif

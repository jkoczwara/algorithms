//
//  main.cpp
//  HuffmanCoding
//
//  Created by Jakub Koczwara on 19.12.2016.
//  Copyright © 2016 Jakub Koczwara. All rights reserved.
//

#include <iostream>
#include <assert.h>
#include <cstdio>
#include <map>
#include <queue>
#include <vector>


#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef char u8;

global_variable u8 input_name[] = "input.txt";
global_variable u8 output_name[] = "output.txt";

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// ***************** UTILS *****************

struct file_contents {
    size_t fileSize;
    u8 *contents;
};

internal file_contents
readEntireFileToMemory(u8 *fileName){
    file_contents result = {};
    FILE *file = fopen(fileName, "r");
    
    assert(file);
    
    fseek(file, 0, SEEK_END);
    result.fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    result.contents = (u8*) malloc(result.fileSize);

    fread(result.contents, 1, result.fileSize, file);
    result.contents[result.fileSize] = '\0';
    
    fclose(file);
    
    return result;
}

internal void
writeFileToDisk(int32 size, u8 *content, u8 *fileName) {
    FILE *outFile = fopen(fileName, "w");
    
    if (outFile) {
        fwrite(content, 1, size, outFile);
    }
}

// ***************** ALGORITHM *****************

struct huffman_node {
    u8 letter;
    int32 count;
    huffman_node *left, *right;
};

struct queue_comparator {
    bool operator() (const huffman_node *left, const huffman_node *right) {
        return left->count > right->count;
    }
};

internal std::map<u8, int32>
computeFrequenceOfLetters(int32 size, u8 *buffer) {
    std::map<u8, int32> freq;
    while (size--) {
        u8 letter = *buffer++;
        if (freq.find(letter) == freq.end()){
            freq[letter] = 1;
        } else {
            freq[letter] += 1;
        }
    }
    
    return freq;
}

internal void
buildCodes(huffman_node *node, std::string prefix, std::map<u8, std::string> *codes){
    if (node->left == nullptr && node->right == nullptr) {
        // leaf
        (*codes)[node->letter] = prefix;
        return;
    } else {
        buildCodes(node->left, prefix + '0', codes);
        buildCodes(node->right, prefix + '1', codes);
    }
}

internal file_contents
encode(int32 inSize, u8 *in){
    std::map<u8, int32> freq = computeFrequenceOfLetters(inSize, in);
    
    std::priority_queue<huffman_node*, std::vector<huffman_node*>, queue_comparator> huffman_table;
    for (auto entry: freq) {
        huffman_node *node = new huffman_node;
        
        node->letter = entry.first;
        node->count = entry.second;
        node->left = nullptr;
        node->right = nullptr;
        
        huffman_table.emplace(node);
    }
    
    // build the huffman tree
    huffman_node *newNode = nullptr;
    while(huffman_table.size() >= 2) {
        huffman_node *first_low = huffman_table.top();
        huffman_table.pop();
        
        huffman_node *second_low = huffman_table.top();
        huffman_table.pop();
        
        newNode = new huffman_node;
        newNode->letter = '?';
        newNode->count = first_low->count + second_low->count;
        newNode->left = first_low;
        newNode->right = second_low;
        
        huffman_table.emplace(newNode);
    }
    
    huffman_node *root = huffman_table.top();

    std::map<u8, std::string> *codes = new std::map<u8, std::string>;
    buildCodes(root, "", codes);
    
    std::string outputString = "";
    while (inSize--) {
        outputString += (*codes)[*in++].c_str();
    }

    file_contents ret = {};
    ret.fileSize = outputString.size();
    ret.contents = (u8*)malloc(outputString.size() + 1);
    strcpy(ret.contents, outputString.c_str());
    
    return ret;
}

internal file_contents
decode(int32 inSize, u8 *in){
    
    return {};
}

// TODO wrong buffer size (/xaa chars) in readEntireFileToMemory method - causes u8* to not terminate with \0
// TODO saving code table to file
// TODO decoding
// TODO tests


int main(int argc, const char * argv[]) {
    file_contents inputContents = readEntireFileToMemory(input_name);
    file_contents outputContents = encode((int32)inputContents.fileSize, inputContents.contents);
    writeFileToDisk((int32)outputContents.fileSize, outputContents.contents, output_name);
    
//    file_contents encodedInput = readEntireFileToMemory(output_name);
//    file_contents decodedInput = decode((int32) encodedInput.fileSize, encodedInput.contents);
    
    std::cout << "Encoder input: \n" << inputContents.contents << "\n";
    std::cout << "Encoder output: \n" << outputContents.contents << "\n\n\n";
//    std::cout << "Decoder input: \n" << encodedInput.contents << "\n";
//    std::cout << "Decoder output: \n" << decodedInput.contents << "\n";
    
    std::cout << "Comprass ratio: " << inputContents.fileSize * 8 << "\\" << outputContents.fileSize << "\n";
    
    return 0;
}

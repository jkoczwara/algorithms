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
#include <sstream>
#include <string>


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

global_variable u8 input_name[] = "input_3.txt";
global_variable u8 output_name[] = "output.txt";
global_variable u8 output_name_decoded[] = "output-decoded.txt";
global_variable u8 output_name_code_table[] = "output-coding.txt";

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// ***************** UTILS *****************

struct file_contents {
    size_t fileSize;
    u8 *contents;
};

struct encoded_file_contents {
    file_contents encodedFile;
    file_contents codeTable;
};

internal file_contents
readEntireFileToMemory(u8 *fileName){
    file_contents result = {};
    FILE *file = fopen(fileName, "r");
    
    assert(file);
    
    fseek(file, 0, SEEK_END);
    result.fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    result.contents = (u8*) malloc(result.fileSize + 1);
    
    fread(result.contents, 1, result.fileSize, file);
    result.contents[result.fileSize] = '\0';
    
    fclose(file);
    
    return result;
}

internal void
writeFileToDisk(int32 size, u8 *content, u8 *fileName) {
    FILE *outFile = fopen(fileName, "w");
    
    assert(outFile);
    fwrite(content, 1, size, outFile);
    
    
    fclose(outFile);
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

internal encoded_file_contents
encode(int32 inSize, u8 *in){
    // calculate frequencies of letters in input string
    std::map<u8, int32> freq = computeFrequenceOfLetters(inSize, in);
    
    // make huffman table of nodes which contains frequency, letter and null references to subtrees
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
    
    // assign root of huffman tree to last element from huffman table
    huffman_node *root = huffman_table.top();
    
    // build coding table
    std::map<u8, std::string> *codes = new std::map<u8, std::string>;
    buildCodes(root, "", codes);
    
    // make string out of encoded letters
    std::string outputString = "";
    while (inSize--) {
        outputString += (*codes)[*in++].c_str();
    }
    file_contents retEncoded = {};
    retEncoded.fileSize = outputString.size();
    retEncoded.contents = (u8*)malloc(outputString.size() + 1);
    strcpy(retEncoded.contents, outputString.c_str());
    
    // make string out of coding table
    outputString = "";
    for (auto& kv : *codes) {
        outputString += kv.first;
        outputString += "=";
        outputString += kv.second;
        outputString += ",";
    }
    file_contents retCoding = {};
    retCoding.fileSize = outputString.size();
    retCoding.contents = (u8*)malloc(outputString.size() + 1);
    strcpy(retCoding.contents, outputString.c_str());
    
    // assign to return struct
    encoded_file_contents ret {};
    ret.encodedFile = retEncoded;
    ret.codeTable = retCoding;
    
    return ret;
}

internal std::map<std::string, u8>
makeCodeTable(u8 *in){
    std::map<std::string, u8> ret;
    std::string tempString = std::string(in);
    std::stringstream ss(tempString);
    std::vector<std::string> vect;
    
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t pos   = token.find('=');
        std::string letter = token.substr(0, pos);
        std::string code   = token.substr(pos + 1);
        ret[code] = letter.at(0);
    }
    
    return ret;
}

internal file_contents
decode(int32 inSize, u8 *in, int32 codeTableSize, u8 *codeTable){
    std::map<std::string, u8> codes = makeCodeTable(codeTable);
    std::string outputString = "";
    
    // decode input string
    std::string code = "";
    for (int i = 0; i < inSize; ++i) {
        code += in[i];
        if ( codes.count(code)) {
            // append character from code table to output string
            outputString += codes.at(code);
            code = "";
        }
    }
    
    file_contents retEncoded = {};
    retEncoded.fileSize = outputString.size();
    retEncoded.contents = (u8*)malloc(outputString.size() + 1);
    strcpy(retEncoded.contents, outputString.c_str());
    return retEncoded;
}

int main(int argc, const char * argv[]) {
    file_contents inputContents = readEntireFileToMemory(input_name);
    encoded_file_contents outputContents = encode((int32)inputContents.fileSize, inputContents.contents);
    writeFileToDisk((int32)outputContents.encodedFile.fileSize, outputContents.encodedFile.contents, output_name);
    writeFileToDisk((int32)outputContents.codeTable.fileSize, outputContents.codeTable.contents, output_name_code_table);

    std::cout << "\nEncoder input: \n" << inputContents.contents << "\n";
    std::cout << "\nEncoder output: \n" << outputContents.encodedFile.contents << "\n";
    std::cout << "\nEncoder output codes: \n" << outputContents.codeTable.contents << "\n\n";

    file_contents encodedInput = readEntireFileToMemory(output_name);
    file_contents codeTableInput = readEntireFileToMemory(output_name_code_table);
    file_contents decodedInput = decode((int32) encodedInput.fileSize, encodedInput.contents, (int32) codeTableInput.fileSize, codeTableInput.contents);
    
    std::cout << "\nDecoder input: \n" << encodedInput.contents << "\n";
    std::cout << "\nDecoder output: \n" << decodedInput.contents << "\n\n";
    
    writeFileToDisk((int32)decodedInput.fileSize, decodedInput.contents, output_name_decoded);
    
//    std::cout << "\nComprass ratio (input/output [bytes]): " << inputContents.fileSize * 8 << "/" << outputContents.encodedFile.fileSize << " = " << (double)inputContents.fileSize * 8 / (double)outputContents.encodedFile.fileSize<< "\n";
    
    return 0;
}

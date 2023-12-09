#ifndef HUFFMAN_H
#define HUFFMAN_H

/// Top level function that implements the huffman algorithm. Reads
/// the contents from \p in_path and outputs the encoded file to
/// \p out_path.
/// 
void huffman_encode(const char* inFile, const char* outFile);

/// Top level function that implements the huffman algorithm. Reads
/// the contents from \p in_path and decodes the huffman codes and writes
/// out the original message to \p out_path.
/// 
void huffman_decode(const char* inFile, const char* outFile);

#endif

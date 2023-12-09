/*
 * A simple huffman encoder/decoder for files.
 *
 * Use:
 *   ./Huffman encode [inFile] [outFile]
 *   ./Huffman decode [inFile] [outFile]
 *
 */
#include "huffman.h"
#include <iostream>
#include <algorithm>

void print_options();

int main(int argc, char** argv) {

	if (argc < 4) {
		print_options();
		return 1;
	}

	std::string action = argv[1];
	std::transform(action.begin(), action.end(), action.begin(), ::tolower);

	if (action == "encode") {
		huffman_encode(argv[2], argv[3]);
	} else if (action == "decode") {
		huffman_decode(argv[2], argv[3]);
	} else {
		print_options();
		return 1;
	}

	return 0;
}

void print_options() {
  std::cout << "Optional calls:\n";
  std::cout << "Huffman encode [inFile] [outFile]\n";
  std::cout << "Huffman decode [inFile] [outFile]\n";
}

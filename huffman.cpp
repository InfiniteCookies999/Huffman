#include "huffman.h"
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <iostream>
#include <queue>

static const char DEFAULT_CHAR = '$';
static size_t num_nodes = 0;

// Hode within the huffman tree that is built when encoding or decoding.
struct HNode {
	size_t id;
	// Originally the value is the frequency of the character then once
	// nodes are combined it becomes the value of the combined nodes.
	size_t value;
	HNode* ln, *rn;
	std::string encoding;
	char ch = DEFAULT_CHAR;
};

HNode* create_node(size_t value, HNode* ln, HNode* rn, char ch) {
	++num_nodes;
	return new HNode { num_nodes, value, ln, rn, "", ch };
}

/// Creates a map between a character and how many times it appeared
/// within the \p msg.
///
std::unordered_map<char, HNode*> calculate_frequences(const std::string& msg) {
	std::unordered_map<char, HNode*> freqs;
	// We begin by finding frequencies.
	for (const char& ch : msg) {
		if (freqs.find(ch) == freqs.end()) {
			HNode* node = create_node(1, nullptr, nullptr, ch);
			freqs.insert({ ch, node });
		} else {
			++freqs[ch]->value;
		}
	}
	return freqs;
}

/// Calling this function with the top node of a huffman tree
/// will result in encoding each node with its appropriate binary
/// string.
///
void create_encodings(HNode* node, std::string binary = "") {
	if (node->ln != nullptr) {
		// As long as we are not a leaf!
		create_encodings(node->rn, binary + "0");
		create_encodings(node->ln, binary + "1");
		// delete node;
		return;
	}

	node->encoding = binary;
}

/// Iterates over every character in \p msg and writes its binary string to
/// the \p out_stream.
///
void write_encoding(std::ofstream& out_stream, const std::string& msg,
	                std::unordered_map<char, HNode*>& freqs) {
	char cbyte = 0;
	size_t bidx = 0;
	for (char ch : msg) {
		const std::string& encoding = freqs[ch]->encoding;
		for (char bin : encoding) {
			if (bin == '1') {
				cbyte |= 1 << bidx;
			}
			++bidx;
			if (bidx == 8) {
				out_stream.write(&cbyte, 1);
				cbyte = 0;
				bidx  = 0;
			}
		}
	}
	// Write out last byte if it exists.
	if (bidx != 0) {
		out_stream.write(&cbyte, 1);
	}
}

/// Calling this function with the top node of the huffman tree
/// it will write out information for every node with the following
/// structures:
/// 
/// [id] [character] [left node id] [right node id].
/// 
/// If the node does not have children it will write zero for both
/// ids.
/// 
/// This encoding when read back in can be used to reconstruct the
/// huffman tree.
void write_tree(std::ofstream& out_stream, HNode* node) {
	out_stream << (char)node->id << node->ch;
	if (node->ln) {
		out_stream << (char)node->ln->id << (char)node->rn->id;
		write_tree(out_stream, node->ln);
		write_tree(out_stream, node->rn);
	} else {
		char c = 0;
		out_stream << c << c;
	}
}

/// This function writes out how many characters were in the msg
/// then will write how many nodes are within the huffman tree after which
/// it will call a function to write all the needed information to reconstruct
/// the huffman tree.
/// 
void write_table(std::ofstream& out_stream, size_t msg_size, HNode* top) {
	out_stream << msg_size << " ";
	out_stream << (char) num_nodes;
	write_tree(out_stream, top);
}

void huffman_encode(const char* in_path, const char* out_path) {
	num_nodes = 0;

	std::ifstream in_stream(in_path, std::ios::binary | std::ios::in);
	if (in_stream.fail()) {
		perror(in_path);
		return;
	}
	std::string msg((std::istreambuf_iterator<char>(in_stream)),
                      (std::istreambuf_iterator<char>()));
	in_stream.close();

	std::ofstream out_stream(out_path, std::ios::binary | std::ios::out);
	if (out_stream.fail()) {
		perror(out_path);
		return;
	}
	
	// No input so no output.
	if (msg.empty()) return;
	

	std::unordered_map<char, HNode*> freqs = calculate_frequences(msg);
	
	// Using a priority queue to quickly resolve nodes with minimal
	// values.
	auto sorter = [](const HNode* n1, const HNode* n2) -> bool {
		return n1->value > n2->value;
	};
	std::priority_queue<HNode*, std::vector<HNode*>,
		decltype(sorter)> node_queue(sorter);
	std::vector<HNode*> nodes;
	
	for (auto [ch, node] : freqs) {
		node_queue.push(node);
		nodes.push_back(node);
	}

	while (node_queue.size() != 1) {
		HNode* sn1 = node_queue.top(); node_queue.pop();
		HNode* sn2 = node_queue.top(); node_queue.pop();

		HNode* parent = create_node(sn1->value + sn2->value, sn1, sn2, DEFAULT_CHAR);
		node_queue.push(parent);
		nodes.push_back(parent);
	}

	create_encodings(node_queue.top());

	write_table(out_stream, msg.size(), node_queue.top());
	write_encoding(out_stream, msg, freqs);

	// Cleaning up allocated nodes.
	for (HNode* node : nodes) {
		delete node;
	}
}

/// Traverses the huffman tree when supplied with the top node of the huffman tree
/// and an encoding to match. It will try and find a node within the tree with a
/// matching binary encoding. If no such node is found this function returns nullptr.
/// 
HNode* get_node(HNode* node, const std::string& encoding, size_t depth = 0) {
	if (encoding.length() != depth && node->ln) {
		if (HNode* ln = get_node(node->ln, encoding, depth + 1)) {
			return ln;
		}
		if (HNode* rn = get_node(node->rn, encoding, depth + 1)) {
			return rn;
		}
	} else if (node->encoding == encoding && !node->ln) {
		return node;
	}
	return nullptr;
}

void huffman_decode(const char* in_path, const char* out_path) {
	num_nodes = 0;

	std::ifstream in_stream(in_path, std::ios::binary | std::ios::in);
	if (in_stream.fail()) {
		perror(in_path);
		return;
	}

	std::ofstream out_stream(out_path, std::ios::binary | std::ios::out);
	if (out_stream.fail()) {
		perror(in_path);
		return;
	}

	size_t msg_size;
	if (!(in_stream >> msg_size)) {
		// No input so no output.
		return;
	}
	// Ignore the space.
	char ign;
	in_stream.read(&ign, 1);

	auto read_8bit_number = [&]() {
		unsigned char num;
		in_stream.read((char*)&num, 1);
		return (size_t) num;
	};

	size_t num_nodes = read_8bit_number();

	std::unordered_map<size_t, HNode*> nodes;
	for (size_t i = 0; i < num_nodes; i++) {
		HNode* node = create_node(0, nullptr, nullptr, DEFAULT_CHAR);
		nodes.insert({ node->id, node });
	}

	HNode* top = nullptr;
	for (size_t i = 0; i < num_nodes; i++) {
		size_t id = read_8bit_number();
		HNode* node = nodes[id];
		if (i == 0) {
			top = node;
		}

		in_stream.read(&node->ch, 1);

		size_t ln_id = read_8bit_number();
		size_t rn_id = read_8bit_number();
		
		if (ln_id != 0) {
			node->ln = nodes[ln_id];
			node->rn = nodes[rn_id];
		}
	}

	create_encodings(top);

	char cbyte = 0;
	std::string encoding;
	size_t num_chars = 0;
	while (in_stream.read(&cbyte, 1)) {
		size_t bidx = 0;
		
		while (bidx != 8) {
			encoding += (cbyte & (1 << bidx)) != 0 ? '1' : '0';
			++bidx;

			if (HNode* node = get_node(top, encoding)) {
				out_stream << node->ch;
				++num_chars;
				if (num_chars == msg_size) {
					// We are done.
					goto finished_decoding;
				}
				encoding = "";
			}
		}
	}

finished_decoding:
	// Cleaning up allocated nodes.
	for (auto [id, node] : nodes) {
		delete node;
	}
}

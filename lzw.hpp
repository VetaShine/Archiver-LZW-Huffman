#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <string>
#include <bitset>
#include "buffer.hpp"

struct TrieNode {
    std::unordered_map<uint8_t, TrieNode*> children;
    int index;

    TrieNode() {
        index = -1;
    }

    ~TrieNode() {
        for (auto& child : children) {
            delete child.second;
            child.second = nullptr;
        }
    }
};

class Trie {
    public:
        TrieNode* root;
        int next_index;
        uint32_t code_length;
        bool flag;

        Trie() {
            root = new TrieNode();
            next_index = 0;
            code_length = 9;
            flag = 0;
        }

        ~Trie() {
            delete root;
        }

        void deleteTrie(TrieNode* node) {
            if (node == nullptr) {
                return;
            }

            for (auto& child : node->children) {
                deleteTrie(child.second);
                child.second = nullptr;
            }

            delete node;
            node = nullptr;
        }

        void Insert(const std::string& line) {
            TrieNode* node = root;

            for (int index = 0; index < line.length(); index++) {
                if (node->children.find((uint8_t)line[index]) == node->children.end()) {
                    node->children[(uint8_t)line[index]] = new TrieNode();
                    
                    if (next_index == pow(2, code_length)) {
                        code_length++;
                    }
                }

                node = node->children[(uint8_t)line[index]];
            }
            
            node->index = next_index++;
        }

        int GetIndex(const std::string& line) {
            TrieNode* node = root;

            for (int index = 0; index < line.length(); index++) {
                if (node->children.find((uint8_t)line[index]) == node->children.end()) {
                    return -1;
                }

                node = node->children[(uint8_t)line[index]];
            }
            
            return node->index;
        }

        std::string GetStringByIndex(int index) {
            return GetString(root, "", index);
        }

    private:
        std::string GetString(TrieNode* node, const std::string& current, int index_number) {
            if (node->index == index_number) {
                return current;
            }

            std::string result;

            for (const auto& value : node->children) {
                result = GetString(value.second, current + std::string(1, static_cast<char>(value.first)), index_number);

                if (result != "") {
                    return result;
                }
            }

            return "";
        }
};

void Compress(std::istream& input, std::ostream& output, uint32_t level) {
    Trie trie;
    std::string current, ending = "", concatenation;
    int index;
    OutputBuffer buffer(output);
    buffer.Add(level, 32);
    uint32_t stop = 0;
    uint8_t element, symbol = 0x0;
    input.peek();
    trie.Insert(ending);

    for (uint32_t counter = 0; counter <= 0xff; symbol++, counter++) {
        trie.Insert(std::string(1, static_cast<char>(symbol)));
    }

    while (!input.eof()) {
        input.read((char*)&element, 1);
        concatenation = current;
        concatenation += std::string(1, static_cast<char>(element));
        index = trie.GetIndex(concatenation);

        if (index != -1) {
            current = concatenation;
        } else {
            buffer.Add(trie.GetIndex(current), trie.code_length);
            trie.Insert(concatenation);
            current = std::string(1, static_cast<char>(element));
            
            if (trie.code_length > level) {
                trie.deleteTrie(trie.root);
                trie.root = new TrieNode();
                trie.next_index = 0;
                trie.code_length = 9;
                trie.Insert(ending);

                for (uint32_t index = 0; index <= 0xff; symbol++, index++) {
                    trie.Insert(std::string(1, static_cast<char>(symbol)));
                }
            }       
        }

        input.peek();
    }

    if (!current.empty()) {
        buffer.Add(trie.GetIndex(current), trie.code_length);
    }

    buffer.Add(stop, trie.code_length);
    buffer.Cleaning();
}

int Decompress(std::istream& input, std::ostream& output) {
    Trie trie;
    std::string current = "", symbol, concatenation, ending = "", check;
    InputBuffer buffer(input);
    int level = buffer.Get(32), code, index;
    uint32_t stop = 0;

    if (!(level == 12 || level == 14 || level == 16)) {
        return 1;
    }

    trie.Insert(ending);

    for (uint32_t symbol = 0; symbol <= 0xff; symbol++) {
        trie.Insert(std::string(1, static_cast<char>(symbol)));
    }

    while (!buffer.Empty()) {
        if (trie.next_index == pow(2, trie.code_length)) {
            code = buffer.Get(trie.code_length + 1);
        } else {
            code = buffer.Get(trie.code_length);
        }

        if (code == stop) {
            break;
        }

        check = trie.GetStringByIndex(code);

        if (check != "") {
            symbol = check;
        } else {
            symbol = current + current[0];
        }

        concatenation = current + symbol[0];
        index = trie.GetIndex(concatenation);

        if (index != -1) {
            current = concatenation;
        } else {
            output.write(current.data(), current.length());
            trie.Insert(concatenation);
            current = symbol;

            if (trie.next_index + 1 > pow(2, level)) {
                trie.deleteTrie(trie.root);
                trie.root = new TrieNode();
                trie.next_index = 0;
                trie.code_length = 9;
                trie.Insert(ending);
                uint8_t insert_symbol = 0x0;

                for (uint32_t counter = 0; counter <= 0xff; insert_symbol++, counter++) {
                    trie.Insert(std::string(1, static_cast<char>(insert_symbol)));
                }

                output.write(current.data(), current.length());
                current = "";
            }    
        }
    }

    output.write(current.data(), current.length());

    return 0;
}

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <bitset>
#include <unordered_map>
#include "buffer.hpp"

class TreeNode {
    public:
        uint16_t symbol;
        uint32_t frequency;
        bool leaf;
        TreeNode* left;
        TreeNode* right;

        TreeNode() {
            symbol = 0;
            frequency = 0;
            leaf = 0;
            left = nullptr;
            right = nullptr;
        }

        TreeNode(uint16_t element) {
            symbol = element;
            frequency = 0;
            leaf = 1;
            left = nullptr;
            right = nullptr;
        }

        TreeNode(uint16_t element, uint32_t periodicity) {
            symbol = element;
            frequency = periodicity;
            leaf = 1;
            left = nullptr;
            right = nullptr;
        }

        TreeNode(TreeNode* left_node, TreeNode* right_node) {
            symbol = 0;
            frequency = left_node->frequency + right_node->frequency;
            leaf = 0;
            left = left_node;
            right = right_node;
        }

        virtual ~TreeNode() {
            if (!leaf) {
                delete left;
                delete right;
            }
        }
};

void DFS(const TreeNode* node, std::unordered_map<uint16_t, std::vector<bool>>& codes, std::vector<bool>& code) {
    if (node->leaf) {
        codes[node->symbol] = code;
        return;
    }

    code.push_back(0);
    DFS(node->left, codes, code);
    code.pop_back();
    code.push_back(1);
    DFS(node->right, codes, code);
    code.pop_back();
}

void GetCodes(TreeNode* node, std::unordered_map<uint16_t, std::vector<bool>>& codes) {
    std::vector<bool> code;
    DFS(node, codes, code);
}

void DFSSerialize(const TreeNode* node, OutputBuffer& buffer) {
    if (node->leaf) {
        buffer.Add((bool)1);
        buffer.Add(node->symbol);
        return;
    }

    buffer.Add((bool)0);
    DFSSerialize(node->left, buffer);
    DFSSerialize(node->right, buffer);
}

void SerializeTree(TreeNode* root, OutputBuffer& buffer) {
    if (root == nullptr) {
        return;
    }

    DFSSerialize(root, buffer);
}

TreeNode* DFSDeserialize(InputBuffer& buffer) {
    if (!buffer.Get(1)) {
        TreeNode* node = new TreeNode();
        node->left = DFSDeserialize(buffer);
        node->right = DFSDeserialize(buffer);
        return node;
    } else {
        return new TreeNode((uint16_t)buffer.Get(16));
    }
}

TreeNode* DeserializeTree(InputBuffer& buffer) {
    return DFSDeserialize(buffer);
}

int Encode(std::istream& input, std::ostream& output) {
    std::vector<uint32_t> frequencies(0xff + 1, 0);
    uint8_t byte;
    uint16_t symbol = 0, exit_code = 0xff + 1;

    while (!input.eof()) {
        input.read((char*)&byte, 1);
        frequencies[byte]++;
        input.peek();
    }

    std::priority_queue<TreeNode*, std::vector<TreeNode*>, std::function<bool(TreeNode*, TreeNode*)>> queue(
        [](TreeNode* first_node, TreeNode* second_node) { 
            return first_node->frequency > second_node->frequency; 
        }
    );

    for (uint32_t index = 0; index < frequencies.size(); index++, symbol++) {
        if (frequencies[index]) {
            queue.push(new TreeNode(symbol, frequencies[index]));
        }
    }

    queue.push(new TreeNode(exit_code, 0));
    frequencies.clear();
    TreeNode* left_node, *rigth_node;

    while (queue.size() > 1) {
        left_node = queue.top();
        queue.pop();
        rigth_node = queue.top();
        queue.pop();
        queue.push(new TreeNode(left_node, rigth_node));
    }

    TreeNode* root = queue.top();
    queue.pop();
    std::unordered_map<uint16_t, std::vector<bool>> code_table(0xff + 2);
    GetCodes(root, code_table);
    OutputBuffer buffer(output);
    SerializeTree(root, buffer);
    input.clear();
    input.seekg(0);

    while (!input.eof()) {
        input.read((char*)&byte, 1);
        buffer.Add(code_table[byte]);
        input.peek();
    }

    buffer.Add(code_table[exit_code]);
    buffer.Cleaning();
    code_table.clear();
}

void Decode(std::istream& input, std::ostream& output) {
    InputBuffer buffer(input);
    TreeNode* root = DeserializeTree(buffer);
    TreeNode* node = root;
    uint16_t exit_code = 0xff + 1;

    while (!buffer.Empty()) {
        if (buffer.Get(1)) {
            node = node->right;
        } else {
            node = node->left;
        }

        if (node->leaf) {
            if (node->symbol == exit_code) {
                return;
            }
            
            output.write((char*)&node->symbol, 1);
            node = root;
        }
    }
}

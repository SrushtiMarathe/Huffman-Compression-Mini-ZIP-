#include <bits/stdc++.h>
using namespace std;

/* ---------- Node Structure for Huffman Tree ---------- */
struct Node {
    unsigned char ch;
    int freq;
    Node *left, *right;
    Node(unsigned char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}
};

/* ---------- Min-Heap Comparator ---------- */
struct cmp {
    bool operator()(Node* a, Node* b) { return a->freq > b->freq; }
};

/* ---------- Generate Huffman Codes ---------- */
void buildCodes(Node* root, string code, unordered_map<unsigned char,string>& table) {
    if (!root) return;
    if (!root->left && !root->right)  // leaf
        table[root->ch] = code;
    buildCodes(root->left,  code + "0", table);
    buildCodes(root->right, code + "1", table);
}

/* ---------- Free Huffman Tree ---------- */
void freeTree(Node* root){
    if(!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

/* ---------- Compress Function ---------- */
void compress(const string &inFile, const string &outFile) {
    ifstream in(inFile, ios::binary);
    if (!in) { cout << "Input file error.\n"; return; }

    vector<unsigned char> data((istreambuf_iterator<char>(in)), {});
    in.close();

    unordered_map<unsigned char,int> freq;
    for (unsigned char c : data) freq[c]++;

    // Build Huffman Tree
    priority_queue<Node*, vector<Node*>, cmp> pq;
    for (auto &p : freq) pq.push(new Node(p.first, p.second));
    if (pq.size() == 1) pq.push(new Node('\0', 1));

    while (pq.size() > 1) {
        Node *a = pq.top(); pq.pop();
        Node *b = pq.top(); pq.pop();
        pq.push(new Node(a, b));
    }
    Node* root = pq.top();

    unordered_map<unsigned char,string> table;
    buildCodes(root, "", table);

    // Convert to bit string
    string bitstream = "";
    for (unsigned char c : data) bitstream += table[c];

    // Pack bits into bytes
    vector<unsigned char> output;
    unsigned char byte = 0;
    int count = 0;
    for(char bit : bitstream){
        byte = (byte << 1) | (bit - '0');
        count++;
        if (count == 8) {
            output.push_back(byte);
            count = 0;
            byte = 0;
        }
    }
    if (count > 0) {
        byte <<= (8 - count);
        output.push_back(byte);
    }

    // Write compressed file
    ofstream out(outFile, ios::binary);

    // Header: frequency table
    uint32_t size = freq.size();
    out.write((char*)&size, sizeof(size));
    for (auto &p : freq) {
        out.put(p.first);
        out.write((char*)&p.second, sizeof(int));
    }

    // Bit padding info
    unsigned char padding = (8 - (bitstream.size() % 8)) % 8;
    out.put(padding);

    // Write compressed data
    for (unsigned char b : output) out.put(b);

    out.close();
    freeTree(root);

    cout << "Compression complete.\n";
}

/* ---------- Decompress Function ---------- */
void decompress(const string &inFile, const string &outFile) {
    ifstream in(inFile, ios::binary);
    if (!in) { cout << "Input file error.\n"; return; }

    // Read frequency table
    uint32_t size;
    in.read((char*)&size, sizeof(size));
    unordered_map<unsigned char,int> freq;

    for (uint32_t i=0;i<size;i++){
        unsigned char ch = in.get();
        int f;
        in.read((char*)&f, sizeof(int));
        freq[ch] = f;
    }

    // Read padding
    unsigned char padding = in.get();

    // Build Huffman Tree
    priority_queue<Node*, vector<Node*>, cmp> pq;
    for (auto &p : freq) pq.push(new Node(p.first, p.second));
    if (pq.size() == 1) pq.push(new Node('\0', 1));

    while (pq.size() > 1) {
        Node *a = pq.top(); pq.pop();
        Node *b = pq.top(); pq.pop();
        pq.push(new Node(a, b));
    }
    Node* root = pq.top();

    // Read compressed bytes
    vector<unsigned char> bytes((istreambuf_iterator<char>(in)), {});
    in.close();

    // Convert bytes to bit stream
    string bits = "";
    for (size_t i = 0; i < bytes.size(); i++) {
        unsigned char b = bytes[i];
        for (int j = 7; j >= 0; j--)
            bits += ((b >> j) & 1) ? '1' : '0';
    }

    // Remove padding
    if (padding > 0)
        bits.erase(bits.end() - padding, bits.end());

    // Decode
    ofstream out(outFile, ios::binary);
    Node* cur = root;

    for (char bit : bits) {
        cur = (bit=='0') ? cur->left : cur->right;
        if (!cur->left && !cur->right) {
            out.put(cur->ch);
            cur = root;
        }
    }

    out.close();
    freeTree(root);
    cout << "Decompression complete.\n";
}

/* ---------- Main Menu ---------- */
int main() {
    cout << "Mini ZIP (Huffman Compression)\n";
    cout << "1. Compress\n2. Decompress\nChoice: ";

    int c;
    cin >> c;

    string inFile, outFile;
    cout << "Input file: ";
    cin >> inFile;
    cout << "Output file: ";
    cin >> outFile;

    if (c == 1) compress(inFile, outFile);
    else decompress(inFile, outFile);

    return 0;
}

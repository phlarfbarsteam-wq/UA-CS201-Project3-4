#pragma once
#include <string>
#include <vector>
#include <memory>

template <typename V>
class BTree {
private:
    struct Node {
        std::vector<std::string> keys;
        std::vector<V> values;
        std::vector<std::unique_ptr<Node>> children;
        bool leaf;

        Node(bool leaf) : leaf(leaf) {};
    };

    int t;
    std::unique_ptr<Node> root;

    void splitChild(Node* parent, int i) {
        //implementation
    }
    void insertNonFull(Node* n, std::string key, V value) {
        //implementation
    }

public:
    BTree(int t) : root(std::make_unique<Node>(true)), t(t < 2 ? 2 : t) {}
    void insert(std::string key, V value) {
        //implementation
    }
    V search(std::string key) {
        //implementation
    }

};

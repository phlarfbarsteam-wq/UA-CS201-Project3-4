#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Test-only back door (defined in the test file). It lets the tests corrupt a
// tree on purpose to prove validate() really detects broken trees.
struct BTreeTestAccess;

template <typename V, typename Key = std::string,
          typename Compare = std::less<Key>>
class BTree {
    friend struct BTreeTestAccess;

private:
    // One node of the tree. keys[i] is paired with values[i]. A leaf has no
    // children; an internal node has keys.size() + 1 children.
    struct Node {
        std::vector<Key> keys;
        std::vector<V> values;
        std::vector<std::unique_ptr<Node>> children;
        bool leaf;

        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    int t;                       // minimum degree (always >= 2)
    Compare comp;                // key ordering
    std::unique_ptr<Node> root;  // null only in a moved-from tree

    /*
     * Convert an index to an iterator offset (avoids signed/unsigned warnings).
     * Input:  a vector index. Output: the same value as a std::ptrdiff_t.
     */
    static std::ptrdiff_t offset(std::size_t index) {
        return static_cast<std::ptrdiff_t>(index);
    }

    /*
     * Largest number of keys a node may hold (2t - 1).
     * Output: that count.
     */
    std::size_t maxKeys() const {
        return static_cast<std::size_t>(2 * t - 1);
    }

    /*
     * Smallest number of keys a non-root node may hold (t - 1).
     * Output: that count.
     */
    std::size_t minKeys() const {
        return static_cast<std::size_t>(t - 1);
    }

    /*
     * Binary-search one node for a key.
     * Input:  node to search, key to look for, and a bool to fill in.
     * Output: the index of the first key that is not less than `key`.
     *         `found` is set true when that key equals `key`. When not found,
     *         the returned index is where the key belongs, and (for internal
     *         nodes) which child to descend into.
     */
    std::size_t locate(const Node& node, const Key& key, bool& found) const {
        auto it = std::lower_bound(node.keys.begin(), node.keys.end(), key,
                                   comp);
        found = (it != node.keys.end()) && !comp(key, *it);
        return static_cast<std::size_t>(it - node.keys.begin());
    }

    /*
     * Split the full child at parent->children[i] into two nodes and move its
     * median key/value up into the parent.
     *
     * Input:  parent (must NOT be full) and i, the index of a FULL child.
     * Output: none; the tree is modified in place.
     *
     */
    
    void splitChild(Node* parent, std::size_t i) {
        Node* child = parent->children[i].get();
        auto sibling = std::make_unique<Node>(child->leaf);
        const std::size_t medianIndex = static_cast<std::size_t>(t - 1);

        // Take the median out of the child; it will go up into the parent.
        Key medianKey = std::move(child->keys[medianIndex]);
        V medianValue = std::move(child->values[medianIndex]);

        // Move everything above the median into the new sibling.
        sibling->keys.insert(
            sibling->keys.end(),
            std::make_move_iterator(child->keys.begin() + t),
            std::make_move_iterator(child->keys.end()));
        sibling->values.insert(
            sibling->values.end(),
            std::make_move_iterator(child->values.begin() + t),
            std::make_move_iterator(child->values.end()));

        // Chop the child down to the lower half (this also drops the median's
        // moved-from slot).
        child->keys.erase(child->keys.begin() + (t - 1), child->keys.end());
        child->values.erase(child->values.begin() + (t - 1),
                            child->values.end());

        if (!child->leaf) {
            // An internal node also hands its upper t child pointers over.
            sibling->children.insert(
                sibling->children.end(),
                std::make_move_iterator(child->children.begin() + t),
                std::make_move_iterator(child->children.end()));
            child->children.erase(child->children.begin() + t,
                                  child->children.end());
        }

        // Insert the median into the parent at position i, and hook the new
        // sibling in just to the right of the original child.
        parent->keys.insert(parent->keys.begin() + offset(i), std::move(medianKey));
        parent->values.insert(parent->values.begin() + offset(i),
                              std::move(medianValue));
        parent->children.insert(parent->children.begin() + offset(i) + 1,
                                std::move(sibling));
    }

    /*
     * Insert a key/value pair into the subtree rooted at `n`.
     * Input:  n, a node that is NOT full; the key and value to store.
     * Output: true if the key was new, false if it replaced an existing value.
     *
     * Any full child on the path is split BEFORE we step into it, which is
     * what guarantees the node we recurse into always has room.
     */
    bool insertNonFull(Node* n, Key key, V value) {
        bool found = false;
        std::size_t i = locate(*n, key, found);

        // Key already present in this node: overwrite its value.
        if (found) {
            n->values[i] = std::move(value);
            return false;
        }

        // Leaf: the key belongs at position i. There is room, so just insert.
        if (n->leaf) {
            n->keys.insert(n->keys.begin() + offset(i), std::move(key));
            n->values.insert(n->values.begin() + offset(i), std::move(value));
            return true;
        }

        // Internal node: descend into child i, splitting it first if full.
        if (n->children[i]->keys.size() == maxKeys()) {
            splitChild(n, i);

            // The split moved a median into n->keys[i]. That median might be
            // the very key we are inserting (a duplicate)...
            if (!comp(key, n->keys[i]) && !comp(n->keys[i], key)) {
                n->values[i] = std::move(value);
                return false;
            }
            // ...otherwise decide which half of the split to continue into.
            if (comp(n->keys[i], key)) {
                ++i;
            }
        }

        return insertNonFull(n->children[i].get(), std::move(key),
                             std::move(value));
    }

    void removeFromLeaf(Node* node, std::size_t index) {
        node->keys.erase(node->keys.begin() + offset(index));
        node->values.erase(node->values.begin() + offset(index));
    }

    void borrowFromPrevious(Node* parent, std::size_t index) {
        Node* child = parent->children[index].get();
        Node* sibling = parent->children[index - 1].get();

        // Rotate the separator down and the left sibling's largest key up.
        child->keys.insert(child->keys.begin(),
                           std::move(parent->keys[index - 1]));
        child->values.insert(child->values.begin(),
                             std::move(parent->values[index - 1]));
        if (!child->leaf) {
            child->children.insert(child->children.begin(),
                                    std::move(sibling->children.back()));
            sibling->children.pop_back();
        }

        parent->keys[index - 1] = std::move(sibling->keys.back());
        parent->values[index - 1] = std::move(sibling->values.back());
        sibling->keys.pop_back();
        sibling->values.pop_back();
    }

    void borrowFromNext(Node* parent, std::size_t index) {
        Node* child = parent->children[index].get();
        Node* sibling = parent->children[index + 1].get();

        // Rotate the separator down and the right sibling's smallest key up.
        child->keys.push_back(std::move(parent->keys[index]));
        child->values.push_back(std::move(parent->values[index]));
        if (!child->leaf) {
            child->children.push_back(std::move(sibling->children.front()));
            sibling->children.erase(sibling->children.begin());
        }

        parent->keys[index] = std::move(sibling->keys.front());
        parent->values[index] = std::move(sibling->values.front());
        sibling->keys.erase(sibling->keys.begin());
        sibling->values.erase(sibling->values.begin());
    }

    void mergeChildren(Node* parent, std::size_t index) {
        Node* left = parent->children[index].get();
        Node* right = parent->children[index + 1].get();

        // Join both children around their separator when neither can lend.
        left->keys.push_back(std::move(parent->keys[index]));
        left->values.push_back(std::move(parent->values[index]));
        left->keys.insert(left->keys.end(),
                          std::make_move_iterator(right->keys.begin()),
                          std::make_move_iterator(right->keys.end()));
        left->values.insert(left->values.end(),
                            std::make_move_iterator(right->values.begin()),
                            std::make_move_iterator(right->values.end()));
        if (!left->leaf) {
            left->children.insert(
                left->children.end(),
                std::make_move_iterator(right->children.begin()),
                std::make_move_iterator(right->children.end()));
        }

        parent->keys.erase(parent->keys.begin() + offset(index));
        parent->values.erase(parent->values.begin() + offset(index));
        parent->children.erase(parent->children.begin() + offset(index + 1));
    }

    std::size_t fillChild(Node* parent, std::size_t index) {
        // Keep the descent target above minimum occupancy before recursing.
        if (index > 0 &&
            parent->children[index - 1]->keys.size() >=
                static_cast<std::size_t>(t)) {
            borrowFromPrevious(parent, index);
        } else if (index < parent->keys.size() &&
                   parent->children[index + 1]->keys.size() >=
                       static_cast<std::size_t>(t)) {
            borrowFromNext(parent, index);
        } else if (index < parent->keys.size()) {
            mergeChildren(parent, index);
        } else {
            mergeChildren(parent, index - 1);
            --index;
        }
        return index;
    }

    Node* predecessor(Node* node) const {
        while (!node->leaf) {
            node = node->children.back().get();
        }
        return node;
    }

    Node* successor(Node* node) const {
        while (!node->leaf) {
            node = node->children.front().get();
        }
        return node;
    }

    bool removeFromInternal(Node* node, std::size_t index) {
        Node* left = node->children[index].get();
        Node* right = node->children[index + 1].get();

        // Replace the key with a neighbor if its child can spare one.
        if (left->keys.size() >= static_cast<std::size_t>(t)) {
            Node* leaf = predecessor(left);
            Key replacementKey = leaf->keys.back();
            node->keys[index] = replacementKey;
            node->values[index] = std::move(leaf->values.back());
            return removeFromNode(left, replacementKey);
        }
        if (right->keys.size() >= static_cast<std::size_t>(t)) {
            Node* leaf = successor(right);
            Key replacementKey = leaf->keys.front();
            node->keys[index] = replacementKey;
            node->values[index] = std::move(leaf->values.front());
            return removeFromNode(right, replacementKey);
        }

        Key mergedKey = node->keys[index];
        mergeChildren(node, index);
        return removeFromNode(left, mergedKey);
    }

    bool removeFromNode(Node* node, const Key& key) {
        bool found = false;
        std::size_t index = locate(*node, key, found);

        if (found) {
            if (node->leaf) {
                removeFromLeaf(node, index);
                return true;
            }
            return removeFromInternal(node, index);
        }
        if (node->leaf) {
            return false;
        }

        if (node->children[index]->keys.size() == minKeys()) {
            index = fillChild(node, index);
        }
        return removeFromNode(node->children[index].get(), key);
    }

    /*
     * Count every key in a subtree.
     * Input:  subtree root (may be null).
     * Output: number of keys in the subtree.
     */
    static std::size_t countKeys(const Node* node) {
        if (node == nullptr) {
            return 0;
        }

        std::size_t count = node->keys.size();
        for (const auto& child : node->children) {
            count += countKeys(child.get());
        }
        return count;
    }

    /*
     * Count the levels in a subtree.
     * Input:  subtree root (may be null).
     * Output: 0 for null, 1 for a lone leaf, otherwise 1 + deepest child.
     */
    static std::size_t countLevels(const Node* node) {
        if (node == nullptr) {
            return 0;
        }

        std::size_t deepestChild = 0;
        for (const auto& child : node->children) {
            deepestChild = std::max(deepestChild, countLevels(child.get()));
        }
        return deepestChild + 1;
    }

    /*
     * In-order traversal: left child, key, next child, key, ... last child.
     * Input:  subtree root (may be null) and a callable visitor(key, value).
     * Output: none; the visitor is called once per pair, in ascending order.
     */
    template <typename Visitor>
    static void traverseInOrder(const Node* node, Visitor& visitor) {
        if (node == nullptr) {
            return;
        }

        for (std::size_t i = 0; i < node->keys.size(); ++i) {
            if (!node->leaf) {
                traverseInOrder(node->children[i].get(), visitor);
            }
            visitor(node->keys[i], node->values[i]);
        }
        if (!node->leaf) {
            traverseInOrder(node->children.back().get(), visitor);
        }
    }

    /*
     * Recursively check B-Tree rules 1-5 (see the top of this file) for one
     * subtree.
     * Input:  node to check; whether it is the root; the open interval
     *         (lowerBound, upperBound) its keys must fall inside (null means
     *         unbounded); its depth; and the shared leaf-depth record.
     * Output: true if the whole subtree is valid.
     */
    bool validateNode(const Node* node, bool isRoot, const Key* lowerBound,
                      const Key* upperBound, std::size_t depth,
                      bool& leafDepthSet, std::size_t& leafDepth) const {
        if (node == nullptr) {
            return false;
        }

        // Rules 1 and 2: key counts. Keys and values must stay paired.
        const std::size_t keyCount = node->keys.size();
        if (keyCount != node->values.size() || keyCount > maxKeys()) {
            return false;
        }
        if (isRoot) {
            if (!node->leaf && keyCount == 0) {
                return false;  // an internal root must hold at least one key
            }
        } else if (keyCount < minKeys()) {
            return false;
        }

        // Rule 4: keys strictly ascending and inside the parent's interval.
        for (std::size_t i = 0; i < keyCount; ++i) {
            const Key& key = node->keys[i];
            if ((i > 0 && !comp(node->keys[i - 1], key)) ||
                (lowerBound != nullptr && !comp(*lowerBound, key)) ||
                (upperBound != nullptr && !comp(key, *upperBound))) {
                return false;
            }
        }

        // Rule 5: the first leaf found sets the depth every other leaf must hit.
        if (node->leaf) {
            if (!node->children.empty()) {
                return false;
            }
            if (!leafDepthSet) {
                leafDepth = depth;
                leafDepthSet = true;
                return true;
            }
            return leafDepth == depth;
        }

        // Rule 3: k keys means k+1 children. Check each child against the
        // keys on either side of it.
        if (node->children.size() != keyCount + 1) {
            return false;
        }
        for (std::size_t i = 0; i < node->children.size(); ++i) {
            const Key* childLower = (i == 0) ? lowerBound : &node->keys[i - 1];
            const Key* childUpper = (i == keyCount) ? upperBound : &node->keys[i];
            if (!validateNode(node->children[i].get(), false, childLower,
                              childUpper, depth + 1, leafDepthSet,
                              leafDepth)) {
                return false;
            }
        }
        return true;
    }

public:
    // Smallest legal minimum degree; smaller requests are raised to this.
    static constexpr int MIN_DEGREE = 2;

    /*
     * Create an empty tree.
     * Input:  degree, the minimum degree t. A node holds at most 2t-1 keys.
     *         Values below MIN_DEGREE (2) are raised to 2.
     * Output: an empty tree.
     */
    explicit BTree(int degree, Compare compare = Compare())
        : t(degree < MIN_DEGREE ? MIN_DEGREE : degree),
          comp(std::move(compare)),
          root(std::make_unique<Node>(true)) {}

    // The tree owns its nodes, so it can be moved but not copied. A moved-from
    // tree is empty and can be reused.
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;
    BTree(BTree&&) = default;
    BTree& operator=(BTree&&) = default;

    /*
     * Add a key/value pair.
     * Input:  the key and value to store.
     * Output: true if the key was new; false if it already existed, in which
     *         case its value is replaced. (The tree can still reshape itself
     *         on a duplicate, because splits happen on the way down before the
     *         duplicate is noticed. It stays valid and its size is unchanged.)
     */
    bool insert(Key key, V value) {
        // A moved-from tree has no root; give it a fresh empty one.
        if (!root) {
            root = std::make_unique<Node>(true);
        }

        // A full root is the one split that needs a new level: make a new
        // root above it, then split the old root as its only child.
        if (root->keys.size() == maxKeys()) {
            auto newRoot = std::make_unique<Node>(false);
            newRoot->children.push_back(std::move(root));
            splitChild(newRoot.get(), 0);
            root = std::move(newRoot);
        }

        return insertNonFull(root.get(), std::move(key), std::move(value));
    }

    /*
     * Remove a key and its value.
     * Input:  key to remove.
     * Output: true if the key was present, false otherwise.
     */
    bool erase(const Key& key) {
        if (!root || root->keys.empty()) {
            return false;
        }

        const bool removed = removeFromNode(root.get(), key);
        if (root->keys.empty() && !root->leaf) {
            // An empty internal root is replaced by its only remaining child.
            root = std::move(root->children.front());
        }
        return removed;
    }

    /*
     * Look up a key (const version: the value cannot be modified).
     * Input:  key to find.
     * Output: pointer to the stored value, or nullptr if the key is absent.
     *         The pointer is invalidated by the next insert() or erase().
     */
    const V* search(const Key& key) const {
        const Node* node = root.get();
        while (node != nullptr) {
            bool found = false;
            const std::size_t i = locate(*node, key, found);

            if (found) {
                return &node->values[i];
            }
            if (node->leaf) {
                return nullptr;  // fell off the bottom: not in the tree
            }
            node = node->children[i].get();
        }
        return nullptr;  // moved-from tree
    }

    /*
     * Look up a key (non-const version: the value can be modified in place).
     * Input:  key to find.
     * Output: pointer to the stored value, or nullptr if the key is absent.
     *         The pointer is invalidated by the next insert() or erase().
     */
    V* search(const Key& key) {
        // Safe: *this is non-const here, so the value really is modifiable.
        return const_cast<V*>(std::as_const(*this).search(key));
    }

    /*
     * Check whether a key is stored.
     * Input:  key to find.
     * Output: true if present.
     */
    bool contains(const Key& key) const {
        return search(key) != nullptr;
    }

    /*
     * Count the stored key/value pairs.
     * Output: number of pairs. Walks the whole tree, so it is O(n).
     */
    std::size_t size() const {
        return countKeys(root.get());
    }

    /*
     * Check for an empty tree.
     * Output: true if no pairs are stored.
     */
    bool empty() const {
        return !root || root->keys.empty();
    }

    /*
     * Report the tree's height.
     * Output: number of node levels. 0 for an empty (or moved-from) tree,
     *         1 when everything fits in the root.
     */
    std::size_t height() const {
        if (empty()) {
            return 0;
        }
        return countLevels(root.get());
    }

    /*
     * Report the minimum degree in use.
     * Output: t (after any raising to MIN_DEGREE).
     */
    int degree() const {
        return t;
    }

    /*
     * Visit every pair in ascending key order.
     * Input:  any callable taking (const Key&, const V&), such as a lambda.
     * Output: none. Do not insert into the tree from inside the visitor.
     *
     * Example:  tree.forEach([](const std::string& k, const Patient& p) { });
     */
    template <typename Visitor>
    void forEach(Visitor&& visitor) const {
        traverseInOrder(root.get(), visitor);
    }

    /*
     * Self-check used by the tests: verify all B-Tree rules listed at the top
     * of this file.
     * Output: true if the tree is a valid B-Tree. Intended for debugging.
     */
    bool validate() const {
        if (!root) {
            return true;  // moved-from trees count as empty
        }

        bool leafDepthSet = false;
        std::size_t leafDepth = 0;
        return validateNode(root.get(), true, nullptr, nullptr, 0,
                            leafDepthSet, leafDepth);
    }
};
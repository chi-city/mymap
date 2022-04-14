// -----------------------------------------------------------------------

// Project 5 - mymap, mymap.h
//
// mymap.h implements a self balancing threaded
// BST.
//
// Author: Zaid Awaidah
// Date: 3 / 8 / 2022
// Class: CS 251, Spring 2022, UIC

// -----------------------------------------------------------------------

#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>
#include <sstream>
using namespace std;

// -----------------------------------------------------------------------

template<typename keyType, typename valueType>
class mymap {
 private:
    struct NODE {
        keyType key;  // used to build BST
        valueType value;  // stored data for the map
        NODE* left;  // links to left child
        NODE* right;  // links to right child
        int nL;  // number of nodes in left subtree
        int nR;  // number of nodes in right subtree
        bool isThreaded;
    };
    NODE* root;  // pointer to root node of the BST
    int size;  // # of key/value pairs in the mymap

    // ----------------------

    /* iterator:
     * This iterator is used so that mymap will work with a foreach loop.
    */
    struct iterator {
     private:
        NODE* curr;  // points to current in-order node for begin/end

     public:
        iterator(NODE* node) {
            curr = node;
        }

        // ----------------------

        keyType operator *() {
            return curr -> key;
        }

        // ----------------------

        bool operator ==(const iterator& rhs) {
            return curr == rhs.curr;
        }

        // ----------------------

        bool operator !=(const iterator& rhs) {
            return curr != rhs.curr;
        }

        // ----------------------

        bool isDefault() {
            return !curr;
        }

        // ----------------------

        /* operator++:
         * This function should advance curr to the next in-order node.
         * O(logN)
        */
        iterator operator++() {
            // if right node is root
            if (curr->isThreaded == true) {
                this->curr = this->curr->right;

            } else {
                if (curr->right != nullptr)
                    this->curr = this->curr->right;

                if (this->curr->left != nullptr) {
                    while (this->curr->left != nullptr) {
                        this->curr = this->curr->left;
                    }
                }
            }
            return iterator(curr);
        }
    };

    // ----------------------

    /* searchForKeyandMovePtrs
     * Helper function for put()
     * and operator[]
    */
    void searchForKeyandMovePtrs(NODE* &prev, NODE* &curr,
        keyType key) {
        while (curr != nullptr) {
            if (key == curr->key)
                return;

            if (key < curr->key) {  // search left
                prev = curr;
                curr = curr->left;
            } else {
                prev = curr;
                curr = (curr->isThreaded) ? nullptr : curr->right;
            }
        }
    }

    // ----------------------

    /* updateHeight
     * Iteratively updates number of
     * children for each curr node
    */
    void updateHeight(NODE* prev, NODE* curr, keyType key) {
        while (prev != curr) {
            if (prev->key < key) {
                prev->nR++;
                prev = prev->right;
            } else {
                prev->nL++;
                prev = prev->left;
            }
        }
    }

    // ----------------------

    /* insertNode
     * inserts the curr node in right position
     * helper function for put()
    */
    void insertNode(NODE* &prev, NODE* &curr, NODE* &newChild,
        keyType key, valueType value) {
        curr = newChild;
        if (prev == nullptr) {
            this->root = newChild;
            this->root->nL = 0;
            this->root->nR = 0;
            this->root->right = nullptr;
            this->root->left = nullptr;
            this->root->isThreaded = true;

        } else if (key < prev->key) {
            NODE* temp = this->root;
            prev->left = curr;
            curr->right = prev;

            // update height
            updateHeight(temp, curr, key);

            curr->nL = 0;
            curr->nR = 0;
            curr->isThreaded = true;

        } else if (key > prev->key) {
            NODE* temp = this->root;
            curr->right = prev->right;
            prev->right = curr;

            updateHeight(temp, curr, key);

            curr->nL = 0;
            curr->nR = 0;
            prev->isThreaded = false;
            curr->isThreaded = true;
        }
    }

    // ----------------------

    /* checkViolater
     * checks if node violates
     * seesaw balancing property
    */
    bool checkViolater(NODE* curr) {
        int maxNodes = max(curr->nL, curr->nR);
        int minNodes = 2 * min(curr->nL, curr->nR) + 1;

        if (maxNodes > minNodes)
            return true;
        else
            return false;
    }

    // ----------------------

    /* searchForViolaters
     * searches for violating node
     * traversing insertion path
    */
    NODE* searchForViolaters(NODE*& curr, NODE*& violater,
        NODE*& violaterParent) {
        NODE* start = this->root;

        if (start == curr)  // root added, no violations
            return nullptr;

        // check root
        bool temp = checkViolater(this->root);
        if (temp == true) {
            violaterParent = nullptr;
            violater = start;
            return violater;
        }

        // traverse if root is not violater
        while (start != curr) {
            if (curr->key < start->key) {
                // check first left subchild
                temp = checkViolater(start->left);
                if (temp == true) {
                    violater = start->left;
                    violaterParent = start;
                    return violater;
                }
                start = start->left;  // move left

            } else if (curr->key > start->key) {
                temp = checkViolater(start->right);
                if (temp == true) {
                    violater = start->right;
                    violaterParent = start;
                    return violater;
                }
                start = start->right;  // move right
            }
        }
        return violater;
    }

    // ----------------------

    /* _fillVector
     * recursively fills the vector
     * with nodes in order
     * Helper function for violaterExists
    */
    void _fillVector(NODE* curr, vector<NODE*> &imbalancedNodes) {
        if (curr == nullptr)
            return;

        _fillVector(curr->left, imbalancedNodes);

        imbalancedNodes.push_back(curr);
        if (curr->isThreaded == false)
            _fillVector(curr->right, imbalancedNodes);
        else
            return;
    }

    // ----------------------

    /* threadPtrs
     * threads the ptrs based of their
     * parents' key
     * Helper function for _balanceNodes
    */
    void threadPtrs(NODE*& prev, NODE*& curr) {
        if (prev->key < curr->key) {
            prev->right = curr;
            prev->isThreaded = true;
        } else if (prev->key > curr->key) {
            prev->right = curr->right;
            prev->isThreaded = true;
            curr->isThreaded = false;
        } else if (prev->key == curr->key) {
            prev->isThreaded = false;
            return;
        }
    }

    // ----------------------

    /* _balanceNodes
     * recursive helper function for violaterExists
     * creates a new subtree with nodes in vector
    */
    NODE* _balanceNodes(vector<NODE*> imbalancedSubTree,
        int start, int end, NODE* parent) {
        int middle = (start + end) / 2;

        // base case 1
        if (imbalancedSubTree.size() == 0) {
            return nullptr;
        } else if (imbalancedSubTree.size() == 1) {       // base case 2
            NODE* subRoot = imbalancedSubTree.at(middle);
            subRoot->nL = 0;
            subRoot->nR = 0;
            subRoot->left = nullptr;
            subRoot->right = parent->right;
            subRoot->isThreaded = true;
            return subRoot;
        }

        // create subroot with default values
        NODE* subRoot = imbalancedSubTree.at(middle);
        subRoot->nL = 0;
        subRoot->nR = 0;
        subRoot->left = nullptr;
        subRoot->right = nullptr;

        // connect left nodes
        if (middle - 1 >= start) {
            subRoot->left = _balanceNodes(imbalancedSubTree,
                start, middle - 1, subRoot);
            subRoot->nL = subRoot->left->nL + subRoot->left->nR + 1;
        }

        threadPtrs(subRoot, parent);

        // connect right nodes
        if (middle + 1 <= end) {
            subRoot->right = _balanceNodes(imbalancedSubTree,
                middle + 1, end, subRoot);
            subRoot->nR = subRoot->right->nL + subRoot->right->nR + 1;
        }

        return subRoot;
    }

    // ----------------------

    /* violaterExists
     * rebalances the subtree by
     * calling helper functions
    */
    NODE* violaterExists(NODE* violater, vector<NODE*> imbalancedNodes) {
        NODE* subTreeRoot;

        _fillVector(violater, imbalancedNodes);
        int left = 0;
        int right = imbalancedNodes.size() - 1;
        int midIndex = (left + right) / 2;

        // insert nodes - create subtree, balance and rethread
        subTreeRoot = _balanceNodes(imbalancedNodes, left, right
            , imbalancedNodes.at(midIndex));

        return subTreeRoot;
    }

    // ----------------------

  /* _BSTPrintInorder
   * recursive helper function for toString()
  */
    void _BSTPrintInorder(NODE* node, stringstream& temp) {
        if (node == nullptr)
            return;

        _BSTPrintInorder(node->left, temp);
        temp << "key: " << node->key << " "
            << "value: " << node->value << endl;

        if (node->isThreaded == false)
            _BSTPrintInorder(node->right, temp);
        else
            return;
    }

    // ----------------------

    /* _BSTPrintBalance
     * recursive helper function for checkBalance
    */
    void _BSTPrintBalance(NODE* node, stringstream& temp) {
        if (node == nullptr)
            return;

        temp << "key: " << node->key << ", "
            << "nL: " << node->nL << ", "
            << "nR: " << node->nR << endl;

        _BSTPrintBalance(node->left, temp);

        if (node->isThreaded == false)
            _BSTPrintBalance(node->right, temp);
        else
            return;
    }

    // ----------------------

    /* _toVectorPrint
     * recursive helper function for toVector
     * references back a vector of nodes
    */
    void _toVectorPrint(NODE* node,
        vector<pair<keyType, valueType>> &temp) {
        if (node == nullptr)
            return;

        _toVectorPrint(node->left, temp);

        temp.push_back(make_pair(node->key, node->value));

        if (node->isThreaded == false)
            _toVectorPrint(node->right, temp);
        else
            return;
    }

    // ----------------------

    /* _clearNodes
     * recursive helper function for clear 
    */
    void _clearNode(NODE* curr) {
        if (curr == nullptr)
            return;

        _clearNode(curr->left);
        if (curr->isThreaded == false)
            _clearNode(curr->right);

        this->size--;
        delete curr;
    }

    // ----------------------

    /* _copyNodes
     * recursive helper function
     * for copy constructor and
     * operator=
    */
    void _copyNodes(NODE* other) {
        if (other == nullptr)
            return;

        _copyNodes(other->left);
        if (other->isThreaded == false)
            _copyNodes(other->right);
        this->put(other->key, other->value);
    }

    // ----------------------
 public:
    /* default constructor :
     * Creates an empty mymap.
     * Time complexity: O(1)
    */
    mymap() {
        this->root = nullptr;
        this->size = 0;
    }

    // ----------------------

    /* copy constructor:
     * Constructs a new mymap which is a copy of the "other" mymap.
     * Sets all member variables appropriately.
     * Time complexity: O(nlogn), where n is total number of nodes in threaded,
     * self-balancing BST.
    */
    mymap(const mymap& other) {
        // copy root
        this->root = new NODE();
        this->root->key = other.root->key;
        this->root->value = other.root->value;
        this->size = 1;

        // copy nodes
        _copyNodes(other.root);
    }

    // ----------------------

    /* operator=:
     * Clears "this" mymap and then makes a copy of the "other" mymap.
     * Sets all member variables appropriately.
     * Time complexity: O(nlogn), where n is total number of nodes in threaded,
     * self-balancing BST.
    */
    mymap& operator=(const mymap& other) {
        // not a self assignment
        if (this == &other)
            return *this;

        // deallocate prev memory
        this->clear();

        // copy root first
        this->root = new NODE();
        this->root->key = other.root->key;
        this->root->value = other.root->value;
        this->size = 1;

        // copy nodes
        _copyNodes(other.root);

        return *this;
    }

    // ----------------------

    /* clear:
     * Frees the memory associated with the mymap; can be used for testing.
     * Time complexity: O(n), where n is total number of nodes in threaded,
     * self-balancing BST.
    */
    void clear() {
        _clearNode(this->root);
        this->root = nullptr;
    }

    // ----------------------

    /* destructor:
     * Frees the memory associated with the mymap.
     * Time complexity: O(n), where n is total number of nodes in threaded,
     * self-balancing BST.
    */
    ~mymap() { this->clear(); }

    // ----------------------

    /* put:
     * Inserts the key/value into the threaded, self-balancing BST based on
     * the key.
     * Time complexity: O(logn + mlogm), where n is total number of nodes in the
     * threaded, self-balancing BST and m is the number of nodes in the
     * sub-tree that needs to be re-balanced.
     * Space complexity: O(1)
    */
    void put(keyType key, valueType value) {
        vector<NODE*> imbalancedNodes;
        NODE* violaterParent = this->root;
        NODE* violater = nullptr;
        NODE* subTreeRoot = nullptr;
        NODE* prev = nullptr;
        NODE* curr = this->root;

        // check for key, return if found
        searchForKeyandMovePtrs(prev, curr, key);
        if (curr != nullptr && key == curr->key) {
            curr->value = value;
            return;
        }

        // create new node
        NODE* n = new NODE();
        n->key = key;
        n->value = value;
        n->isThreaded = false;

        // insert node in order
        insertNode(prev, curr, n, key, value);

        // check seesaw balanced property along insertion path
        violater = searchForViolaters(curr, violater, violaterParent);

        // balance here, pass violater and violaterParenr as arguments
        if (violater != nullptr)
            subTreeRoot = violaterExists(violater, imbalancedNodes);

        // updating original parent ptrs
        if (subTreeRoot != nullptr && violaterParent != nullptr) {
            if (violaterParent->key < subTreeRoot->key) {
                violaterParent->right = subTreeRoot;
            } else if (violaterParent->key > subTreeRoot->key) {
                violaterParent->left = subTreeRoot;
            }

        } else if (subTreeRoot != nullptr && violaterParent == nullptr) {
            this->root = subTreeRoot;
        }

        // increment size
        this->size++;
    }

    // ----------------------

    /*contains:
     * Returns true if the key is in mymap, return false if not.
     * Time complexity: O(logn), where n is total number of nodes in the
     * threaded, self-balancing BST
    */
    bool contains(keyType key) {
        NODE* curr = this->root;

        while (curr != nullptr) {
            if (key == curr->key)
                return true;
            else if (key < curr->key)
                curr = curr->left;
            else if (key > curr->key)
                curr = (curr->isThreaded) ? nullptr : curr->right;
        }
        return false;
    }

    // ----------------------

    /* get:
     * Returns the value for the given key; if the key is not found, the
     * default value, valueType(), is returned (but not added to mymap).
     * Time complexity: O(logn), where n is total number of nodes in the
     * threaded, self-balancing BST
    */
    valueType get(keyType key) {
        NODE* curr = this->root;

        while (curr != nullptr) {
            if (key == curr->key)
                return curr->value;
            else if (key < curr->key)
                curr = curr->left;
            else if (key > curr->key)
                curr = (curr->isThreaded) ? nullptr : curr->right;
        }

        return valueType();
    }

    // ----------------------

    /* operator[]:
     * Returns the value for the given key; if the key is not found,
     * the default value, valueType(), is returned (and the resulting new
     * key/value pair is inserted into the map).
     * Time complexity: O(logn + mlogm), where n is total number of nodes in the
     * threaded, self-balancing BST and m is the number of nodes in the
     * sub-trees that need to be re-balanced.
     * Space complexity: O(1)
    */
    valueType operator[](keyType key) {
        NODE* curr = this->root;
        NODE* prev = nullptr;

        searchForKeyandMovePtrs(prev, curr, key);
        if (curr != nullptr && key == curr->key)  // return value if found
            return curr->value;

        // create new node and insert in map
        this->put(key, valueType());

        return valueType();
    }

    // ----------------------

    /* Size:
     * Returns the # of key/value pairs in the mymap, 0 if empty.
     * O(1)
    */
    int Size() { return this->size; }

    // ----------------------

    /* begin:
     * returns an iterator to the first in order NODE.
     * Time complexity: O(logn), where n is total number of nodes in the
     * threaded, self-balancing BST
    */
    iterator begin() {
        NODE* curr = this->root;
        if (curr == nullptr)
            return iterator(nullptr);

        while (curr->left != nullptr)
            curr = curr->left;

        return iterator(curr);
    }

    // ----------------------

    /* end:
     * returns an iterator to the last in order NODE.
     * this function is given to you.
     * Time Complexity: O(1)
    */
    iterator end() { return iterator(nullptr); }

    // ----------------------

    /* toString:
     * Returns a string of the entire mymap, in order.
     * Format for 8/80, 15/150, 20/200:
     * "key: 8 value: 80\nkey: 15 value: 150\nkey: 20 value: 200\n
     * Time complexity: O(n), where n is total number of nodes in the
     * threaded, self-balancing BST
    */
    string toString() {
        NODE* curr = this->root;
        stringstream ss;
        _BSTPrintInorder(curr, ss);

        return ss.str();
    }

    // ----------------------

    /* toVector:
     * Returns a vector of the entire map, in order.  For 8/80, 15/150, 20/200:
     * {{8, 80}, {15, 150}, {20, 200}}
     * Time complexity: O(n), where n is total number of nodes in the
     * threaded, self-balancing BST
    */
    vector<pair<keyType, valueType>> toVector() {
        vector<pair<keyType, valueType>> mapVector;
        NODE* curr = this->root;

        _toVectorPrint(curr, mapVector);

        return mapVector;
    }

    // ----------------------

    /* checkBalance:
     * Returns a string of mymap that verifies that the tree is properly
     * balanced.  For example, if keys: 1, 2, 3 are inserted in that order,
     * function should return a string in this format (in pre-order):
     * "key: 2, nL: 1, nR: 1\nkey: 1, nL: 0, nR: 0\nkey: 3, nL: 0, nR: 0\n";
     * Time complexity: O(n), where n is total number of nodes in the
     * threaded, self-balancing BST
    */
    string checkBalance() {
        NODE* curr = this->root;
        stringstream ss;

        _BSTPrintBalance(curr, ss);
        return ss.str();
    }
};

// -----------------------------------------------------------------------

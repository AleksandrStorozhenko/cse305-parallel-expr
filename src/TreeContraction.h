#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include <vector>
#include <unordered_set>

// class TreeContraction
// {
    
// public:
//     static std::size_t contract(Node*& root)
//     {
//         if (!root) return 0;
//         std::size_t passes = 0;

//         std::unordered_set<Node*> alive;
//         collect(root, alive);

//         while (alive.size() > 1)
//         {
//             ++passes;

//             // rake
//             std::vector<Node*> dead;
//             for (Node* v : alive)
//                 if (v != root && v->isLeaf()) dead.push_back(v);

//             for (Node* v : dead)
//             {
//                 if (Node* p = v->parent()) p->removeChild(v);
//                 alive.erase(v);
//                 delete v;
//             }

//             // compress
//             std::vector<Node*> compress;
//             for (Node* v : alive)
//                 if (v != root && v->degree() == 1) compress.push_back(v);

//             for (Node* v : compress)
//             {
//                 Node* p = v->parent();
//                 Node* c = v->children().front();

//                 p->replaceChild(v, c);
//                 c->parent = p;  // friend access
//                 v->children().clear();
//                 alive.erase(v);
//                 delete v;
//             }

//             // promote child
//             if (root->degree() == 1)
//             {
//                 Node* newRoot = root->children().front();
//                 newRoot->parent = nullptr;
//                 root->children().clear();
//                 alive.erase(root);
//                 delete root;
//                 root = newRoot;
//                 alive.insert(root);
//             }
//         }
//         return passes;
//     }

// private:
//     static void collect(Node* v, std::unordered_set<Node*>& s)
//     {
//         s.insert(v);
//         for (Node* c : v->children()) collect(c, s);
//     }
// };

#endif

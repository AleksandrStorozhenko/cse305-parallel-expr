#ifndef BUILD_TREES_H
#define BUILD_TREES_H

#include <random>
#include <vector>
#include <cassert>

#include "Node.h"
#include "ValueNode.h"
#include "PlusNode.h"
#include "MultiplyNode.h"
#include "DivideNode.h"

namespace bench
{

    inline Node* makeOp(char op, Node* l, Node* r) noexcept
    {
        switch (op) {
            case '+': return new PlusNode(l, r);
            case '*': return new MultiplyNode(l, r);
            case '/': return new DivideNode(l, r);
        }
        assert(false && "unsupported op"); return nullptr;
    }

    inline char pickOp(std::mt19937& g) noexcept
    {
        static const char ops[3] = {'+', '*', '/'};
        return ops[ std::uniform_int_distribution<int>(0,2)(g) ];
    }

    inline double randLeaf(std::mt19937& g, double lo = 1.0, double hi = 10.0) noexcept
    {
        return std::uniform_real_distribution<double>(lo,hi)(g);
    }

    // basic tree gen
    inline Node* perfectBin(unsigned depth, std::mt19937& g, bool mixOps = false, char fixedOp = '+')
    {
        if (depth == 0) return new ValueNode(randLeaf(g));
        Node* l = perfectBin(depth-1, g, mixOps, fixedOp);
        Node* r = perfectBin(depth-1, g, mixOps, fixedOp);
        return makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    }

    // lr chains
    inline Node* leftChain(std::size_t leaves, std::mt19937& g, bool mixOps = false, char fixedOp = '+')
    {
        assert(leaves);
        Node* root = new ValueNode(randLeaf(g));
        for (std::size_t i = 1; i < leaves; ++i)
            root = makeOp(mixOps ? pickOp(g) : fixedOp, root, new ValueNode(randLeaf(g)));
        return root;
    }
    
    inline Node* rightChain(std::size_t leaves, std::mt19937& g, bool mixOps = false, char fixedOp = '+')
    {
        assert(leaves);
        Node* root = new ValueNode(randLeaf(g));
        for (std::size_t i = 1; i < leaves; ++i)
            root = makeOp(mixOps ? pickOp(g) : fixedOp,
                        new ValueNode(randLeaf(g)),
                        root);
        return root;
    }

    // random tests
    inline Node* randomTree(std::size_t internal, std::mt19937& g, bool mixOps = true, char fixedOp = '+')
    {
        std::vector<Node*> pool;
        pool.reserve(internal + 1);
        for (std::size_t i = 0; i < internal + 1; ++i)
            pool.push_back(new ValueNode(randLeaf(g)));

        for (std::size_t i = 0; i < internal; ++i) {
            std::uniform_int_distribution<std::size_t> pick(0, pool.size()-1);
            std::size_t a = pick(g), b = pick(g);
            while (b == a) b = pick(g);

            Node* parent = makeOp(mixOps ? pickOp(g) : fixedOp, pool[a], pool[b]);
            pool[a] = parent;
            pool[b] = pool.back();
            pool.pop_back();
        }
        assert(pool.size() == 1);
        return pool.front();
    }

    // Fibonacii Trees
    inline Node* fibTree(unsigned depth, std::mt19937& g, bool mixOps = false, char fixedOp = '+')
    {
        if (depth <= 1) return new ValueNode(randLeaf(g));
        Node* l = fibTree(depth-1, g, mixOps, fixedOp);
        Node* r = fibTree(depth-2, g, mixOps, fixedOp);
        return makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    }

    // More test tree generators
    inline Node* caterpillar(std::size_t spineLen, std::mt19937& g, bool mixOps = true, char fixedOp = '+')
    {
        assert(spineLen);
        Node* root = new ValueNode(randLeaf(g));
        for (std::size_t i = 1; i < spineLen; ++i) {
            Node* leaf  = new ValueNode(randLeaf(g));
            Node* spine = makeOp(mixOps ? pickOp(g) : fixedOp,
                                new ValueNode(randLeaf(g)), leaf);
            root = makeOp(mixOps ? pickOp(g) : fixedOp, root, spine);
        }
        return root;
    }

    inline Node* accordion(unsigned depth, std::mt19937& g, bool mixOps = true, char fixedOp = '+')
    {
        if (depth == 0) return new ValueNode(randLeaf(g));
        if (depth % 2 == 0)
            return perfectBin(1, g, mixOps, fixedOp);
        return makeOp(mixOps ? pickOp(g) : fixedOp,
                    accordion(depth-1, g, mixOps, fixedOp),
                    new ValueNode(randLeaf(g)));
    }

}
#endif

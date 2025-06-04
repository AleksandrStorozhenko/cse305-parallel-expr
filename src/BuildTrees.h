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

    // perfect binary tree
    inline Node* perfectBin(unsigned depth, std::mt19937& g, bool mixOps = false, char fixedOp = '+')
    {
        if (depth == 0) return new ValueNode(randLeaf(g));
        Node* l = perfectBin(depth-1, g, mixOps, fixedOp);
        Node* r = perfectBin(depth-1, g, mixOps, fixedOp);
        return makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    }

    // random binary tree
    inline Node* randomBalanced(unsigned depth, std::mt19937& g, bool mixOps = true, char fixedOp = '+')
    {
        if (depth == 0) {
            return new ValueNode(randLeaf(g));
        }

        unsigned leftMax  = depth - 1;
        unsigned leftUsed = std::uniform_int_distribution<unsigned>(0, leftMax)(g);
        unsigned rightUsed = depth - 1;

        Node* l = randomBalanced(leftUsed,  g, mixOps, fixedOp);
        Node* r = randomBalanced(rightUsed, g, mixOps, fixedOp);

        return makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    }

}
#endif

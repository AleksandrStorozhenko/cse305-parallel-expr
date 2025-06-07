#ifndef BUILD_TREES_H
#define BUILD_TREES_H

#include <random>
#include <cassert>
#include "ValueNode.h"
#include "PlusNode.h"
#include "MultiplyNode.h"
#include "DivideNode.h"

namespace bench {

inline Node::Ptr makeOp(char op, const Node::Ptr& l, const Node::Ptr& r)
{
    switch (op) {
        case '+': return std::make_shared<PlusNode>(l, r);
        case '*': return std::make_shared<MultiplyNode>(l, r);
        case '/': return std::make_shared<DivideNode>(l, r);
    }
    assert(false && "unsupported op");
    return nullptr;
}

inline char pickOp(std::mt19937& g)
{
    static const char ops[3] = {'+', '*', '/'};
    return ops[ std::uniform_int_distribution<int>(0,2)(g) ];
}

inline double randLeaf(std::mt19937& g, double lo = 1.0, double hi = 2.0)
{
    return std::uniform_real_distribution<double>(lo, hi)(g);
}

inline void link(Node::Ptr p, const Node::Ptr& c) { if (c) c->set_parent(p); }

inline Node::Ptr perfectBin(unsigned depth, std::mt19937& g,
                            bool mixOps = false, char fixedOp = '+')
{
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));
    auto l = perfectBin(depth - 1, g, mixOps, fixedOp);
    auto r = perfectBin(depth - 1, g, mixOps, fixedOp);
    auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(n, l); link(n, r);
    return n;
}

inline Node::Ptr randomBalanced(unsigned depth, std::mt19937& g,
                                bool mixOps = true, char fixedOp = '+')
{
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));

    unsigned leftMax = depth - 1;
    unsigned leftUsed = std::uniform_int_distribution<unsigned>(0, leftMax)(g);
    unsigned rightUsed = depth - 1 - leftUsed;

    auto l = randomBalanced(leftUsed, g, mixOps, fixedOp);
    auto r = randomBalanced(rightUsed, g, mixOps, fixedOp);

    auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(n, l); link(n, r);
    return n;
}

inline Node::Ptr longSkewed(unsigned depth, std::mt19937& g, bool mixOps = true, char fixedOp = '+', bool leftHeavy = true)
{
    if (depth == 0)
        return std::make_shared<ValueNode>(randLeaf(g));

    auto inner = longSkewed(depth - 1, g, mixOps, fixedOp, leftHeavy);
    auto leaf = std::make_shared<ValueNode>(randLeaf(g));

    Node::Ptr l = leftHeavy ? inner : leaf;
    Node::Ptr r = leftHeavy ? leaf : inner;

    auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(n, l); link(n, r);
    return n;
}

// sparsity-aware generator (acts as full benchmark generator)
inline Node::Ptr sparseBin(unsigned depth, double sparsity, std::mt19937& g,
                            bool mixOps = true, char fixedOp = '+')
{
    if (sparsity <= 0.0) return perfectBin(depth, g, mixOps, fixedOp); // fast perfect
    if (sparsity >= 1.0) {
        bool leftHeavy = std::uniform_int_distribution<int>(0, 1)(g);
        return longSkewed(depth, g, mixOps, fixedOp, leftHeavy); // chain path
    }
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));
    std::bernoulli_distribution recurse(1.0 - sparsity);
    bool leftRec = recurse(g);
    bool rightRec = recurse(g);
    if (!leftRec && !rightRec) {
        if (std::uniform_int_distribution<int>(0, 1)(g)) leftRec = true;
        else rightRec = true;
    }
    auto l = leftRec ? sparseBin(depth - 1, sparsity, g, mixOps, fixedOp)
                     : std::make_shared<ValueNode>(randLeaf(g));
    auto r = rightRec ? sparseBin(depth - 1, sparsity, g, mixOps, fixedOp)
                      : std::make_shared<ValueNode>(randLeaf(g));
    auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(n, l); link(n, r);
    return n;
}

}
#endif

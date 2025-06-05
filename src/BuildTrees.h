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

inline double randLeaf(std::mt19937& g, double lo = 1.0, double hi = 10.0)
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

} // namespace bench
#endif


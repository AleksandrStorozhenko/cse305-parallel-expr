#ifndef BUILD_TREES_H
#define BUILD_TREES_H

#include <random>
#include <cassert>
#include <functional>
#include <vector>
#include <cstdint>
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
    return ops[ std::uniform_int_distribution<int>(0, 2)(g) ];
}

inline double randLeaf(std::mt19937& g, double lo = 1.0, double hi = 2.0)
{
    return std::uniform_real_distribution<double>(lo, hi)(g);
}

inline void link(Node::Ptr p, const Node::Ptr& c) { if (c) c->set_parent(p); }

inline Node::Ptr perfectBin(unsigned depth, std::mt19937& g,
                            bool mixOps = true, char fixedOp = '+')
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
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));

    auto inner = longSkewed(depth - 1, g, mixOps, fixedOp, leftHeavy);
    auto leaf = std::make_shared<ValueNode>(randLeaf(g));

    Node::Ptr l = leftHeavy ? inner : leaf;
    Node::Ptr r = leftHeavy ? leaf : inner;

    auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(n, l); link(n, r);
    return n;
}

// speedup: we batch the bernoullis inside (8 per int)
class BatchBernoulli {
public:
    BatchBernoulli(std::mt19937& rng, double p)
        : g(rng),
          thresh(static_cast<uint8_t>(p * 255.0)),
          buf(0),
          bytesLeft(0) {}
    bool next() {
        if (bytesLeft == 0) {
            buf = (static_cast<uint64_t>(g()) << 32) | g();
            bytesLeft = 8;
        }
        uint8_t b = static_cast<uint8_t>(buf & 0xFF);
        buf >>= 8;
        --bytesLeft;
        return b < thresh;
    }
private:
    std::mt19937& g;
    uint8_t thresh;
    uint64_t buf;
    int bytesLeft;
};

// sparseBin: decide-first/allocate-later generator
inline Node::Ptr sparseBin(unsigned depth, double sparsity, std::mt19937& g,
                           bool mixOps = true, char fixedOp = '+')
{
    if (sparsity <= 0.0) return perfectBin(depth, g, mixOps, fixedOp); // perfect
    if (sparsity >= 1.0) {
        bool leftHeavy = std::uniform_int_distribution<int>(0, 1)(g);
        return longSkewed(depth, g, mixOps, fixedOp, leftHeavy);
    }
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));

    BatchBernoulli coin(g, 1.0 - sparsity);

    std::vector<uint8_t> masks;
    masks.reserve((1u << depth) - 1);

    struct Item { unsigned d; };
    std::vector<Item> st;
    st.push_back({depth});
    while (!st.empty()) {
        unsigned d = st.back().d;
        st.pop_back();
        if (d == 0) continue;
        bool L = coin.next();
        bool R = coin.next();
        if (!L && !R) { if (coin.next()) L = true; else R = true; }
        masks.push_back((L ? 1 : 0) | (R ? 2 : 0));
        if (R) st.push_back({d - 1});
        if (L) st.push_back({d - 1});
    }

    std::size_t idx = 0;
    std::function<Node::Ptr(unsigned)> build = [&](unsigned d) -> Node::Ptr {
        if (d == 0) return std::make_shared<ValueNode>(randLeaf(g));
        uint8_t m = masks[idx++];
        bool L = m & 1;
        bool R = m & 2;
        Node::Ptr l = L ? build(d - 1) : std::make_shared<ValueNode>(randLeaf(g));
        Node::Ptr r = R ? build(d - 1) : std::make_shared<ValueNode>(randLeaf(g));
        auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
        link(n, l); link(n, r);
        return n;
    };
    return build(depth);
}

inline Node::Ptr fibonacciTree(unsigned n, std::mt19937& g, bool mixOps = true, char fixedOp = '+')
{
    if (n == 0 || n == 1) {
        return std::make_shared<ValueNode>(randLeaf(g));
    }
    auto l = fibonacciTree(n - 1, g, mixOps, fixedOp);
    auto r = fibonacciTree(n - 2, g, mixOps, fixedOp);
    auto nPtr = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(nPtr, l); link(nPtr, r);
    return nPtr;
}

inline Node::Ptr alternatingLeftHeavy(unsigned depth, std::mt19937& g)
{
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));
    static const char ops[3] = {'+', '*', '/'};
    char op = ops[depth % 3];
    auto left = alternatingLeftHeavy(depth - 1, g);
    auto right = std::make_shared<ValueNode>(randLeaf(g));
    auto n = makeOp(op, left, right);
    link(n, left); link(n, right);
    return n;
}

inline Node::Ptr zigZagTree(unsigned depth, std::mt19937& g, bool left = true)
{
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));
    auto deep = zigZagTree(depth - 1, g, !left);
    auto leaf = std::make_shared<ValueNode>(randLeaf(g));
    Node::Ptr l = left ? deep : leaf;
    Node::Ptr r = left ? leaf : deep;
    auto n = makeOp(pickOp(g), l, r);
    link(n, l); link(n, r);
    return n;
}

inline Node::Ptr midDensityTree(unsigned depth, std::mt19937& g, bool mixOps = true, char fixedOp = '+')
{
    if (depth == 0) return std::make_shared<ValueNode>(randLeaf(g));
    bool skewLeft = std::bernoulli_distribution(0.6)(g);
    unsigned lDepth = skewLeft ? depth - 1 : std::uniform_int_distribution<unsigned>(0, depth - 1)(g);
    unsigned rDepth = skewLeft ? std::uniform_int_distribution<unsigned>(0, depth - 1)(g) : depth - 1;
    auto l = midDensityTree(lDepth, g, mixOps, fixedOp);
    auto r = midDensityTree(rDepth, g, mixOps, fixedOp);
    auto n = makeOp(mixOps ? pickOp(g) : fixedOp, l, r);
    link(n, l); link(n, r);
    return n;
}

Node::Ptr randomFixedSizeTree(unsigned depth, std::mt19937& rng) {
    unsigned N = 1<<depth;
    assert(N >= 1);
    std::vector<Node::Ptr> nodes;
    for (unsigned i = 0; i < N; ++i) {
        nodes.push_back(std::make_shared<ValueNode>(rng() % 10 + 1)); // random values 1-100
    }

    std::uniform_int_distribution<int> opDist(0, 2);
    std::vector<Node::Ptr> pool = nodes;
    // std::cout<<"Building random fixed tree"<<std::endl;
    while (pool.size() > 1) {
        std::shuffle(pool.begin(), pool.end(), rng);
        auto left = pool.back(); pool.pop_back();
        auto right = pool.back(); pool.pop_back();

        Node::Ptr parent;
        switch (opDist(rng)) {
            case 0: parent = std::make_shared<PlusNode>(left, right); break;
            case 1: parent = std::make_shared<MultiplyNode>(left, right); break;
            case 2: parent = std::make_shared<DivideNode>(left, right); break;
        }

        pool.push_back(parent);
    }
    return pool.front(); // root node
}

}
#endif

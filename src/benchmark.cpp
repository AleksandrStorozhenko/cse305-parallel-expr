#include <iostream>
#include <iomanip> // to compute and output speedup
#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <thread>

#include "Node.h"
#include "ValueNode.h"
#include "PlusNode.h"
#include "MultiplyNode.h"
#include "DivideNode.h"
#include "BuildTrees.h"
#include "TreeContraction.h"
#include "ThreadPool.h"

// analytic expectation of node count
static double expected_nodes(unsigned depth, double s) { // s is sparsity
    if (s <= 0.0) return std::pow(2.0, depth + 1) - 1.0;
    if (s >= 1.0) return 2.0 * depth + 1.0;
    const double recurse = 2.0 - 2.0 * s + s * s; // expected num.children that recurse
    const double leaves = 2.0 * s - s * s; // expected num. leaf children
    double E = 1.0; // depth=0
    for (unsigned d = 1; d <= depth; ++d)
        E = 1.0 + recurse * E + leaves;
    return E;
}

static std::size_t countNodes(const Node::Ptr& root) {
    if (!root) return 0;
    std::size_t n = 1;
    for (auto& c : root->children()) n += countNodes(c);
    return n;
}

static void collect_nodes(const Node::Ptr& n, std::vector<Node::Ptr>& v) {
    if (!n) return;
    v.push_back(n);
    for (auto& c : n->children()) collect_nodes(c, v);
}

static double runTreeContraction(const Node::Ptr& root,
                                 unsigned threads,
                                 SimplePool& pool,
                                 unsigned long seed = 0) {
    std::vector<Node::Ptr> nodes;
    collect_nodes(root, nodes);
    std::mt19937 rng(seed ? seed : 0x9e3779b97f4a7c15ULL);
    if (nodes.size() > 1)
        std::shuffle(nodes.begin() + 1, nodes.end(), rng);
    TreeContraction::TreeContract(nodes, root, threads, pool);
    return root->value ? *root->value : root->compute();
}

template<class F>
double time_us(F&& f) {
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::micro>(t1 - t0).count();
}

int main(int argc, char* argv[])
{
    const int REPS = (argc > 1) ? std::stoi(argv[1]) : 10;
    const double tolFactor = (argc > 2) ? std::stod(argv[2]) : 1e-6;
    const double tolExp = (argc > 3) ? std::stod(argv[3]) : 1.0001;

    std::vector<unsigned> testDepths{2, 6, 9, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};

    // sparsities - expected size grows roughly linearly
    std::vector<double> sparsities;
    for (int i = 0; i <= 10; ++i) {
        double branch = 2.0 - i * 0.1;
        double s = 1.0 - std::sqrt(branch - 1.0);
        sparsities.push_back(s);
    }

    const unsigned HW_THREADS =
        std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1;
    std::vector<unsigned> threadCounts;
    for (unsigned v = 2; v <= HW_THREADS * 2; v <<= 1) threadCounts.push_back(v);
    if (threadCounts.back() != HW_THREADS) threadCounts.push_back(HW_THREADS);

    std::mt19937 master(42);
    std::cout << "sparsity,depth,threads,expected_nodes,n_nodes,baseline_us,contraction_us,speedup\n";
    std::cout << std::fixed << std::setprecision(3);

    auto run_case = [&](double sparsity,
                        unsigned depth,
                        unsigned threads,
                        unsigned long seed,
                        SimplePool& pool)
    {
        bool mixOps = (depth <= 21); // overflow fix

        auto makeTree = [&](unsigned d, std::mt19937& rng, bool mix, char op) {
            return (sparsity == 0.0)
                   ? bench::perfectBin(d, rng, mix, op)
                   : bench::sparseBin(d, sparsity, rng, mix, op);
        };

        std::mt19937 tmpRng(seed + 999999);
        Node::Ptr sample = makeTree(depth, tmpRng, mixOps, '+');
        std::size_t n_nodes = countNodes(sample);
        double exp_nodes = expected_nodes(depth, sparsity);

        std::vector<double> baselineVals(REPS);
        double base_sum = 0.0;
        for (int i = 0; i < REPS; ++i) {
            std::mt19937 gi(seed + i);
            Node::Ptr tmp = makeTree(depth, gi, mixOps, '+');
            double val = 0.0;
            base_sum += time_us([&] { val = tmp->compute(); });
            baselineVals[i] = val;
        }
        double base_us = base_sum / REPS;

        constexpr double ABS_REL_SWITCH = 1.0;
        constexpr double ABS_EPS = 1e-12;

        double contr_sum = 0.0;
        for (int i = 0; i < REPS; ++i) {
            std::mt19937 gi(seed + i);
            Node::Ptr tmp = makeTree(depth, gi, mixOps, '+');
            double val = 0.0;
            contr_sum += time_us([&] { val = runTreeContraction(tmp, threads, pool); });
            double ref = baselineVals[i];
            double absTol = tolFactor * std::pow(static_cast<double>(n_nodes), tolExp);
            double relTol = tolFactor * std::fabs(ref);
            double tol = (std::fabs(ref) < ABS_REL_SWITCH) ? std::max(absTol, ABS_EPS) : relTol;
            assert(std::fabs(ref - val) <= tol);
        }
        double contr_us = contr_sum / REPS;
        double speedup = base_us / contr_us;

        std::cout << sparsity << ','
                  << depth << ','
                  << threads << ','
                  << exp_nodes << ','
                  << n_nodes << ','
                  << base_us << ','
                  << contr_us << ','
                  << speedup << '\n';
    };

    for (unsigned t : threadCounts) {
        SimplePool pool(t);
        for (unsigned d : testDepths) {
            unsigned long seed = master();
            for (double s : sparsities) run_case(s, d, t, seed, pool);
        }
        pool.stop();
    }
    return 0;
}

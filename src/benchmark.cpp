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

const int MAX_NODES = 4'000'000;

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
    // std::cout<<"threads = "<<thresads<<std::endl;
    TreeContraction::TreeContract(nodes, root, threads, pool);
    pool.waitIdle();
    return root->value ? *root->value : root->compute();
}

template<class F>
double time_us(F&& f) {
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::micro>(t1 - t0).count();
}

static inline bool isFinite(double x) { return std::isfinite(x); }

int main(int argc, char* argv[])
{
    const int REPS = (argc > 1) ? std::stoi(argv[1]) : 1;
    const double tolFactor = (argc > 2) ? std::stod(argv[2]) : 1e-6;
    const double tolExp = (argc > 3) ? std::stod(argv[3]) : 1.0001;

    const unsigned HW_THREADS =
        std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1;
    std::vector<unsigned> threadCounts;
    for (unsigned v = 2; v <= HW_THREADS * 2; v <<= 1) threadCounts.push_back(v);
    if (threadCounts.back() != HW_THREADS) threadCounts.push_back(HW_THREADS);

    std::mt19937 master(42);
    std::cout << "tree_type,depth,threads,n_nodes,baseline_us,contraction_us,speedup\n";
    std::cout << std::fixed << std::setprecision(3);

    using TreeGen = std::function<Node::Ptr(unsigned, std::mt19937&)>;

    std::vector<std::pair<std::string, TreeGen>> treeGenerators = {
        // {"perfectBin", [](unsigned d, std::mt19937& g) { return bench::perfectBin(d, g); }},
        // {"randomBalanced", [](unsigned d, std::mt19937& g) { return bench::randomBalanced(d, g); }},
        // {"longSkewed", [](unsigned d, std::mt19937& g) { return bench::longSkewed(d, g); }},
        // {"fibonacciTree", [](unsigned d, std::mt19937& g) { return bench::fibonacciTree(d, g); }},
        // {"alternatingLeftHeavy", [](unsigned d, std::mt19937& g) { return bench::alternatingLeftHeavy(d, g); }},
        // {"zigZagTree", [](unsigned d, std::mt19937& g) { return bench::zigZagTree(d, g); }},
        // {"midDensityTree", [](unsigned d, std::mt19937& g) { return bench::midDensityTree(d, g); }},
        {"randomFixedSize", [](unsigned d, std::mt19937& g) { return bench::randomFixedSizeTree(d, g); }}
        };

    for (unsigned t : threadCounts) {
        SimplePool pool(t);
        for (const auto& [name, makeTree] : treeGenerators) {
            for (unsigned d = 2; d <= 20; ++d) {
                unsigned long seed = master();
                std::mt19937 baseRng(seed);
                Node::Ptr sample = makeTree(d, baseRng);
                std::size_t n_nodes = countNodes(sample);
                if (n_nodes > MAX_NODES) continue;

                std::vector<double> baselineVals(REPS);
                double base_sum = 0.0;
                for (int i = 0; i < REPS; ++i) {
                    std::mt19937 gi(seed + i);
                    Node::Ptr tmp = makeTree(d, gi);
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
                    Node::Ptr tmp = makeTree(d, gi);
                    double val = 0.0;
                    contr_sum += time_us([&] { val = runTreeContraction(tmp, t, pool); });
                    double ref = baselineVals[i];
                    double absTol = tolFactor * std::pow(static_cast<double>(n_nodes), tolExp);
                    double relTol = tolFactor * std::fabs(ref);
                    double tol = (std::fabs(ref) < ABS_REL_SWITCH) ? std::max(absTol, ABS_EPS) : relTol;
                    if (isFinite(ref) && isFinite(val)) {
                        assert(std::fabs(ref - val) <= tol);
                    }
                }
                double contr_us = contr_sum / REPS;
                double speedup = base_us / contr_us;

                std::cout << name << ','
                          << d << ','
                          << t << ','
                          << n_nodes << ','
                          << base_us << ','
                          << contr_us << ','
                          << speedup <<
                          std::endl;
            }
        }
        pool.stop();
    }
    return 0;
}


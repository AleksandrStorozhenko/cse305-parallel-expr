#include <iostream>
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

static double runTreeContraction(const Node::Ptr& root, unsigned threads = 1) {
    std::vector<Node::Ptr> nodes;
    collect_nodes(root, nodes);
    TreeContraction::TreeContract(nodes, root, threads);
    return *root->value;
}

template<class F>
double time_ms(F&& f) {
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main(int argc, char* argv[])
{
    const int REPS = (argc > 1) ? std::stoi(argv[1]) : 10;
    const double tolFactor = (argc > 2) ? std::stod(argv[2]) : 1e-6;
    const double tolExp = (argc > 3) ? std::stod(argv[3]) : 1.0001;

    std::vector<unsigned> testDepths{2, 6, 9, 12};

    const unsigned HW_THREADS =
        std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1;
    std::vector<unsigned> threadCounts;
    for (unsigned v = 1; v <= HW_THREADS; v <<= 1) threadCounts.push_back(v);
    if (threadCounts.back() != HW_THREADS) threadCounts.push_back(HW_THREADS);

    std::mt19937 master(42);
    std::cout << "type,depth,threads,n_nodes,baseline_ms,contraction_ms\n";

    auto run_case = [&](const std::string& shape,
                        unsigned depth,
                        auto treeMaker,
                        unsigned threads,
                        unsigned long seed)
    {
        std::mt19937 tmpRng(seed + 999999);
        Node::Ptr example = treeMaker(depth, tmpRng, true, '+');
        std::size_t n_nodes = countNodes(example);

        std::vector<double> baselineVals(REPS);
        double base_sum = 0.0;
        for (int i = 0; i < REPS; ++i) {
            std::mt19937 gi(seed + i);
            Node::Ptr tmp = treeMaker(depth, gi, true, '+');
            double val = 0.0;
            base_sum += time_ms([&] { val = tmp->compute(); });
            baselineVals[i] = val;
        }
        double base_ms = base_sum / REPS;

        double contr_sum = 0.0;
        for (int i = 0; i < REPS; ++i) {
            std::mt19937 gi(seed + i);
            Node::Ptr tmp = treeMaker(depth, gi, true, '+');
            double val = 0.0;
            contr_sum += time_ms([&] { val = runTreeContraction(tmp, threads); });
            double tol = tolFactor * std::pow(static_cast<double>(n_nodes), tolExp);
            std::cout<<val<<" "<<baselineVals[i]<<"\n";
            assert(std::fabs(baselineVals[i] - val) <= tol);
        }
        double contr_ms = contr_sum / REPS;

        std::cout << shape << ','
                  << depth << ','
                  << threads << ','
                  << n_nodes << ','
                  << base_ms << ','
                  << contr_ms << '\n';
    };

    for (unsigned d : testDepths) {
        unsigned long seed = master();
        for (unsigned t : threadCounts) {
            run_case("perfectBin", d, bench::perfectBin, t, seed);
            run_case("randomBalanced", d, bench::randomBalanced, t, seed);
        }
    }

    return 0;
}


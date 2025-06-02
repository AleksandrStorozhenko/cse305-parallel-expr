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

static inline void collect_nodes(Node* n, std::vector<Node*>& v) {
    if (!n) return;
    v.push_back(n);
    for (auto* c : n->children()) collect_nodes(c, v);
}

double runTreeContraction(Node* root, unsigned threads = 1) {
    std::vector<Node*> nodes;
    collect_nodes(root, nodes);
    ::TreeContraction::TreeContract(nodes, root, threads);
    return *root->value;
}

// timer
template<class F>
double time_ms(F&& f)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double,std::milli>(t1 - t0).count();
}

// counters to avoid private problems
static inline std::size_t caterpillarNodes(std::size_t spine){return 3 * spine - 2;}

static std::size_t accordionNodes(unsigned d) {
    if (d == 0) return 1;
    if (d % 2 == 0) return 3 + accordionNodes(d-1);
    return 2 + accordionNodes(d-1);
}

static inline std::size_t fibNodes(unsigned d)
{
    std::size_t a = 1, b = 1;
    for (unsigned i = 0; i < d; ++i) { std::size_t t = a + b; a = b; b = t;}
    return b - 1;
}

int main(int argc, char* argv[])
{
    const int REPS = (argc > 1) ? std::stoi(argv[1]) : 20;
    const double tolFactor = (argc > 2) ? std::stod(argv[2]) : 1e-12;
    const double tolExp = (argc > 3) ? std::stod(argv[3]) : 1.0;

    // tables
    const std::vector<unsigned> perfectDepth = {6, 9, 12};
    const std::vector<std::size_t> chainLen = {32, 256, 2048};
    const std::vector<unsigned> fibDepth = {8, 14, 20};
    const std::vector<std::size_t> rndInt = {127, 1023, 4095};
    const std::vector<std::size_t> catSpine = {32, 256, 2048};
    const std::vector<unsigned> accDepth = {7, 11, 15};

    // thread
    const unsigned HW_THREADS = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1;
    std::vector<unsigned> threadCounts;
    for (unsigned v = 1; v <= HW_THREADS; v <<= 1)
        threadCounts.push_back(v);
    if (threadCounts.back() != HW_THREADS)
        threadCounts.push_back(HW_THREADS);

    std::mt19937 master(42);
    std::cout << "shape,n_nodes,threads,baseline_ms,contraction_ms\n";

    auto run_case = [&](const std::string& shape, std::size_t n_nodes, auto maker, auto param, unsigned threads, unsigned long seed)
    {
        std::vector<double> baselineVals(REPS);
        double base_sum = 0.0;
        for (int i = 0; i < REPS; ++i) {
            std::mt19937 gi(seed + i);
            Node* tmp = maker(param, gi, true, '+');
            double val = 0.0;
            base_sum += time_ms([&]{ val = tmp->compute(); });
            baselineVals[i] = val;
            delete tmp;
        }
        double base_ms = base_sum / REPS;

        double contr_sum = 0.0;
        for (int i = 0; i < REPS; ++i) {
            std::mt19937 gi(seed + i);
            Node* tmp = maker(param, gi, true, '+');
            double val = 0.0;
            contr_sum += time_ms([&]{ val = runTreeContraction(tmp, threads); });
            const double tol = tolFactor * std::pow(static_cast<double>(n_nodes), tolExp);
            assert(std::fabs(baselineVals[i] - val) <= tol);
        }
        double contr_ms = contr_sum / REPS;

        std::cout << shape << ',' << n_nodes << ',' << threads << ','
                  << base_ms << ',' << contr_ms << '\n';
    };

    auto one_shape = [&](const std::string& shape, auto list, auto maker, auto nodesFn)
    {
        for (auto p : list) {
            unsigned long seed = master();
            std::size_t n_nodes = nodesFn(p);
            for (unsigned t : threadCounts)
                run_case(shape, n_nodes, maker, p, t, seed);
        }
    };

    one_shape("perfect", perfectDepth, bench::perfectBin, [](unsigned d){ return (1u << (d+1)) - 1u; });
    auto chainNodes = [](std::size_t len){ return 2*len - 1; };
    one_shape("left_chain", chainLen, bench::leftChain, chainNodes);
    one_shape("right_chain",chainLen, bench::rightChain, chainNodes);
    one_shape("fibonacci", fibDepth, bench::fibTree, fibNodes);
    one_shape("random", rndInt, bench::randomTree,[](std::size_t k){ return 2*k + 1; });
    one_shape("caterpillar",catSpine, bench::caterpillar,caterpillarNodes);
    one_shape("accordion", accDepth, bench::accordion, accordionNodes);

    return 0;
}
# cse305-parallel-expr
Run with
```
make APP=bench
./bench
```
Miller-Reif algorithm for tree contraction of expression trees. 
In order to use on different trees, API call is 
```
TreeContraction::TreeContract(const std::vector<Node::Ptr>& nodes,
                                    const Node::Ptr& root,
                                    int num_threads,
                                    SimplePool& pool)
```
It requires a list of all the nodes, the root, and a threadpool.
In order to build trees, use the class `Node` and it's children. Examples are given in `sources/benchmark.cpp`.
For our data, check the folder `benchmarks`.
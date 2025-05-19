#include <iostream>
#include "Node.h"
#include "ValueNode.h"
#include "PlusNode.h"
#include "MultiplyNode.h"
#include "DivideNode.h"

int main() {
    Node* root = new MultiplyNode(
        new PlusNode(
            new ValueNode(3),
            new ValueNode(5)
        ),
        new ValueNode(2)
    );

    double result = root->compute();

    std::cout << "Result of (3 + 5) * 2 is: " << result << std::endl;

    delete root; 

    return 0;
}

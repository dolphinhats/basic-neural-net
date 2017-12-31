#include "Node.h"
#include "TreeNode.h"
#include "NeuralNet.h"
#include "NeuralNetLayer.h"

int main (int argc, char** argv)
{
    MAX_THREAD_COUNT = std::thread::hardware_concurrency();
    CURRENT_THREAD_COUNT = 1;

    NeuralNet<InputNode,OutputNode>* net = new NeuralNet<InputNode,OutputNode>(1,4096);
//todo Everything needed to run the neural net
}

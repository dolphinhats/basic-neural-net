#ifndef NODE_H_
#define NODE_H_

#include <cstdio>

//todo Figure out some kind of thread management because the current method is not very practical
//arbitrary number of elements to work on which decides when we should split into multiple threads
#define MULTITHREADING_SPLIT_THRESHOLD 10

static unsigned MAX_THREAD_COUNT;
static unsigned MAX_THREADS_PER_NODE;
static unsigned MAX_THREADS_PER_LAYER;
atomic<unsigned> CURRENT_THREAD_COUNT;

atomic<unsigned> TOP_LAYER; //the highest layer in the network
atomic<unsigned> BOTTOM_LAYER; //the current lowest-unevaluated layer in the network
atomic<unsigned> BOTTOM_LAYER_SPLIT_DELTA; //the distance from the bottom layer another layer must fall within before it splits the work into separate threads

//todo Update interface given the changes to the layer class and the treeNode class
class Node
{
public:
    virtual Node(unsigned) = 0;
    virtual ~Node() = 0;
    
    virtual unsigned double prop() = 0; //propegate value up the tree (meant to be called by parent node)
    virtual void backProp(unsigned double) = 0;
};

#endif

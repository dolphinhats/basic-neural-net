#include <cstdio>
#include <list>
#include <vector>
#include <utility>
#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>

//wait time for threads to await asyncronous value 
#define THREAD_WAIT_TIME std::chrono::seconds(1)

//arbitrary number of children which decides when we should split into multiple threads
#define MULTITHREADING_SPLIT_THRESHOLD 10

static unsigned MAX_THREAD_COUNT;
static unsigned MAX_THREADS_PER_NODE;
atomic<unsigned> CURRENT_THREAD_COUNT;

atomic<unsigned> TOP_LAYER; //the highest layer in the network
atomic<unsigned> BOTTOM_LAYER; //the current lowest-unevaluated layer in the network
atomic<unsigned> BOTTOM_LAYER_SPLIT_DELTA; //the distance from the bottom layer another layer must fall within before it splits the work into separate threads

/*
class ThreadHandler
{
private:
    static unsigned maxThreads;
    
    
public:
    ThreadHandler(m)
	{
	    maxThreads = m;
	}
    
    ~ThreadHandler()
	{

	}
    void reserveThreads(unsigned count)
	{

	}
}
*/

class Node
{
public:
    virtual Node(unsigned) = 0;
    virtual ~Node() = 0;

    virtual unsigned double prop() = 0; //propegate value up the tree (meant to be called by parent node)
    virtual void backProp(unsigned double) = 0;
};

class NodeConnection
{
private:
    
public:
    
};

#define NodePair std::pair<Node*, unsigned double>

class TreeNode : public Node
{
private:
    const unsigned int layer;

    std::atomic<bool> lock;
    
    unsigned double value;

    unsigned double learningRate;
    
    
    std::vector<NodePair*>* prevNodes;
    
    std::vector<NodePair*>* nextNodes;

    void eval(unsigned start, unsigned end, unsigned double* tempValue)
	{
	    *tempValue = 0.0;
	    for (unsigned i = start; i < end; i++)
	    {
		NodePair* node = nodes->at(i);
		unsigned double nodeValue = node->first->prop();
		unsigned double weight = node->second;
		*tempValue += nodeValue * weight;
	    }
	}

    void updateWeight(unsigned start, unsigned end, unsigned double actualValue)
	{
	    for (unsigned i = start; i < end; i++)
	    {
		NodePair* nodePair = nodes->at(i);
		Node* node = nodePair->first;
		unsigned double weight = nodePair->second;
		
		unsigned double nodeValue = node->prop();
		
		//to be honest, I'm not particularlly good at math so this is the best I got with the minute knowledge I have coupled with about an hour of internet research that was honestly far over my head.
		weight = weight - (learningRate * (nodeValue-actualValue)*(nodeValue*(1-nodeValue)));

		delete nodePair;
		nodePair = new NodePair(node,weight);
	    }
	}

    unsigned double splitWork(unsigned double backprop = 2)
	{
	    unsigned double tempValue = 0.0;
	    unsigned long nodeCount = prevNodes->size();

	    if (prevNodes->size() > MULTITHREADING_SPLIT_THRESHOLD && //if we have a certain number of children then we will split and ...
		CURRENT_THREAD_COUNT < MAX_THREAD_COUNT && //if there are threads available and ...
		BOTTOM_LAYER + BOTTOM_LAYER_SPLIT_DELTA > layer) //if this node is at the layer below the delta from bottom-most working layer
		//basically, we don't want to waste time splitting threads when we're near the leaf nodes of the tree
	    {
		unsigned numOfThreads = 0;
		if (MAX_THREAD_COUNT - CURRENT_THREAD_COUNT < MAX_THREADS_PER_NODE)
		{
		    numOfThreads = MAX_THREAD_COUNT - CURRENT_THREAD_COUNT;
		}
		else
		{
		    numOfThreads = MAX_THREADS_PER_NODE;
		}
		
		CURRENT_THREAD_COUNT += numOfThreads;
		
		std::thread** threads = new std::thread*[numOfThreads];
		unsigned double** tempValues = new unsigned double*[numOfThreads];
		
		unsigned nodesPerThread = floor(nodeCount/numOfThreads);
		unsigned currThread = 0;
		
		for (unsigned i = 0; i < nodeCount; )
		{
		    unsigned end = i+nodesPerThread;
		    unsigned double* tempValue = new unsigned double();
		    tempValues[currThread] = tempValue;
		    if (backprop == 2)
		    {
			threads[currThread] = new std::thread(eval,i,end,tempValue);
		    }
		    else
		    {
			threads[currThread] = new std::thread(updateWeight,i,end,backprop);
		    }
		    currThread++;
		    i += nodesPerThread;
		}
			
		for (unsigned i = 0; i < numOfThreads; i++)
		{
		    threads[i]->join();
		    tempValue += *(values[i]);
		}
		return tempValue;
	    }
	    else
	    {
		if (backprop == 2)
		{
		    eval(0,prevNodes->size(),&tempValue);
		}
		else
		{
		    tupdateWeight(0,prevNodes->size(),backprop);
		}
		return tempValue;
	    }
	}

public:
    //todo include biases
    //learning rate
    TreeNode(unsigned int l, unsigned double r)
	{
	    layer = l;
	    learningRate = r;
	    haveEvaled = 0;
	    lock = false;
	    currNodes = 0;
	    prevNodes = new std::vector<NodePair*>();
	    nextNodes = new std::vector<NodePair*>();
	}
    
    virtual ~TreeNode()
	{
	    delete prevNodes;
	}

    unsigned double prop()
	{
	    if (isLocked())
	    {
		//if this node is currently locked then it is being processed and we need to weight until it is done
		while(isLocked())
		{
		    std::this_thread::yield();
		}
	    
		return value;
	    }
	    lock();

	    value = splitWork();
	    value = 1/(1-(exp(-1*value))); //scaling
	    
	    clearLock()
	    
	    return value;
	}
    
    void backProp(unsigned double w)
	{
	    if (isLocked()) return; //if so, continue processing other nodes
	    lock();
	    
	    splitWork(w);
	    
	    clearLock();
	}
    
    void pushNode(Node* n, unsigned double w)
	{
	    NodePair* pair = new NodePair(n,w);
	    prevNodes->push_back(pair);
	}

    void unlock()
	{
	    lock = false;
	}
    
    void clearLock()
	{
	    lock = false;
	}

    void lock()
	{
	    lock = true;
	}

    bool isLocked()
	{
	    return lock;
	}

};

template <class T : public Node>
class NeuralNetLayer
{
private:
    T** nodes;

    unsigned nodeCount;
    unsigned layerNumber;
    
public:
    NeuralNetLayer(unsigned c, unsigned n)
	{
	    nodeCount = c;
	    layerNumber = n;
	    nodes = new T*[nodeCount];

	    for (int i=0; i<nodeCount; i++)
	    {
		nodes[i] = new T(layerNumber);
	    }
	}
    
    virtual ~NeuralNetLayer()
	{
	    for (int i=0; i<nodeCount; i++)
	    {
		delete nodes[i];
	    }
	    delete [] nodes;
	}

    unsigned getNodeCount()
	{
	    return nodeCount;
	}
    
    unsigned getSize()
        {
	    return nodeCount;
	}
        
    //reset evaled status between evaluations
    void clearEvaled()
	{
	    for (int i=0; i<nodeCount; i++)
	    {
		T* node = nodes[i];
		it->clearEvaled();
	    }
	}
    
    void addChildLayer(NeuralNetLayer<Node>* layer)
	{
	    unsigned mostNodeCount = 0;
	    if (layer->getSize() > nodeCount)
	    {
		mostNodeCount = layer->getSize();
	    }
	    else
	    {
		mostNodeCount = nodeCount;
	    }
	    
	    for (int i=0; i<mostNodeCount; i++)
	    {
		//setting weight of 0.5 by default
		node[i%nodeCount]->pushNode(layer->getElement(i%layer->getNodeCount()),0.5);
	    }
	}

    Node* getElement(unsigned int i)
	{
	    return nodes[i];
	}
    
    Node* operator[] (index_type i)
	{
	    return nodes[i];
	}
};

template<class I, class O>
class NeuralNet
{
private:
    NeuralNetLayer<TreeNode>** layers;
    unsigned layerCount;

    unsigned nodesPerLayer;
    
    NodeLayer<I> inputLayer;
    NodeLayer<O> outputLayer;
    
public:
    NeuralNet(unsigned c, unsigned n)
	{
	    layerCount = c;
	    nodesPerLayer = n;

	    //create layers
	    inputLayer = new NeuralNetLayer<I>(n);
	    outputLayer = new NeuralNetLayer<O>(n);

	    layers = new NeuralNetLayer<TreeNode>*[layerCount];
	    for (int i=0; i<layerCount; i++)
	    {
		layers[i] = new NeuralNetLayer<TreeNode>(nodesPerLayer);
	    }

	    //link all the layers
	    layers[0]->addChildLayer(inputLayer);
	    for (int i=1; i<layerCount; i++)
	    {
		layers[i]->addChildLayer(layers[i-1]);
	    }
	    outputLayer->addChildLayer(layers[layerCount-1]);
	}
    
    virtual ~NeuralNet()
	{
	    delete inputLayer;
	    delete outputLayer;
	    for (int i=0; i<layerCount;i++)
	    {
		delete layers[i];
	    }
	    delete [] layers;
	}
    
};

class InputNode : public Node
{
public:
    //depending on the implementation
    virtual InputNode(unsigned) = 0;
    virtual ~InputNode() = 0;

    virtual unsigned double prop() = 0; //propegate value up the tree (meant to be called by parent node)
    virtual void backProp(unsigned double) = 0;
};

class OutputNode : public Node
{
public:
    //depending on the implementation
    virtual OutputNode(unsigned) = 0;
    virtual ~OutputNode() = 0;

    virtual unsigned double prop() = 0; //propegate value up the tree (meant to be called by parent node)
    virtual void backProp(unsigned double) = 0;
};

int main (int argc, char** argv)
{
    MAX_THREAD_COUNT = std::thread::hardware_concurrency();
    CURRENT_THREAD_COUNT = 1;

    NeuralNet<InputNode,OutputNode>* net = new NeuralNet<InputNode,OutputNode>(1,4096);
    
}

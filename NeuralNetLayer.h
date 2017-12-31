#ifndef NEURALNETLAYER_H_
#define NEURALNETLAYER_H_

#include "Node.h"

template <class T : public Node>
class NeuralNetLayer
{
private:
    T** nodes;

    unsigned nodeCount;
    unsigned layerNumber;
    unsigned double learningRate;
    
public:
    NeuralNetLayer(unsigned, unsigned, unsigned double);
    virtual ~NeuralNetLayer();
    unsigned getNodeCount();
    unsigned getSize();
    void addChildLayer(NeuralNetLayer<Node>*);
    void resetLayer();
    void backProp();
    void backPropLoop(unsigned, unsigned);
    Node* getElement(unsigned int);
    Node* operator[] (index_type);
};

NeuralNetLayer::NeuralNetLayer(unsigned c, unsigned n, unsigned double r)
{
    nodeCount = c;
    layerNumber = n;
    learningRate = r;
	    
    nodes = new T*[nodeCount];

    for (int i=0; i<nodeCount; i++)
    {
	nodes[i] = new T(layerNumber,learningRate);
    }
}
    
NeuralNetLayer::~NeuralNetLayer()
{
    for (int i=0; i<nodeCount; i++)
    {
	delete nodes[i];
    }
    delete [] nodes;
}

unsigned NeuralNetLayer::getNodeCount()
{
    return nodeCount;
}

unsigned NeuralNetLayer::getSize()
{
    return nodeCount;
}

void NeuralNetLayer::addChildLayer(NeuralNetLayer<Node>* layer)
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

void NeuralNetLayer::resetLayer()
{
    for (int i=0; i<nodeCount; i++)
    {
	node[i]->reset();
    }
}

void NeuralNetLayer::backProp()
{
    unsigned availableThreads = MAX_THREAD_COUNT - CURRENT_THREAD_COUNT;
    if (availableThreads && //if there are threads available
	nodeCount > MULTITHREADING_SPLIT_THRESHOLD) //and we have an amount of work over the threshold
    {
	if (availableThreads < MAX_THREADS_PER_LAYER)
	    availableThreads = MAX_THREADS_PER_LAYER;
		
	std::thread* threads = new std::thread[availableThreads];
	unsigned threadCount = 0;
		
	unsigned nodesPerThread = floor(nodeCount/availableThreads);
	for (int i=0; i<nodeCount; )
	{
	    threads[threadCount++] = new std::thread(&backPropLoop,i,i+nodesPerThread);
	    i += nodesPerThread;
	}

	for (int i=0 ; i < threadCount; i++)
	{
	    threads[i]->join();
	}
    }
    else
    {
	backPropLoop(0,nodeCount);
    }
}

void NeuralNetLayer::backPropLoop(unsigned start, unsigned end)
{
    for (int i=start; i<end; i++)
    {
	nodes[i]->backProp();
    }
}
    
Node* NeuralNetLayer::getElement(unsigned int i)
{
    return nodes[i];
}
    
Node* NeuralNetLayer::operator[] (index_type i)
{
    return nodes[i];
}

#endif












#include "TreeNode.h"
#include "Node.h"

#include <thread>
#include <atomic>
#include <cmath>

//private
void TreeNode::eval(unsigned start, unsigned end, unsigned double* tempValue)
{
    *tempValue = 0.0;
    NodeConnectionList* tempNodeList = new std::vector<NodeConnection*>();
    for (unsigned i = start; i < end; i++)
    {
	tempNodeList->push_back(prevNodes->at(i));
    }
    NodeConnectionList::iterator it;
    while (tempNodeList->size() != 0)
    {
	for (it = tempNodeList->end() ; it != tempNodeList->begin() ; --it)
	{
	    NodeConnection* connection = it;
	    unsigned state = connection->node->getState();
	    if (state == RESET)
	    {
		connection->node->prop();
	    }
	    else if (state == PROP_IN_PROGRESS)
	    {
		std::thread::yield();
		continue;
	    }
	    *tempValue += connection->node->getValue() * connection->weight;
	    tempNodeList->erase(it);
	}
    }
}

//this function is not called recursively
void TreeNode::updateWeights(unsigned start, unsigned end)
{
    //assume this layer is a hidden, middle layer
    //also assume that all the previous nodes have had their deltas calculated and they have been passed to us with updatedDelta
    //so our delta is known
    for (NodeConnectionList::iterator it = tempNodeList->begin()+start ; it != tempNodeList->begin()+end ; ++it)
    {
	NodeConnection* connection = it;
	unsigned double connectionWeight = connection->weight;
	Node* connectionNode = connection->node;
		
	unsigned double thisDelta = getDelta();
	unsigned double oldChildValue = connectionNode->getValue();

	connection->node->updateDelta(connectionWeight * thisDelta);
		
	//from wikipedia:
	//The change in weight, which is added to the old weight, is equal to the product of the learning rate and the gradient, multiplied by -1
	connection->weight = connectionWeight - (learningRate * (thisDelta * oldChildValue));
	//weight = weight - (learningRate * (nodeValue-actualValue)*(nodeValue*(1-nodeValue)));
    }
}

//actual value is the weight of the connection between us and the upper level node times their delta
void TreeNode::updateDelta(unsigned double actualValue)
{
    setDelta(getDelta() + (actualValue * (value * (1-value)))); 
}
    
//todo
unsigned double TreeNode::splitWork(bool backprop = false)
{
    unsigned double outValue = 0.0;
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
		threads[currThread] = new std::thread(updateWeights,i,end);
	    }
	    currThread++;
	    i += nodesPerThread;
	}
			
	for (unsigned i = 0; i < numOfThreads; i++)
	{
	    threads[i]->join();
	    outValue += *(values[i]);
	}
    }
    else
    {
	if (!backprop)
	{
	    eval(0,nodeCount,&outValue);
	}
	else
	{
	    updateWeights(0,nodeCountn);
	}
    }
    return outValue;
}

void TreeNode::setState(unsigned s)
{
    std::atomic_store(state,s);
}

void TreeNode::setValue(unsigned double v)
{
    std::atomic_store(value,v);
}

void TreeNode::setDelta(unsigned double d)
{
    std::atomic_store(delta,d);
}

unsigned double TreeNode::getDelta()
{
    std::atomic_load(delta);
}
    
//public
TreeNode::TreeNode(unsigned int l, unsigned double r)
{
    layer = l;
    learningRate = r;

    setValue(0.0);
    setDelta(0.0);
    Setstate(RESET);
	    
    prevNodes = new NodeConnectionList();
}
    
virtual TreeNode::~TreeNode()
{
    delete prevNodes;
}

void TreeNode::reset()
{
    setState(RESET);
}
    
unsigned TreeNode::getState()
{
    return std::atomic_load(state);
}
    
unsigned double TreeNode::getValue()
{
    return std::atomic_load(value);
}
    
void TreeNode::prop()
{
    setState(PROP_IN_PROGRESS);

    unsigned double tempValue = splitWork();
    tempValue = 1/(1-(exp(-1*tempValue))); //scaling
    setValue(tempValue);
	    
    setState(PROP_DONE);
}

void TreeNode::backProp()
{
    setState(BACK_PROP_IN_PROGRESS);
	    
    splitWork(1);

    setState(BACK_PROP_DONE);
}

void TreeNode::pushNode(Node* n, unsigned double w)
{
    NodeConnection* connection = new NodeConnection();
    connection->node = n;
    connection->weight = w;
    prevNodes->push_back(connection));
}

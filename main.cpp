#include <cstdio>
#include <list>
#include <utility>
#include <thread>
#include <atomic>
#include <chrono>
#include <math.h>

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
    
#define NodePair std::pair<Node*, unsigned double> 
    
private:
    const unsigned int layer;

    std::atomic<unsigned char> haveEvaled; //0 for not evaluated at all, 1 for evaluation finished, 2 for evaluation in progress
    unsigned double value;

    //a list of pairs where the first element is the connected node from the previous layer and the second element is the weight of their connection.
    std::list<NodePair>* prevNodes;

    unsigned double eval(unsigned double value, unsigned double weight)
	{
	    return value * weight; //todo
	}

    void spawnThread(unsigned start, unsigned end, unsigned double* valuePtr, std::thread* thread)
	{
	    unsigned double value = *valuePtr;
	    value = 0;
	    thread = new std::thread([start,end,value](){
		    for (std::list<NodePair>::iterator iter = prevNodes->begin()+start; iter < prevNodes->begin()+end; ++iter)
		    {
			NodePair* pair = iter;
			value += eval(pair->first->prop(),pair->second);
		    }
		});
	}
    
public:
    Node(unsigned int l)
	{
	    layer = l;
	    haveEvaled = 0;
	    currNodes = 0;
	    prevNodes = new std::list<NodePair>();
	}
    
    virtual ~Node()
	{
	}

    unsigned double prop()
	{
	    if (haveEvaled == 1) return value;
	    if (haveEvaled == 2) //multithreaded wait for evaluation from another thread
	    {
		//there are better ways but I don't want to bother with those right now
		while (haveEvaled == 2)
		{
		    std::this_thread::yield();
		}
		return value;
	    }
	    
	    haveEvaluated = 2;

	    value = 0;

	    if (prevNodes->size() > MULTITHREADING_SPLIT_THRESHOLD && //if we have a certain number of children then we will split and ...
		CURRENT_THREAD_COUNT < MAX_THREAD_COUNT && //if there are threads available and ...
		BOTTOM_LAYER + BOTTOM_LAYER_SPLIT_DELTA > layer) //if this node is at the layer below the delta from bottom-most working layer
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
		unsigned double** values = new unsigned double*[numOfThreads];
		
		unsigned long nodeCount = prevNodes->size();
		unsigned long nodesPerThread = floor(nodeCount/numOfThreads);
		unsigned currThread = 0;
		
		for (unsigned long i = 0; i < nodeCount; i++)
		{
		    unsigned long end = i+nodesPerThread;
		    unsigned double* tempValue = new unsigned double();
		    values[currThread] = tempValue;
		    spawnThread(i,end,tempValue,threads[currThread]);
		    currThread++;
		}
			
		for (unsigned i = 0; i < numOfThreads; i++)
		{
		    threads[i]->join();
		    value += *(values[i]);
		}
	    }
	    else
	    {
		for (std::list<NodePair>::iterator it=prevNodes->begin(); it != prevNodes->end(); ++it)
		{
		    NodePair* pair = it;
		    value += eval(pair->first->prop(),pair->second);
		}
	    }
	    haveEvaluated = 1;
		
	    return value;
	}
    
    void pushNode(Node* n, unsigned double w)
	{
	    NodePair* pair = new NodePair(n,w);
	    prevNodes->push(pair);
	}
    
    void clearEvaled()
	{
	    haveEvaled = 0;
	}
};

int main (int argc, char** argv)
{
    MAX_THREAD_COUNT = std::thread::hardware_concurrency();
    CURRENT_THREAD_COUNT = 1;
}

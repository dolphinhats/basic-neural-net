#ifndef TREENODE_H_
#define TREENODE_H_

class Node
{
};

namespace std
{
    template <class T>
    class atomic
    {
    };
}

struct NodeConnection
{
    Node* node;
    unsigned double weight;
};

#define std::vector<NodeConnection*> NodeConnectionList

class TreeNode : public Node
{
private:
    const unsigned int layer;
    const unsigned double learningRate;
    
    std::atomic<unsigned> state;
    std::atomic<unsigned double> value;
    std::atomic<unsigned double> delta;
    
    NodeConnectionList* prevNodes;

    void eval(unsigned, unsigned, unsigned double*);
    void updateWeights(unsigned,unsigned);
    void updateDelta(unsigned double);

    unsigned double splitWork(bool);
	
    void setState(unsigned);
    void setValue(unsigned double);
    void setDelta(unsigned double);
    unsigned double getDelta();

public:
    TreeNode(unsigned int, unsigned double);
    virtual ~TreeNode();

    void reset();
    unsigned getState();
    unsigned double getValue();
    void prop();
    void backProp();
    void pushNode(Node*, unsigned double);
};

#endif

#ifndef NEURALNET_H_
#define NEURALNET_H_

template<class T>
class NeuralNetLayer
{};

class TreeNode
{};

class Node
{};

//class I and O are the input and output classes respectfully
template<class I : public Node, class O : public Node>
class NeuralNet
{
private:
    NeuralNetLayer<TreeNode>** layers;
    unsigned layerCount;

    //need to make this adjustable
    unsigned nodesPerLayer;
    
    NodeLayer<I> inputLayer;
    NodeLayer<O> outputLayer;
    
public:
    NeuralNet(unsigned, unsigned);
    virtual ~NeuralNet();

};

NeuralNet::NeuralNet(unsigned c, unsigned n)
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
    
NeuralNet::~NeuralNet()
	{
	    delete inputLayer;
	    delete outputLayer;
	    for (int i=0; i<layerCount;i++)
	    {
		delete layers[i];
	    }
	    delete [] layers;
	}
#endif

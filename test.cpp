#include <cstdio>
#include <thread>

int main(int argc, char** argv)
{
    for (int i=0;i<100;i++)
    {
	std::thread* thread = new std::thread([i](){
		int a = i;
		std::printf("%i\n",a);
	    });
    }
    return 0;
}

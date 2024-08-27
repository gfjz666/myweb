#include "log.h"
int main()
{
    //buffer * b = new buffer();
    std::unique_ptr<buffer> b = std::make_unique<buffer>();
    std::cout<<b->WritableBytes()<<"\n";
    return 0;
}
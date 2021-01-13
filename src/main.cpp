#include <mpris.h>

int main()
{
    Mpris mpris;
    Metadata* metadata = mpris.GetMetadata();
    std::cout << *metadata << std::endl;
}
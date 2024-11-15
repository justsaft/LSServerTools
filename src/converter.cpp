#include <iostream>
#include <stdint.h>
#include <vector>


int main(int argc, char** argv)
{
    if (!(argv [1]))
    {
        std::cout << "No string was given to convert" << std::endl;
        return 1;
    }


    for (uint8_t i = 0; argv [i]; i++)
    {
        std::vector<uint8_t> result;
        result.reserve(16);

        std::string str = argv [i];
        for (char c : str)
        {
            if (c >= 97 && c <= 122)
                c -= 32;

            if (c <= 90 && c >= 65)
            {
                result.push_back(c - 65);
            }
        }
        std::cout << "\\x" << result.data() << std::endl;
    }
    return 0;
}
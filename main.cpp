#include <iostream>
#include <string.h>

using namespace std;

void greeting(int nSize)
{
    char* p = (char*) malloc(nSize* sizeof(char));
    if (nSize < 12 || p == NULL)
    {
        printf("Out of memory\n");
        return;
    }

    strcpy(p, "hello");
    free(p);
}
int main()
{
    while (1)
    {
        greeting(11);
    }
    return 0;
}
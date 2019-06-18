#include "gc_pointer.h"
#include "LeakTester.h"

int main()
{
    Pointer<int> p = new int(19);
    p = new int(21);
    p = new int(28);
    int *ptr = new int;
    Pointer<int> p1(ptr);
    Pointer<int> p2(p1);
    Pointer<int> p3 = p2;
    return 0;
}
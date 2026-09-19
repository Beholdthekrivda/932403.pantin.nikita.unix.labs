#include <iostream>

using namespace std;

int main()
{   
    int a, b, c, n = 10;

    a = 0; b = 1;

    for (int i = 0; i < n; ++i)
    {
        c = a + b;
        a = b;
        b = c;
        // &Output: test_compile
        cout << a << " ";
    }
    cout << endl;
    return 0;
}

#include <fstream>
#include "library/cStateObserver.h"

int main()
{
    unique_ptr<cStateObserver> so( make_unique<cStateObserver>() );
    return 0;
}

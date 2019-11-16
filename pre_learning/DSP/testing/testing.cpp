#include <fstream>
#include <iostream>
#include <algorithm>
#include "library/cStateObserver.h"
#include <iterator>
#include <valarray>

int main()
{
    using namespace std;
    unique_ptr<cStateObserver> so( make_unique<cStateObserver>() );
    unsigned lenghtOfRecord(0);

    ifstream ifs("data/SlidersTest.log");
    ifs.unsetf(std::ios_base::skipws);
    lenghtOfRecord = count( istream_iterator<char>(ifs), istream_iterator<char>(), '\n');
    cout << "#" << lenghtOfRecord << " items" << endl;
    ifs.clear();
    ifs.seekg(0);

    valarray<float> x(lenghtOfRecord +1),y(lenghtOfRecord +1);

    {
        string l;
        size_t k(0);
        float dummy(0);
        while( getline(ifs, l) )
        {
            stringstream iss(l);
            ( iss >> dummy >> x[k] >> y[k]);
            k++ ;
        }
    }

    for( auto s : y )
    {
        auto f = so->sync( s );
        cout << s << " ";copy( f.begin(), f.end(), ostream_iterator<float>( cout," ") ); cout << "\n";
    }
    return 0;

}

#include <memory>
#include <thread>
#include <iostream>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string_view>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <valarray>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <sys/time.h>
#include <boost/format.hpp>
#include <ctime>
#include <sys/time.h>

using namespace boost;
using boost::format;

using namespace std;


static string snapshot()
{
    std::ostringstream oss;
    oss << " ( " << setw(5) << getpid( ) << " )( ";
    time_t t; time( & t );
    struct tm * tm = localtime( & t );
    struct timeval tv; gettimeofday( & tv, nullptr);
    oss << setfill ('0') << setw (2) << tm->tm_hour;
    oss << ":" << setfill ('0') << setw (2) << tm->tm_min;
    oss << ":" << setfill ('0') << setw (2) << tm->tm_sec;
    oss << ":" << setfill ('0') << setw (3) << static_cast<int>(round( tv.tv_usec / 1000 )) << " ) ";
    return oss.str( );
}

template <typename ...T>
void auditor( std::string backtrace, T... args )
{
    std::cout << boost::format("%s: %20s") % snapshot() % backtrace;
    std::cout << std::endl;
}

void PointersStuff()
{
    static auto sleep = [](int elapsed){ this_thread::sleep_for( std::chrono::milliseconds(elapsed)); };

    class House
    {
    public:
        House( string _name ="" ) : name( _name ) {}
        ~House(){ auditor( __FUNCTION__ + ( name.empty() ? "" : ( string( "." )+ name ) ) );  }
        const string & named(){ return name; }

    private:
         string name;
    };

    class TreeHouse : public House
    {
    public:
        TreeHouse( std::string name ) : House( "Tree." + name ){ }
        ~TreeHouse(){ auditor( __FUNCTION__ ); }
    };

    {
        cout << std::endl;
        auditor("UniquePointer and Move rationale ::" );

        using HousePtr = std::unique_ptr<House>;
        class HouseKeeper
        {
        public:
            HouseKeeper( HousePtr house_owner = nullptr )
            {
                if( house_owner != nullptr )
                    owner = std::move( house_owner );
            }
            void slide( HousePtr house_owner ){ owner = std::move( house_owner ); }
            ~HouseKeeper(){ auditor( __FUNCTION__ ); }

        private:
            HousePtr owner;
        };

        using HouseKeeperPtr = std::unique_ptr<HouseKeeper>;
        HouseKeeperPtr keeper( new HouseKeeper() );
        {
            auto UniqueHouseFactory = []( std::string name ){ return std::make_unique<House>(name);};
            HousePtr p( UniqueHouseFactory( "DieFirst") );
            keeper->slide( UniqueHouseFactory("DieJoined") );
        }
    }


    {
        cout << std::endl;
        auditor("SharedPointer rationale ::" );
        using HousePtr = std::shared_ptr<House>;

        HousePtr h( std::make_shared<House>("House#shared") );
        thread( [h](){ auditor( h->named() ); }).detach();
        thread( [h](){ sleep(100); auditor( h->named() ); }).detach();
    }

    sleep(1000);

    {
        cout << std::endl;
        auditor("WeakPointer rationale ::" );

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0,100);

        std::weak_ptr<House> w;
        {
            auto TreeHouseFactory = []( std::string name ){ return std::make_shared<TreeHouse>(name);};
            using TreeHousePtr = std::shared_ptr<TreeHouse>;
            TreeHousePtr h = TreeHouseFactory("House#Shared");
            w = h;
            thread( [&]( TreeHousePtr h )
                   {
                       sleep(dis(gen));
                       auditor( __PRETTY_FUNCTION__ + h->named() );
                   }, h ).detach();
        }

        sleep(dis(gen));
        if( auto o = w.lock() )
            auditor( o->named() + "#Weaked" );
        else
            auditor( "Weak Base Reference grounded");

        sleep(1000);
    }

    {
        cout << std::endl;
        auditor("Shared from Embedded rationale ::" );

        using SharedHousePtr = std::shared_ptr<class SharedHouse>;
        class SharedHouse : public House, public std::enable_shared_from_this<SharedHouse>
        {
        public:
            SharedHouse( std::string name = "House") : House( name + "#Shared.Derivated" ){ }
            ~SharedHouse(){}
            SharedHousePtr getptr() { return shared_from_this(); }
        };

        SharedHousePtr root = std::make_shared<SharedHouse>();
        const string & name = root->named();

        thread( [=](){
            sleep(100);
            auditor( name + "#" + __FUNCTION__ + ".threaded : touched by " + std::to_string( root.use_count() ) );
        }).detach();

        auditor( name + " : touched by " + std::to_string( root.use_count() ) );
        sleep(2000);
        auditor( name + " : afterwhile touched by " + std::to_string( root.use_count() ) );

    }
}

int main()
{

    PointersStuff();
}

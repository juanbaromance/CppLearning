#include <iostream>
#include <string>

class Name
{
public:
	Name(const std::string &firstName, const std::string &secondName) : 
	firstName_(std::move(firstName)), secondName_(std::move(secondName) ) {}

	void dump()
	{
		( std::cout << __PRETTY_FUNCTION__ << " : " << firstName_ << "." << secondName_ << std::endl ).flush(); 
	}

private:
	std::string firstName_;
	std::string secondName_;
};



template <typename T>
struct RepeatPrint : T {
public:
	explicit RepeatPrint(T const &t) : T(t) {}
	void repeat(unsigned int n )
	{
		while( n-- >  0 )
        this->dump();
	}
};

#include <tuple>
#include <array>
#include <map>
int main()
{
	Name pp("Pepito", "Perez");
	pp.dump();

	RepeatPrint<Name>(pp).repeat(3);

    // one line coding smell(lack of)
    // c++17 binding capabilites
    using namespace std;

    auto t = make_tuple(1.0, "Ferchi");
    auto & [ number, name ] = t;
    cout << __PRETTY_FUNCTION__ << " : " << name << "." << number << endl;

    array<int,3> v = {1,2,3};
    auto & [ x, y, z ] = v;
    cout << __PRETTY_FUNCTION__ << "(" << x << " " << y << " " << z << ")" << endl;

    int constexpr  index = 00;
    auto m = map<int,const char*>( { {0,"Fulanito"}, {1,"Menganito"} } );
    if( auto i = m.find(index); i == m.end() )
    {
        cout << "Missed key(" << index << ")" << endl;
    }
    else
    {
        cout << "Got " << i->second << endl;
    }
}

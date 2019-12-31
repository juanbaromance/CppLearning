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


int main()
{
	Name pp("Pepito", "Perez");
	pp.dump();

	RepeatPrint<Name>(pp).repeat(3);
}

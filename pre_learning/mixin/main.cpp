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

int main()
{
	Name n("Pepito", "Perez");
	n.dump();
}
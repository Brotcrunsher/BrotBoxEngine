#include "BBE/UtilTest.h"

std::ostream& bbe::test::operator<<(std::ostream& ostrm, const bbe::test::Person& p)
{
	return ostrm
		<< "age: " << p.age
		<< " address: " << p.address
		<< " name: " << p.name
		<< " destructed: " << p.destructed;
}

		

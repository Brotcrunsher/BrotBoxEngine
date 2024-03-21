#include "Tests/AllTests.h"

int main()
{
	bbe::test::runAllTests();

	bbe::INTERNAL::allocCleanup();
    return 0;
}


#include "Tests/AllTests.h"

int main()
{
	bbe::test::runAllTests();

	bbe::INTERNAL::alloc_cleanup();
    return 0;
}


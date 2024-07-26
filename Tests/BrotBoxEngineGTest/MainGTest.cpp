#include "gtest/gtest.h"
#include "BBE/AllocBlock.h"

class MyEnvironment : public ::testing::Environment {
public:
    void TearDown() override {
		bbe::INTERNAL::allocCleanup();
    }
};

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::GTEST_FLAG(filter) = "**";
    ::testing::AddGlobalTestEnvironment(new MyEnvironment);
	
	return RUN_ALL_TESTS();
}

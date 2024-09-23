#include <gtest/gtest.h>
#include "../BBE/StableList.h"
#include <string>


namespace bbe {
    class Trackable {
    public:
        static inline int constructorCount = 0;
        static inline int destructorCount = 0;

        int value;

        Trackable(int val = 0) : value(val) { ++constructorCount; }
        Trackable(const Trackable& other) : value(other.value) { ++constructorCount; }
        Trackable(Trackable&& other) noexcept : value(other.value) { ++constructorCount; }
        ~Trackable() { ++destructorCount; }

        Trackable& operator=(const Trackable& other) {
            if (this != &other) {
                value = other.value;
            }
            return *this;
        }

        Trackable& operator=(Trackable&& other) noexcept {
            if (this != &other) {
                value = other.value;
            }
            return *this;
        }

        bool operator==(const Trackable& other) const {
            return value == other.value;
        }
    };
}

class StableListTest : public ::testing::Test {
protected:
    void SetUp() override {
        bbe::Trackable::constructorCount = 0;
        bbe::Trackable::destructorCount = 0;
    }

    void TearDown() override {
        ASSERT_EQ(bbe::Trackable::constructorCount, bbe::Trackable::destructorCount);
    }
};

TEST_F(StableListTest, AddElementsCopy) {
    bbe::StableList<int> list;

    list.add(10);
    list.add(20);
    list.add(30);

    EXPECT_EQ(*list.begin(), 10);
    auto it = list.begin();
    ++it;
    EXPECT_EQ(*it, 20);
    ++it;
    EXPECT_EQ(*it, 30);
    ++it;
    EXPECT_EQ(it, list.end());
}

// Test adding elements using move semantics
TEST_F(StableListTest, AddElementsMove) {
    bbe::StableList<std::string> list;

    std::string str1 = "Hello";
    std::string str2 = "World";

    list.add(std::move(str1));
    list.add(std::move(str2));

    EXPECT_EQ(*list.begin(), "Hello");
    auto it = list.begin();
    ++it;
    EXPECT_EQ(*it, "World");
    ++it;
    EXPECT_EQ(it, list.end());

    // Ensure original strings have been moved from
    EXPECT_TRUE(str1.empty());
    EXPECT_TRUE(str2.empty());
}

// Test iteration using Iterator
TEST_F(StableListTest, IterateUsingIterator) {
    bbe::StableList<int> list;

    for (int i = 1; i <= 5; ++i) {
        list.add(i * 10);
    }

    int expected = 10;
    for (auto it = list.begin(); it != list.end(); ++it, expected += 10) {
        EXPECT_EQ(*it, expected);
    }
}

// Test iteration using ConstIterator
TEST_F(StableListTest, IterateUsingConstIterator) {
    const bbe::StableList<int> list = []() {
        bbe::StableList<int> temp;
        for (int i = 1; i <= 5; ++i) {
            temp.add(i * 100);
        }
        return temp;
        }();

    int expected = 100;
    for (auto it = list.begin(); it != list.end(); ++it, expected += 100) {
        EXPECT_EQ(*it, expected);
    }
}

// Test range-based for loop with Iterator
TEST_F(StableListTest, RangeBasedForLoopIterator) {
    bbe::StableList<int> list;

    for (int i = 1; i <= 3; ++i) {
        list.add(i);
    }

    int expected = 1;
    for (const auto& value : list) {
        EXPECT_EQ(value, expected++);
    }
}

// Test removing elements using Iterator::remove
TEST_F(StableListTest, RemoveElements) {
    bbe::StableList<int> list;

    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);
    list.add(5);

    // Remove the third element (value 3)
    auto it = list.begin();
    ++it;
    ++it; // Points to 3
    it.remove();

    // Expected list: 1, 2, 4, 5
    std::vector<int> expected = { 1, 2, 4, 5 };
    std::vector<int> actual;
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }

    EXPECT_EQ(actual, expected);
}

// Test removing first and last elements
TEST_F(StableListTest, RemoveFirstAndLastElements) {
    bbe::StableList<int> list;

    list.add(100);
    list.add(200);
    list.add(300);

    // Remove first element
    auto it = list.begin();
    it.remove();

    // Remove last element
    it = list.begin();
    ++it; // Now points to 300
    it.remove();

    // Expected list: 200
    std::vector<int> expected = { 200 };
    std::vector<int> actual;
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }

    EXPECT_EQ(actual, expected);
}

// Test removing elements using Trackable class to verify destructor calls
TEST_F(StableListTest, RemoveElementsTrackable) {
    bbe::StableList<bbe::Trackable> list;

    // Pre-construct Trackable objects
    bbe::Trackable t1(1);
    bbe::Trackable t2(2);
    bbe::Trackable t3(3);

    // Add them to the list using copy semantics
    list.add(t1);
    list.add(t2);
    list.add(t3);

    // At this point:
    // - 3 constructors (for t1, t2, t3)
    // - 3 copy constructors (from list.add)
    // - No destructors yet (temporaries are not used)

    EXPECT_EQ(bbe::Trackable::constructorCount, 6); // 3 original + 3 copies
    EXPECT_EQ(bbe::Trackable::destructorCount, 0);

    // Remove the second element (t2)
    auto it = list.begin();
    ++it; // Points to t2
    it.remove();

    // After removal:
    // - 1 destructor (for t2)

    EXPECT_EQ(bbe::Trackable::destructorCount, 1);

    // Clear the list, which should destruct the remaining elements (t1 and t3)
    list.clear();

    // After clearing:
    // - 2 destructors (for t1 and t3)

    EXPECT_EQ(bbe::Trackable::destructorCount, 3);
}


// Test copy constructor
TEST_F(StableListTest, CopyConstructor) {
    bbe::StableList<int> original;
    original.add(10);
    original.add(20);
    original.add(30);

    bbe::StableList<int> copy = original;

    // Verify elements in the copy
    std::vector<int> expected = { 10, 20, 30 };
    std::vector<int> actual;
    for (auto it = copy.begin(); it != copy.end(); ++it) {
        actual.push_back(*it);
    }

    EXPECT_EQ(actual, expected);

    // Modify original and ensure copy remains unchanged
    original.add(40);
    expected = { 10, 20, 30 };
    actual.clear();
    for (auto it = copy.begin(); it != copy.end(); ++it) {
        actual.push_back(*it);
    }

    EXPECT_EQ(actual, expected);
}

// Test copy assignment
TEST_F(StableListTest, CopyAssignment) {
    bbe::StableList<int> original;
    original.add(5);
    original.add(15);

    bbe::StableList<int> copy;
    copy = original;

    // Verify elements in the copy
    std::vector<int> expected = { 5, 15 };
    std::vector<int> actual;
    for (auto it = copy.begin(); it != copy.end(); ++it) {
        actual.push_back(*it);
    }

    EXPECT_EQ(actual, expected);

    // Modify original and ensure copy remains unchanged
    original.add(25);
    expected = { 5, 15 };
    actual.clear();
    for (auto it = copy.begin(); it != copy.end(); ++it) {
        actual.push_back(*it);
    }

    EXPECT_EQ(actual, expected);
}

// Test move constructor
TEST_F(StableListTest, MoveConstructor) {
    bbe::StableList<int> original;
    original.add(100);
    original.add(200);

    bbe::StableList<int> moved = std::move(original);

    // Original should be empty
    EXPECT_EQ(original.begin(), original.end());

    // Moved list should contain the elements
    std::vector<int> expected = { 100, 200 };
    std::vector<int> actual;
    for (auto it = moved.begin(); it != moved.end(); ++it) {
        actual.push_back(*it);
    }

    EXPECT_EQ(actual, expected);
}

// Test move assignment
TEST_F(StableListTest, MoveAssignment) {
    bbe::StableList<int> original;
    original.add(50);
    original.add(60);

    bbe::StableList<int> moved;
    moved = std::move(original);

    // Original should be empty
    EXPECT_EQ(original.begin(), original.end());

    // Moved list should contain the elements
    std::vector<int> expected = { 50, 60 };
    std::vector<int> actual;
    for (auto it = moved.begin(); it != moved.end(); ++it) {
        actual.push_back(*it);
    }

    EXPECT_EQ(actual, expected);
}

// Test pointer stability
TEST_F(StableListTest, PointerStability) {
    bbe::StableList<int*> list;

    int a = 1, b = 2, c = 3;
    list.add(&a);
    list.add(&b);
    list.add(&c);

    // Store pointers
    int* ptrA = *list.begin();
    auto it = list.begin();
    ++it;
    int* ptrB = *it;
    ++it;
    int* ptrC = *it;

    // Add more elements
    int d = 4;
    list.add(&d);

    // Ensure original pointers are still valid
    EXPECT_EQ(*ptrA, 1);
    EXPECT_EQ(*ptrB, 2);
    EXPECT_EQ(*ptrC, 3);

    // Remove an element and check pointer stability
    it = list.begin();
    it.remove(); // Remove 'a'

    EXPECT_EQ(**it, 2);
    EXPECT_EQ(*ptrB, 2);
    EXPECT_EQ(*ptrC, 3);
}

// Test behavior with an empty list
TEST_F(StableListTest, EmptyList) {
    bbe::StableList<int> list;

    EXPECT_EQ(list.begin(), list.end());

    // Attempt to remove using Iterator (should do nothing)
    auto it = list.begin();
    it.remove(); // Should not crash or throw

    EXPECT_EQ(list.begin(), list.end());
}

// Test adding and removing elements across block boundaries
TEST_F(StableListTest, BlockBoundary) {
    constexpr size_t blockSize = 4; // For testing purposes, use a small block size
    using TestList = bbe::StableList<int, blockSize>;
    TestList list;

    // Add elements to fill the first block
    for (int i = 1; i <= blockSize; ++i) {
        list.add(i);
    }

    // Add elements to create a new block
    list.add(blockSize + 1);
    list.add(blockSize + 2);

    // Verify all elements
    std::vector<int> expected = { 1, 2, 3, 4, 5, 6 };
    std::vector<int> actual;
    for (auto it = list.begin(); it != list.end(); ++it) {
        actual.push_back(*it);
    }
    EXPECT_EQ(actual, expected);

    // Remove elements from both blocks
    auto it = list.begin();
    it.remove(); // Remove 1
    ++it; ++it; ++it; // Points to 5
    it.remove(); // Remove 5

    // Expected list: 2, 3, 4, 6
    expected = { 2, 3, 4, 6 };
    actual.clear();
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);

    // Add more elements to fill freed slots
    list.add(7);
    list.add(8);

    // Expected list: 2, 3, 4, 6, 7, 8
    expected = { 2, 3, 4, 6, 7, 8 };
    actual.clear();
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);
}

// Test clear function
TEST_F(StableListTest, ClearFunction) {
    bbe::StableList<int> list;

    list.add(100);
    list.add(200);
    list.add(300);

    EXPECT_NE(list.begin(), list.end());

    list.clear();

    EXPECT_EQ(list.begin(), list.end());

    // Ensure no elements remain
    std::vector<int> actual;
    for (auto it = list.begin(); it != list.end(); ++it) {
        actual.push_back(*it);
    }
    EXPECT_TRUE(actual.empty());
}

// Test adding after removals to ensure slots are reused
TEST_F(StableListTest, AddAfterRemove) {
    bbe::StableList<int> list;

    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);

    // Remove second and third elements
    auto it = list.begin();
    ++it;
    it.remove(); // Remove 2
    ++it;
    it.remove(); // Remove 4

    // Expected list: 1, 3
    std::vector<int> expected = { 1, 3 };
    std::vector<int> actual;
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);

    // Add new elements, should reuse freed slots
    list.add(5);
    list.add(6);

    // Expected list: 1, 3, 5, 6
    expected = { 1, 3, 5, 6 };
    actual.clear();
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);
}

// Test multiple removals and additions
TEST_F(StableListTest, MultipleRemovalsAndAdditions) {
    bbe::StableList<int> list;

    // Add 10 elements
    for (int i = 1; i <= 10; ++i) {
        list.add(i);
    }

    // Remove even numbers
    for (auto it = list.begin(); it != list.end(); ) {
        if (*it % 2 == 0) {
            it.remove();
        }
        else {
            ++it;
        }
    }

    // Expected list: 1, 3, 5, 7, 9
    std::vector<int> expected = { 1, 3, 5, 7, 9 };
    std::vector<int> actual;
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);

    // Add more elements
    for (int i = 11; i <= 15; ++i) {
        list.add(i);
    }

    // Expected list: 1, 3, 5, 7, 9, 11, 12, 13, 14, 15
    expected = { 1, 3, 5, 7, 9, 11, 12, 13, 14, 15 };
    actual.clear();
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);
}

// Test adding and removing objects to ensure proper constructor/destructor calls
TEST_F(StableListTest, ConstructorDestructorTracking) {
    bbe::StableList<bbe::Trackable> list;

    list.add(bbe::Trackable(10));
    list.add(bbe::Trackable(20));
    list.add(bbe::Trackable(30));

    EXPECT_EQ(bbe::Trackable::constructorCount, 3);
    EXPECT_EQ(bbe::Trackable::destructorCount, 0);

    // Remove the second element
    auto it = list.begin();
    ++it;
    it.remove();

    EXPECT_EQ(bbe::Trackable::destructorCount, 1);

    // Add another element
    list.add(bbe::Trackable(40));
    EXPECT_EQ(bbe::Trackable::constructorCount, 4);

    // Clear the list
    list.clear();
    EXPECT_EQ(bbe::Trackable::destructorCount, 4);
}

// Test adding nullptrs in StableList<int*> and removing them
TEST_F(StableListTest, AddAndRemoveNullptrs) {
    bbe::StableList<int*> list;

    list.add(nullptr);
    int a = 5;
    list.add(&a);
    list.add(nullptr);

    // Verify elements
    auto it = list.begin();
    EXPECT_EQ(*it, nullptr);
    ++it;
    EXPECT_EQ(**it, 5);
    ++it;
    EXPECT_EQ(*it, nullptr);

    // Remove first nullptr
    it = list.begin();
    it.remove();

    // Expected list: nullptr, 5
    std::vector<int*> expected = { nullptr, &a };
    std::vector<int*> actual;
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);

    // Remove second element (5)
    it = list.begin();
    ++it;
    it.remove();

    // Expected list: nullptr
    expected = { nullptr };
    actual.clear();
    for (auto itr = list.begin(); itr != list.end(); ++itr) {
        actual.push_back(*itr);
    }
    EXPECT_EQ(actual, expected);
}


//
// Created by Martin Blicha on 24.07.18.
//

#include <gtest/gtest.h>
#include <utils/string_utils.h>

TEST(Split_test, test_Single)
{
    auto res = splitString("one", ' ');
    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0], "one");
}

TEST(Split_test, test_Multiple)
{
    auto res = splitString("one two three", ' ');
    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[2], "three");
}
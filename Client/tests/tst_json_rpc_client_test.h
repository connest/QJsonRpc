#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

TEST(Json_RPC_Clent, Json_RPC_Client_test)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}


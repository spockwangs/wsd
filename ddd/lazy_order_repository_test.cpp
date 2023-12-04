// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include <gtest/gtest.h>

#include "lazy_order_dao_db_impl.h"
#include "lazy_order_repository_impl.h"
#include "repository.h"

class LazyOrderRepositoryTest : public testing::Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        dao_.ResetForTesting();
    }

protected:
    std::unique_ptr<ddd::infra::LazyOrderRepositoryImpl> MakeRepository()
    {
        return std::make_unique<ddd::infra::LazyOrderRepositoryImpl>(dao_);
    }

    ddd::infra::LazyOrderDaoDbImpl dao_;
};

TEST_F(LazyOrderRepositoryTest, SaveAndFind)
{
    auto repo_ptr = MakeRepository();
    auto& repo = *repo_ptr;

    // Finding an non-existent entity will return nullptr.
    auto status_or_order_ptr = repo.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order_ptr.status()));

    // Add an enity.
    auto order_ptr = repo.AddOrder(ddd::domain::LazyOrder{repo, "a"});
    EXPECT_FALSE(order_ptr.expired());
    EXPECT_EQ(order_ptr.lock()->GetId(), "a");

    // ... and we will find it.
    status_or_order_ptr = repo.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order_p1 = status_or_order_ptr.value();
    EXPECT_FALSE(order_p1.expired());
    EXPECT_EQ(order_p1.lock(), order_ptr.lock());
    EXPECT_EQ(order_p1.lock()->GetId(), "a");
    order_p1.lock()->AddLineItem("apple", 10);

    // If we find it again, it will return the same object.
    status_or_order_ptr = repo.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order_p2 = status_or_order_ptr.value();
    EXPECT_EQ(order_p2.lock(), order_p1.lock());
    EXPECT_EQ(order_p2.lock()->GetTotalPrice(), 10);

    // Commit it.
    EXPECT_TRUE(repo.Commit().ok());

    // In another session, find the order and check it.
    repo_ptr = MakeRepository();
    auto& repo2 = *repo_ptr;
    status_or_order_ptr = repo2.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order = status_or_order_ptr.value().lock();
    EXPECT_EQ(order->GetTotalPrice(), 10);

    // Add a line item.
    order->AddLineItem("banana", 5);
    EXPECT_TRUE(repo2.Commit().ok());

    // In the third session.
    repo_ptr = MakeRepository();
    auto& repo3 = *repo_ptr;
    status_or_order_ptr = repo3.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    order = status_or_order_ptr.value().lock();
    EXPECT_EQ(order->GetTotalPrice(), 15);
    std::string item_id;
    for (const auto& line_item_ptr : order->GetLineItems()) {
        EXPECT_TRUE(!line_item_ptr.expired());
        auto line_item = line_item_ptr.lock();
        if (line_item->GetName() == "banana") {
            item_id = line_item->GetItemId();
            break;
        }
    }
    EXPECT_TRUE(!item_id.empty());
    order->RemoveLineItem(item_id);
    order->AddLineItem("pineapple", 20);
    EXPECT_TRUE(repo3.Commit().ok());

    // In the fourth session.
    repo_ptr = MakeRepository();
    auto& repo4 = *repo_ptr;
    status_or_order_ptr = repo4.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    order = status_or_order_ptr.value().lock();
    EXPECT_EQ(order->GetTotalPrice(), 30);
    EXPECT_EQ(order->GetLineItems().size(), 2);
}

TEST_F(LazyOrderRepositoryTest, ConcurrencyControl)
{
    auto repo_ptr_1 = MakeRepository();
    auto repo_ptr_2 = MakeRepository();
    auto& repo1 = *repo_ptr_1;
    auto& repo2 = *repo_ptr_2;

    // Session 1: find an entity.
    auto status_or_order_ptr = repo1.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order_ptr.status()));

    // Session 2: find the same entity.
    status_or_order_ptr = repo2.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order_ptr.status()));

    // Session 1: add an entity.
    repo1.AddOrder(ddd::domain::LazyOrder{repo1, "a"});
    EXPECT_TRUE(repo1.Commit().ok());

    // Session 2: add the same entity, and we will get conflicts.
    repo2.AddOrder(ddd::domain::LazyOrder{repo1, "a"});
    auto s = repo2.Commit();
    EXPECT_TRUE(absl::IsAborted(s)) << s.ToString();
}

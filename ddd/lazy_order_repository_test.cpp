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
    auto status_or_order = repo.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order.status()));

    // Add an enity.
    auto* order = repo.AddOrder(ddd::domain::LazyOrder{repo, "a"});
    EXPECT_EQ(order->GetId(), "a");

    // ... and we will find it.
    status_or_order = repo.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto* order_1 = status_or_order.value();
    EXPECT_EQ(order_1, order);
    EXPECT_EQ(order_1->GetId(), "a");
    auto s = order_1->AddLineItem("apple", 10);
    EXPECT_TRUE(s.ok());

    // If we find it again, it will return the same object.
    status_or_order = repo.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto* order_2 = status_or_order.value();
    EXPECT_EQ(order_2, order_1);
    EXPECT_EQ(order_2->GetTotalPrice(), 10);

    // Commit it.
    EXPECT_TRUE(repo.Commit().ok());

    // In another session, find the order and check it.
    repo_ptr = MakeRepository();
    auto& repo2 = *repo_ptr;
    status_or_order = repo2.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto* order_3 = status_or_order.value();
    EXPECT_EQ(order_3->GetTotalPrice(), 10);

    // Add a line item.
    s = order->AddLineItem("banana", 5);
    EXPECT_TRUE(s.ok());
    EXPECT_TRUE(repo2.Commit().ok());

    // In the third session.
    repo_ptr = MakeRepository();
    auto& repo3 = *repo_ptr;
    status_or_order = repo3.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto* order_4 = status_or_order.value();
    EXPECT_EQ(order_4->GetTotalPrice(), 15);
    std::string item_id;
    std::vector<ddd::domain::LineItem*> line_items;
    s = order_4->GetLineItems(&line_items);
    EXPECT_TRUE(s.ok());
    for (auto* line_item : line_items) {
        if (line_item->GetName() == "banana") {
            item_id = line_item->GetItemId();
            break;
        }
    }
    EXPECT_TRUE(!item_id.empty());
    s = order_4->RemoveLineItem(item_id);
    s.Update(order_4->AddLineItem("pineapple", 20));
    EXPECT_TRUE(s.ok());
    EXPECT_TRUE(repo3.Commit().ok());

    // In the fourth session.
    repo_ptr = MakeRepository();
    auto& repo4 = *repo_ptr;
    status_or_order = repo4.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto* order_5 = status_or_order.value();
    EXPECT_EQ(order_5->GetTotalPrice(), 30);
    line_items.clear();
    s = order_5->GetLineItems(&line_items);
    EXPECT_TRUE(s.ok());
    EXPECT_EQ(line_items.size(), 2);
}

TEST_F(LazyOrderRepositoryTest, ConcurrencyControl)
{
    auto repo_ptr_1 = MakeRepository();
    auto repo_ptr_2 = MakeRepository();
    auto& repo1 = *repo_ptr_1;
    auto& repo2 = *repo_ptr_2;

    // Session 1: find an entity.
    auto status_or_order = repo1.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order.status()));

    // Session 2: find the same entity.
    status_or_order = repo2.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order.status()));

    // Session 1: add an entity.
    repo1.AddOrder(ddd::domain::LazyOrder{repo1, "a"});
    EXPECT_TRUE(repo1.Commit().ok());

    // Session 2: add the same entity, and we will get conflicts.
    repo2.AddOrder(ddd::domain::LazyOrder{repo1, "a"});
    auto s = repo2.Commit();
    EXPECT_TRUE(absl::IsAborted(s)) << s.ToString();
}

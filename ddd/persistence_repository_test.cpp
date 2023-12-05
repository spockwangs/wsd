// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include <gtest/gtest.h>

#include "order_dao_kv_impl.h"
#include "persistence_repository_impl.h"
#include "repository.h"

class PersistenceRepositoryTest : public testing::Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        dao_.Reset();
    }

protected:
    std::unique_ptr<ddd::domain::PersistenceRepository<std::string, ddd::domain::Order>> MakeRepository()
    {
        return std::make_unique<ddd::infra::PersistenceRepositoryImpl<std::string, ddd::domain::Order>>(dao_);
    }

    ddd::infra::OrderDaoKvImpl dao_;
};

TEST_F(PersistenceRepositoryTest, SaveAndFind)
{
    auto repo_ptr = MakeRepository();
    auto& repo = *repo_ptr;

    // Finding an non-existent entity will return nullptr.
    auto status_or_order_ptr = repo.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order_ptr.status()));

    // Add an enity.
    ddd::domain::Order newOrder{"a"};
    auto s = repo.Save(newOrder);
    EXPECT_TRUE(s.ok()) << s.ToString();

    // ... and we will find it.
    status_or_order_ptr = repo.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order_p1 = status_or_order_ptr.value();
    EXPECT_FALSE(order_p1.expired());
    EXPECT_EQ(order_p1.lock()->GetId(), "a");
    order_p1.lock()->AddLineItem("apple", 10);

    // If we find it again, it will return the same object.
    status_or_order_ptr = repo.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order_p2 = status_or_order_ptr.value();
    EXPECT_EQ(order_p2.lock(), order_p1.lock());
    EXPECT_EQ(order_p2.lock()->GetTotalPrice(), 10);
}

TEST_F(PersistenceRepositoryTest, MultiSessionConflicts)
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

    // Session 1: add and save an entity.
    ddd::domain::Order newOrder{"a"};
    auto s = repo1.Save(newOrder);
    EXPECT_TRUE(s.ok());

    // Session 2: save the same entity, and we will get conflicts.
    ddd::domain::Order newOrder2{"a"};
    s = repo2.Save(newOrder2);
    EXPECT_TRUE(absl::IsAborted(s)) << s.ToString();
}

TEST_F(PersistenceRepositoryTest, MultiSessionConflicts2)
{
    auto repo_ptr_1 = MakeRepository();
    auto repo_ptr_2 = MakeRepository();
    auto& repo1 = *repo_ptr_1;
    auto& repo2 = *repo_ptr_2;

    EXPECT_TRUE(repo1.Save(ddd::domain::Order{"a"}).ok());

    // Session 1: find an entity.
    auto status_or_order_ptr = repo1.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order1 = status_or_order_ptr.value();

    // Session 2: find the same entity.
    status_or_order_ptr = repo2.Find("a");
    EXPECT_TRUE(status_or_order_ptr.ok());
    auto order2 = status_or_order_ptr.value();
    EXPECT_EQ(order1.lock()->GetTotalPrice(), order2.lock()->GetTotalPrice());

    // Session 2: remove the entity.
    auto s = repo2.Remove(order2.lock()->GetId());
    EXPECT_TRUE(s.ok());
    EXPECT_TRUE(order2.expired());

    // Session 1: modify and save the entity.
    order1.lock()->AddLineItem("apple", 2);
    s = repo1.Save(*order1.lock());
    EXPECT_TRUE(absl::IsAborted(s));
    EXPECT_TRUE(order1.expired());
}

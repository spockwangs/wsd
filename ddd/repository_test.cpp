// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include <gtest/gtest.h>

#include <mutex>
#include <unordered_map>

#include "ddd/order.pb.h"
#include "entity.h"
#include "repository_impl.h"

class OrderDao : public ddd::Dao<ddd::Order> {
public:
    OrderDao() = default;

    ~OrderDao() override = default;

    absl::StatusOr<std::unique_ptr<ddd::Order>> Get(const std::string& id, std::string* cas_token);

    absl::Status CasPut(const ddd::Order& order, const std::string& cas_token);

    absl::Status Del(const std::string& id);

    void Reset();

private:
    static absl::StatusOr<std::unique_ptr<ddd::Order>> FromOrderPO(const std::string& s);

    static std::string ToOrderPO(const ddd::Order& order);

    std::mutex mu_;
    std::unordered_map<std::string, std::pair<std::string, int>> kv_;
};

absl::StatusOr<std::unique_ptr<ddd::Order>> OrderDao::Get(const std::string& id, std::string* cas_token)
{
    std::unique_lock<std::mutex> l{mu_};
    auto it = kv_.find(id);
    if (it == kv_.end()) {
        return absl::NotFoundError("");
    }

    *cas_token = std::to_string(it->second.second);
    return FromOrderPO(it->second.first);
}

absl::Status OrderDao::CasPut(const ddd::Order& order, const std::string& cas_token)
{
    int version = 0;
    if (!cas_token.empty() && !absl::SimpleAtoi(cas_token, &version)) {
        return absl::InvalidArgumentError("invalid cas_token");
    }

    int exp_version = 0;
    std::unique_lock<std::mutex> l{mu_};
    auto it = kv_.find(order.GetId());
    if (it == kv_.end()) {
        exp_version = 0;
    } else {
        exp_version = it->second.second;
    }
    if (version != exp_version) {
        return absl::AbortedError("");
    }
    kv_[order.GetId()] = std::make_pair(ToOrderPO(order), ++version);
    return absl::OkStatus();
}

absl::Status OrderDao::Del(const std::string& id)
{
    std::unique_lock<std::mutex> l{mu_};
    kv_.erase(id);
    return absl::OkStatus();
}

void OrderDao::Reset()
{
    kv_.clear();
}

absl::StatusOr<std::unique_ptr<ddd::Order>> OrderDao::FromOrderPO(const std::string& s)
{
    ddd_po::Order order_po;
    if (!order_po.ParseFromString(s)) {
        return absl::DataLossError("ParseFromString() failed");
    }
    return std::make_unique<ddd::Order>(order_po.id(), order_po.price());
}

std::string OrderDao::ToOrderPO(const ddd::Order& order)
{
    ddd_po::Order order_po;
    order_po.set_id(order.GetId());
    order_po.set_price(order.GetPrice());
    std::string result;
    order_po.SerializeToString(&result);
    return result;
}

class RepositoryTest : public testing::Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        dao_.Reset();
    }

protected:
    OrderDao dao_;
};

TEST_F(RepositoryTest, SaveAndFind)
{
    ddd::RepositoryImpl<ddd::Order> repo{dao_};

    // Finding an non-existent entity will return nullptr.
    absl::StatusOr<ddd::Order*> status_or_order = repo.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order.status()));

    // Add an enity.
    ddd::Order newOrder{"a", 1};
    auto s = repo.Save(newOrder);
    EXPECT_TRUE(s.ok()) << s.ToString();

    // ... and we will find it.
    status_or_order = repo.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto order_p1 = status_or_order.value();
    EXPECT_TRUE(order_p1 != nullptr);
    EXPECT_EQ(order_p1->GetId(), "a");
    EXPECT_EQ(order_p1->GetPrice(), 1);
    order_p1->SetPrice(2);

    // If we find it again, it will return the same object.
    status_or_order = repo.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    auto order_p2 = status_or_order.value();
    EXPECT_EQ(order_p2, order_p1);
    EXPECT_EQ(order_p2->GetPrice(), 2);
}

TEST_F(RepositoryTest, MultiSessionConflicts)
{
    ddd::RepositoryImpl<ddd::Order> repo1{dao_}, repo2{dao_};

    // Session 1: find an entity.
    absl::StatusOr<ddd::Order*> status_or_order = repo1.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order.status()));

    // Session 2: find the same entity.
    status_or_order = repo2.Find("a");
    EXPECT_TRUE(absl::IsNotFound(status_or_order.status()));

    // Session 1: add and save an entity.
    ddd::Order newOrder{"a", 1};
    auto s = repo1.Save(newOrder);
    EXPECT_TRUE(s.ok());

    // Session 2: save the same entity, and we will get conflicts.
    ddd::Order newOrder2{"a", 2};
    s = repo2.Save(newOrder2);
    EXPECT_TRUE(absl::IsAborted(s)) << s.ToString();
}

TEST_F(RepositoryTest, MultiSessionConflicts2)
{
    ddd::RepositoryImpl<ddd::Order> repo1{dao_}, repo2{dao_};
    EXPECT_TRUE(repo1.Save(ddd::Order{"a", 1}).ok());

    // Session 1: find an entity.
    absl::StatusOr<ddd::Order*> status_or_order = repo1.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    ddd::Order* order1 = status_or_order.value();

    // Session 2: find the same entity.
    status_or_order = repo2.Find("a");
    EXPECT_TRUE(status_or_order.ok());
    ddd::Order* order2 = status_or_order.value();
    EXPECT_EQ(order1->GetPrice(), order2->GetPrice());

    // Session 2: remove the entity.
    auto s = repo2.Remove(order2->GetId());
    EXPECT_TRUE(s.ok());

    // Session 1: modify and save the entity.
    order1->SetPrice(2);
    s = repo1.Save(*order1);
    EXPECT_TRUE(absl::IsAborted(s));
}

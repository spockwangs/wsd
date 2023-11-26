#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

class Entity {
public:
    Entity() = default;
    
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    void operator=(const Entity&) = delete;

    virtual std::string GetId() const = 0;
};

template <typename Entity>
class Repository {
public:
    Repository() = default;
    
    virtual ~Repository() = default;

    virtual Entity* Find(const std::string& id) = 0;

    virtual void Remove(const Entity& entity) = 0;

    virtual bool Save(const Entity& entity) = 0;
};


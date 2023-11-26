#pragma once

class ID {
public:
    virtual ~ID() {}
};

template <typename Entity>
class Repository<Entity> {
public:
    virtual ~Repository() {}

    virtual Entity* Find(ID id) = 0;

    virtual void Remove(const Entity& entity) = 0;

    virtual bool Save(const Entity& entity) = 0;
};

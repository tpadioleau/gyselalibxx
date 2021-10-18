#pragma once

#include <memory>

#include <geometry.hpp>

class IInterpolatorVx
{
public:
    virtual ~IInterpolatorVx() = default;

    virtual void operator()(DSpanVx const inout_data, DViewVx const coordinates) const = 0;
};

class InterpolatorVxProxy : public IInterpolatorVx
{
    std::unique_ptr<IInterpolatorVx> m_impl;

public:
    InterpolatorVxProxy(std::unique_ptr<IInterpolatorVx>&& impl) : m_impl(std::move(impl)) {}

    virtual void operator()(DSpanVx const inout_data, DViewVx const coordinates) const override
    {
        return (*m_impl)(inout_data, coordinates);
    }
};

class IPreallocatableInterpolatorVx : public IInterpolatorVx
{
public:
    virtual InterpolatorVxProxy preallocate() const = 0;
};

#pragma once

#include "driver/gpio/interface.h"

namespace driver
{
namespace gpio
{
class Stub final : public Interface
{
public:
    Stub(Direction direction = Direction::Input) noexcept
        : myInitialized{true}
        , myDirection{direction}
        , myValue{false}
        , myInterruptEnabled{true}
        , myInterruptOnPortEnabled{true}
    {}

    ~Stub() noexcept override = default;

    bool isInitialized() const noexcept override { return myInitialized; }

    Direction direction() const noexcept override { return myDirection; }

    bool read() const noexcept override { return myValue; }

    void write(bool output) noexcept override { myValue = output; }

    void toggle() noexcept override { myValue = !myValue; }

    void enableInterrupt(bool enable) noexcept override { myInterruptEnabled = enable; }

    void enableInterruptOnPort(bool enable) noexcept override
    {
        myInterruptOnPortEnabled = enable;
    }

    bool isInterruptEnabled() const noexcept { return myInterruptEnabled; }

    bool isInterruptOnPortEnabled() const noexcept { return myInterruptOnPortEnabled; }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

    void setDirection(Direction direction) noexcept { myDirection = direction; }

private:
    bool myInitialized;
    Direction myDirection;
    bool myValue;
    bool myInterruptEnabled;
    bool myInterruptOnPortEnabled;
};
} // namespace gpio
} // namespace driver
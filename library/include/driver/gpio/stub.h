#pragma once

#include "driver/gpio/interface.h"

namespace driver
{
namespace gpio
{
class Stub final : public Interface
{
public:
    explicit Stub(const Direction direction = Direction::Input) noexcept
        : myInitialized{true}
        , myDirection{direction}
        , myValue{false}
        , myInterruptEnabled{true}
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

    bool isInterruptEnabled() const noexcept
    {
        return myInterruptEnabled && myInterruptOnPortEnabled;
    }

    bool isInterruptOnPortEnabled() const noexcept { return myInterruptOnPortEnabled; }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

private:
    bool myInitialized;
    const Direction myDirection;
    bool myValue;
    bool myInterruptEnabled;

    inline static bool myInterruptOnPortEnabled{true};
};
} // namespace gpio
} // namespace driver
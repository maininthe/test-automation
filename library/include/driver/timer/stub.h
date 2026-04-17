#pragma once

#include <stdint.h>

#include "driver/timer/interface.h"

namespace driver
{
namespace timer
{
class Stub final : public Interface
{
public:
    Stub(uint32_t timeout_ms = 1000U) noexcept
        : myInitialized{true}
        , myEnabled{false}
        , myTimedOut{false}
        , myTimeout_ms{timeout_ms}
    {}

    ~Stub() noexcept override = default;

    bool isInitialized() const noexcept override { return myInitialized; }

    bool isEnabled() const noexcept override { return myEnabled; }

    bool hasTimedOut() const noexcept override { return myTimedOut; }

    uint32_t timeout_ms() const noexcept override { return myTimeout_ms; }

    void setTimeout_ms(uint32_t timeout_ms) noexcept override
    {
        if (0U != timeout_ms) { myTimeout_ms = timeout_ms; }
    }

    void start() noexcept override { myEnabled = true; }

    void stop() noexcept override { myEnabled = false; }

    void toggle() noexcept override { myEnabled = !myEnabled; }

    void restart() noexcept override
    {
        myTimedOut = false;
        myEnabled = true;
    }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

    void setTimedOut(bool timedOut) noexcept { myTimedOut = timedOut; }

private:
    bool myInitialized;
    bool myEnabled;
    bool myTimedOut;
    uint32_t myTimeout_ms;
};
} // namespace timer
} // namespace driver
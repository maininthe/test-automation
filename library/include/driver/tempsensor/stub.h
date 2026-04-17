#pragma once

#include "driver/tempsensor/interface.h"

namespace driver
{
namespace tempsensor
{
class Stub final : public Interface
{
public:
    Stub() noexcept
        : myInitialized{true}
        , myTemperature_c{0}
    {}

    ~Stub() noexcept override = default;

    bool isInitialized() const noexcept override { return myInitialized; }

    int16_t read() const noexcept override { return myTemperature_c; }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

    void setTemperature(int16_t temperature_c) noexcept { myTemperature_c = temperature_c; }

private:
    bool myInitialized;
    int16_t myTemperature_c;
};
} // namespace tempsensor
} // namespace driver
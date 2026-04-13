/**
 * @brief Unit tests for the Atmega328p GPIO driver.
 */
#include <cstdint>

#include <gtest/gtest.h>

#include "arch/avr/hw_platform.h"
#include "driver/gpio/atmega328p.h"
#include "utils/utils.h"

#ifdef TESTSUITE



namespace driver
{
namespace
{
/**
 * @brief GPIO register structure.
 */
struct GpioRegs
{
    /** Data direction register. */
    volatile std::uint8_t& ddrx;

    /** Port register. */
    volatile std::uint8_t& portx;

    /** Pin register. */
    volatile std::uint8_t& pinx;
};

/**
 * @brief Pin offset structure.
 */
struct PinOffset
{
    /** Pin offset for I/O port D. */
    static constexpr std::uint8_t D{0U};

    /** Pin offset for I/O port B. */
    static constexpr std::uint8_t B{8U};

    /** Pin offset for I/O port C. */
    static constexpr std::uint8_t C{14U};
};

/** Number of available pins. */
constexpr std::uint8_t PinCount{20U};

// -----------------------------------------------------------------------------
constexpr bool isPinValid(const std::uint8_t id) noexcept { return PinCount > id; }

// -----------------------------------------------------------------------------
constexpr std::uint8_t getPhysicalPin(const std::uint8_t id) noexcept
{
    // Return physical pin 0 - 7 on the associated GPIO port.
    if (!isPinValid(id))        { return static_cast<std::uint8_t>(-1); }
    if (PinOffset::B > id)      { return id; } 
    else if (PinOffset::C > id) { return id - PinOffset::B; }
    return id - PinOffset::C;
}

// -----------------------------------------------------------------------------
constexpr void simulateToggle(GpioRegs& regs) noexcept
{
    constexpr std::uint8_t bitCount{8U};

    // Check each pin one by one.
    for (std::uint8_t pin{}; pin < bitCount; ++pin)
    {
        // Toggle the output of a given pin if configured as output and the pin bit has been set.
        if (utils::read(regs.ddrx, pin) && utils::read(regs.pinx, pin))
        {
            utils::toggle(regs.portx, pin);
            utils::clear(regs.pinx, pin);
        }
    }
}

// -----------------------------------------------------------------------------
void runOutputTest(const std::uint8_t id, GpioRegs& regs) noexcept
{
    const std::uint8_t pin{getPhysicalPin(id)};

    {
        gpio::Atmega328p gpio{id, gpio::Direction::Output};

        EXPECT_EQ(gpio.isInitialized(), isPinValid(id));

        if (!isPinValid(id))
        {
            return;
        }

        EXPECT_TRUE(utils::read(regs.ddrx, pin));

        gpio.write(true);
        EXPECT_TRUE(utils::read(regs.portx, pin));

        gpio.write(false);
        EXPECT_FALSE(utils::read(regs.portx, pin));

        gpio.toggle();
        simulateToggle(regs);
        EXPECT_TRUE(utils::read(regs.portx, pin));

        gpio.toggle();
        simulateToggle(regs);
        EXPECT_FALSE(utils::read(regs.portx, pin));

        gpio.toggle();
        simulateToggle(regs);
        EXPECT_TRUE(utils::read(regs.portx, pin));
    }

    if (isPinValid(id))
    {
        EXPECT_FALSE(utils::read(regs.ddrx, pin));
        EXPECT_FALSE(utils::read(regs.portx, pin));
    }
}

// -----------------------------------------------------------------------------
void runInputTest(const std::uint8_t id, GpioRegs& regs) noexcept
{
    const std::uint8_t pin{getPhysicalPin(id)};

    {
       gpio::Atmega328p gpio{id, gpio::Direction::InputPullup};

        EXPECT_EQ(gpio.isInitialized(), isPinValid(id));

        if (!isPinValid(id))
        {
            return;
        }

        EXPECT_FALSE(utils::read(regs.ddrx, pin));
        EXPECT_TRUE(utils::read(regs.portx, pin));

        utils::set(regs.pinx, pin);
        EXPECT_TRUE(gpio.read());

        utils::clear(regs.pinx, pin);
        EXPECT_FALSE(gpio.read());
    }

    if (isPinValid(id))
    {
        EXPECT_FALSE(utils::read(regs.ddrx, pin));
        EXPECT_FALSE(utils::read(regs.portx, pin));
    }
}

/**
 * @brief GPIO initialization test.
 * 
 *        Verify that only one instance per valid pin can be used at once.
 */
TEST(Gpio_Atmega328p, Initialization)
{
    constexpr std::uint8_t pinMax{50U};

    for (std::uint8_t pin{}; pin < pinMax; ++pin)
    {
        gpio::Atmega328p gpio{pin, gpio::Direction::Output};

        EXPECT_EQ(gpio.isInitialized(), isPinValid(pin));

        gpio::Atmega328p duplicate{pin, gpio::Direction::Output};

        if (isPinValid(pin))
        {
            EXPECT_TRUE(gpio.isInitialized());
            EXPECT_FALSE(duplicate.isInitialized());
        }
        else
        {
            EXPECT_FALSE(gpio.isInitialized());
            EXPECT_FALSE(duplicate.isInitialized());
        }
    }
}

/**
 * @brief GPIO output test.
 * 
 *        Verify that GPIO outputs can be used for reading and writing.
 */
TEST(Gpio_Atmega328p, Output)
{
    // Systematically test I/O port D.
    for (std::uint8_t pin{}; pin < PinOffset::B; ++pin)
    {
        GpioRegs regs{DDRD, PORTD, PIND};
        runOutputTest(pin, regs);
    }

    // Systematically test I/O port B.
    for (std::uint8_t pin{PinOffset::B}; pin < PinOffset::C; ++pin)
    {
        GpioRegs regs{DDRB, PORTB, PINB};
        runOutputTest(pin, regs);
    }

    // Systematically test I/O port C.
    for (std::uint8_t pin{PinOffset::C}; pin < PinCount; ++pin)
    {
        GpioRegs regs{DDRC, PORTC, PINC};
        runOutputTest(pin, regs);
    }
}

/**
 * @brief GPIO input test.
 * 
 *        Verify that GPIO input can be used for reading.
 */
TEST(Gpio_Atmega328p, Input)
{
    // Systematically test I/O port D.
    for (std::uint8_t pin{}; pin < PinOffset::B; ++pin)
    {
        GpioRegs regs{DDRD, PORTD, PIND};
        runInputTest(pin, regs);
    }

    // Systematically test I/O port B.
    for (std::uint8_t pin{PinOffset::B}; pin < PinOffset::C; ++pin)
    {
        GpioRegs regs{DDRB, PORTB, PINB};
        runInputTest(pin, regs);
    }

    // Systematically test I/O port C.
    for (std::uint8_t pin{PinOffset::C}; pin < PinCount; ++pin)
    {
        GpioRegs regs{DDRC, PORTC, PINC};
        runInputTest(pin, regs);
    }
}
} // namespace
} // namespace driver



#endif /** TESTSUITE */


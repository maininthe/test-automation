/**
 * @brief Unit tests for the ATmega328p timer driver.
 */
#include <cstdint>

#include <gtest/gtest.h>

#include "arch/avr/hw_platform.h"
#include "driver/timer/atmega328p.h"
#include "utils/utils.h"

#ifdef TESTSUITE


//! @todo Implement tests according to project requirements.
namespace driver
{
namespace
{
/** Maximum number of timer circuits available on ATmega328P. */
constexpr std::uint8_t MaxTimerCount{3U};



/** Flag to track if callback was invoked. */
bool callbackInvoked{false};

// -----------------------------------------------------------------------------
void resetCallbackFlag() noexcept { callbackInvoked = false; }

// -----------------------------------------------------------------------------
void testCallback() noexcept { callbackInvoked = true; }

// -----------------------------------------------------------------------------
constexpr std::uint32_t getMaxCount(const std::uint32_t timeout_ms) noexcept
{
    constexpr double interruptIntervalMs{0.128};
	return 0U < timeout_ms ? 
        utils::round<std::uint32_t>(timeout_ms / interruptIntervalMs) : 0U;
}



/**
 * @brief Timer initialization test.
 * 
 *        Verify that timer circuits are initialized correctly and that 
 *        resource limits are enforced.
 */
TEST(Timer_Atmega328p, Initialization)
{
    {
        timer::Atmega328p t1{100U};
        timer::Atmega328p t2{200U};
        timer::Atmega328p t3{300U};
        timer::Atmega328p t4{400U};

        EXPECT_TRUE(t1.isInitialized());
        EXPECT_TRUE(t2.isInitialized());
        EXPECT_TRUE(t3.isInitialized());
        EXPECT_FALSE(t4.isInitialized());
    }

    {
        timer::Atmega328p valid{100U};
        EXPECT_TRUE(valid.isInitialized());

        timer::Atmega328p invalid{0U};
        EXPECT_FALSE(invalid.isInitialized());
    }
}

/**
 * @brief Timer enable/disable test.
 * 
 *        Verify that timers can be started and stopped correctly.
 */
TEST(Timer_Atmega328p, EnableDisable)
{
    timer::Atmega328p timer{100U};

    EXPECT_TRUE(timer.isInitialized());
    EXPECT_FALSE(timer.isEnabled());

    timer.start();
    EXPECT_TRUE(timer.isEnabled());

    timer.stop();
    EXPECT_FALSE(timer.isEnabled());

    timer.toggle();
    EXPECT_TRUE(timer.isEnabled());

    timer.toggle();
    EXPECT_FALSE(timer.isEnabled());
}

/**
 * @brief Timer timeout test.
 * 
 *        Verify that timeout values can be set and read correctly.
 */
TEST(Timer_Atmega328p, Timeout)
{
    timer::Atmega328p timer{100U};

    EXPECT_TRUE(timer.isInitialized());
    EXPECT_EQ(timer.timeout_ms(), 100U);

    timer.setTimeout_ms(200U);
    EXPECT_EQ(timer.timeout_ms(), 200U);

    timer.setTimeout_ms(0U);
    EXPECT_EQ(timer.timeout_ms(), 200U);
}

/**
 * @brief Timer callback test.
 * 
 *        Verify that timer callbacks are invoked when timeout occurs.
 */
TEST(Timer_Atmega328p, Callback)
{
    resetCallbackFlag();

    constexpr std::uint32_t timeoutMs{10U};
    timer::Atmega328p timer{timeoutMs, testCallback};

    EXPECT_TRUE(timer.isInitialized());

    timer.start();
    EXPECT_TRUE(timer.isEnabled());

    const std::uint32_t countMax{getMaxCount(timeoutMs)};
    for (std::uint32_t i{}; i < countMax; ++i)
    {
        timer.handleCallback();
    }

    EXPECT_TRUE(callbackInvoked);
}

/**
 * @brief Timer restart test.
 * 
 *        Verify that timers can be restarted correctly.
 */
TEST(Timer_Atmega328p, Restart)
{
    resetCallbackFlag();

    constexpr std::uint32_t timeoutMs{10U};
    timer::Atmega328p timer{timeoutMs, testCallback};

    EXPECT_TRUE(timer.isInitialized());

    timer.start();
    EXPECT_TRUE(timer.isEnabled());

    const std::uint32_t countMax{getMaxCount(timeoutMs)};

    for (std::uint32_t i{}; i + 1U < countMax; ++i)
    {
        timer.handleCallback();
    }

    EXPECT_FALSE(callbackInvoked);

    timer.restart();
    EXPECT_TRUE(timer.isEnabled());

    for (std::uint32_t i{}; i + 1U < countMax; ++i)
    {
        timer.handleCallback();
    }

    EXPECT_FALSE(callbackInvoked);

    timer.handleCallback();
    EXPECT_TRUE(callbackInvoked);
}

//! @todo Add more tests here (e.g., register verification, multiple timers running simultaneously).

} // namespace
} // namespace driver



#endif /** TESTSUITE */

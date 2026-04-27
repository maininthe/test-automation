/**
 * @brief Component tests for the logic implementation.
 */
#include <chrono>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <thread>

#include <gtest/gtest.h>

#include "driver/eeprom/stub.h"
#include "driver/gpio/stub.h"
#include "driver/serial/stub.h"
#include "driver/tempsensor/stub.h"
#include "driver/timer/stub.h"
#include "driver/watchdog/stub.h"
#include "logic/stub.h"



#ifdef TESTSUITE

namespace logic
{
namespace
{
/**
 * @brief Structure of mock instances.
 * 
 * @tparam EepromSize EEPROM size in bytes (default = 1024).
 */
template <std::uint16_t EepromSize = 1024U>
struct Mock final
{
    // Generate a compiler error if the EEPROM size is set to 0.
    static_assert(0U < EepromSize, "EEPROM size must be greater than 0!");

    /** LED stub. */
    driver::gpio::Stub led;

    /** Toggle button stub. */
    driver::gpio::Stub toggleButton;

    /** Temperature button stub. */
    driver::gpio::Stub tempButton;

    /** Debounce timer stub. */
    driver::timer::Stub debounceTimer;

    /** Toggle timer stub. */
    driver::timer::Stub toggleTimer;

    /** Temperature timer stub. */
    driver::timer::Stub tempTimer;

    /** Serial driver stub. */
    driver::serial::Stub serial;

    /** Watchdog timer stub. */
    driver::watchdog::Stub watchdog;

    /** EEPROM stream stub. */
    driver::eeprom::Stub<EepromSize> eeprom;

    /** Temperature sensor stub. */
    driver::tempsensor::Stub tempSensor;

    /** Logic implementation stub. */
    std::unique_ptr<logic::Stub> logicImpl;

    /** 
     * @brief Constructor.
     */
    Mock() noexcept
        : led{}
        , toggleButton{}
        , tempButton{}
        , debounceTimer{}
        , toggleTimer{}
        , tempTimer{}
        , serial{}
        , watchdog{}
        , eeprom{}
        , tempSensor{}
        , logicImpl{nullptr}
    {}

    /**
     * @brief Destructor.
     */
    ~Mock() noexcept = default;

    /**
     * @brief Create logic implementation.
     * 
     * @return Reference to the logic implementation.
     */
    logic::Interface& createLogic()
    {
        logicImpl = std::make_unique<logic::Stub>(
            led, toggleButton, tempButton, debounceTimer, toggleTimer, 
            tempTimer, serial, watchdog, eeprom, tempSensor);
        return *logicImpl;
    }

    /**
     * @brief Run system.
     * 
     * @param[in] testDuration_ms Duration to run the system in ms (default = 10 ms).
     */
    void runSystem(const std::size_t testDuration_ms = 10U)
    {
        // Throw an exception if the logic is not initialized.
        if (nullptr == logicImpl) { throw std::invalid_argument("Logic not initialized!\n"); }
        
        // Run the system, stop after given duration has passed.
        bool stop{false};
        std::thread t1{runLogicThread, std::ref(*logicImpl), std::ref(stop)};
        std::thread t2{stopLogicThread, testDuration_ms, std::ref(stop)};
        t1.join();
        t2.join();
    }

    Mock(const Mock&)            = delete; // No copy constructor.
    Mock(Mock&&)                 = delete; // No move constructor.
    Mock& operator=(const Mock&) = delete; // No copy assignment.
    Mock& operator=(Mock&&)      = delete; // No move assignment.

private:
    // -----------------------------------------------------------------------------
    static void runLogicThread(logic::Interface& logic, bool& stop) noexcept
    {
        // Run the logic loop as long as the stop flag is low.
        logic.run(stop);
    }

    // -----------------------------------------------------------------------------
    static void stopLogicThread(const std::size_t timeout_ms, bool& stop) noexcept
    {
        // Stop the logic loop on timeout.
        stop = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
        stop = true;
    }
};

/**
 * @brief Debounce handling test.
 *
 *        Verify that the debounce handling behaves as expected.
 */
TEST(Logic, DebounceHandling)
{
    // Create logic implementation and run the system.
    Mock mock{};
    logic::Interface& logic{mock.createLogic()};
    mock.runSystem();

    // Case 1 - Press the toggle button, simulate button event.
    // Expect button interrupts to be disabled and the debounce timer to be enabled.
    // Expect the toggle timer to be enabled due to the toggle button being pressed.
    {
        mock.toggleButton.write(true);
        logic.handleButtonEvent();
        mock.toggleButton.write(false);

        EXPECT_FALSE(mock.tempButton.isInterruptEnabled());
        EXPECT_TRUE(mock.debounceTimer.isEnabled());
        EXPECT_TRUE(mock.toggleTimer.isEnabled());
    }

    // Case 2 - Call the button event handler before the debounce timer has timed out.
    // Expect this call to be ignored, i.e, expect the toggle timer to still be enabled.
    {
        mock.toggleButton.write(true);
        logic.handleButtonEvent();
        mock.toggleButton.write(false);
        EXPECT_TRUE(mock.toggleTimer.isEnabled());
    }

    // Case 3 - Simulate debounce timer timeout, expect toggle button interrupt to be re-enabled.
    // Expect the debounce timer to be disabled.
    {
        EXPECT_TRUE(mock.debounceTimer.isEnabled());
        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();
        EXPECT_TRUE(mock.toggleButton.isInterruptEnabled());
        EXPECT_FALSE(mock.debounceTimer.isEnabled());
    }

    // Case 4 - Press the toggle button again after the debounce timer has timed out, 
    // simulate button event.
    // Expect button interrupts to be disabled and the debounce timer to be enabled.
    // Expect the toggle timer to be disabled due to the toggle button being pressed.
    {
        mock.toggleButton.write(true);
        logic.handleButtonEvent();
        mock.toggleButton.write(false);

        EXPECT_FALSE(mock.tempButton.isInterruptEnabled());
        EXPECT_TRUE(mock.debounceTimer.isEnabled());
        EXPECT_FALSE(mock.toggleTimer.isEnabled());
    }
}

/**
 * @brief Toggle handling test.
 *
 *        Verify that the toggle handling behaves as expected.
 */
TEST(Logic, ToggleHandling)
{
    Mock mock{};
    logic::Interface& logic{mock.createLogic()};
    mock.runSystem();

    EXPECT_FALSE(mock.toggleTimer.isEnabled());
    EXPECT_FALSE(mock.led.read());

    {
        mock.tempButton.write(true);
        logic.handleButtonEvent();
        mock.tempButton.write(false);

        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();

        EXPECT_FALSE(mock.toggleTimer.isEnabled());
    }

    {
        mock.toggleButton.write(true);
        logic.handleButtonEvent();
        mock.toggleButton.write(false);

        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();

        EXPECT_TRUE(mock.toggleTimer.isEnabled());
    }

    {
        mock.toggleTimer.setTimedOut(true);
        logic.handleToggleTimerTimeout();
        EXPECT_TRUE(mock.led.read());
        mock.toggleTimer.setTimedOut(false);
    }

    {
        mock.toggleTimer.setTimedOut(true);
        logic.handleToggleTimerTimeout();
        EXPECT_FALSE(mock.led.read());
        mock.toggleTimer.setTimedOut(false);
    }

    {
        mock.toggleTimer.setTimedOut(true);
        logic.handleToggleTimerTimeout();
        EXPECT_TRUE(mock.led.read());
        mock.toggleTimer.setTimedOut(false);
    }

    {
        mock.toggleButton.write(true);
        logic.handleButtonEvent();
        mock.toggleButton.write(false);

        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();

        EXPECT_FALSE(mock.toggleTimer.isEnabled());
        EXPECT_FALSE(mock.led.read());
    }

    {
        mock.tempTimer.setTimedOut(true);
        logic.handleTempTimerTimeout();
        EXPECT_FALSE(mock.led.read());
        mock.tempTimer.setTimedOut(false);
    }

    {
        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();
        EXPECT_FALSE(mock.led.read());
        mock.debounceTimer.setTimedOut(false);
    }
}

/**
 * @brief Temperature handling test.
 *
 *        Verify that the temperature handling behaves as expected.
 */
TEST(Logic, TempHandling)
{
    Mock mock{};
    logic::Interface& logic{mock.createLogic()};
    mock.runSystem();

    EXPECT_TRUE(mock.tempTimer.isEnabled());

    mock.tempSensor.setTemperature(25);
    mock.serial.clearWrittenText();

    {
        mock.toggleButton.write(true);
        logic.handleButtonEvent();
        mock.toggleButton.write(false);

        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();
        mock.debounceTimer.setTimedOut(false);

        EXPECT_EQ(mock.serial.writtenText().find("Temperature: 25 Celsius"), std::string::npos);
    }

    mock.serial.clearWrittenText();

    {
        mock.tempButton.write(true);
        logic.handleButtonEvent();
        mock.tempButton.write(false);

        mock.debounceTimer.setTimedOut(true);
        logic.handleDebounceTimerTimeout();
        mock.debounceTimer.setTimedOut(false);

        EXPECT_NE(mock.serial.writtenText().find("25 Celsius"), std::string::npos);
    }

    mock.serial.clearWrittenText();

    {
        mock.tempTimer.setTimedOut(true);
        logic.handleTempTimerTimeout();
        EXPECT_NE(mock.serial.writtenText().find("25 Celsius"), std::string::npos);
        mock.tempTimer.setTimedOut(false);
    }
}
/**
 * @brief EEPROM handling test.
 *
 *        Verify that the EEPROM handling behaves as expected.
 */
TEST(Logic, Eeprom)
{
    {
        Mock mock{};
        logic::Interface& logic{mock.createLogic()};
        mock.runSystem();

        (void)logic;
        EXPECT_FALSE(mock.toggleTimer.isEnabled());
    }

    {
        Mock mock{};
        mock.eeprom.writeByte(0U, 1U);

        logic::Interface& logic{mock.createLogic()};
        mock.runSystem();

        (void)logic;
        EXPECT_TRUE(mock.toggleTimer.isEnabled());
    }
}
} // namespace
} // namespace logic

#endif /** TESTSUITE */



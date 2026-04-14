/**
 * @brief Unit tests for the ATmega328p serial driver.
 */
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "arch/avr/hw_platform.h"
#include "driver/serial/atmega328p.h"
#include "utils/utils.h"

#ifdef TESTSUITE



//! @todo Implement tests according to project requirements.
namespace driver
{
namespace
{
/** Simulated transmission delay in microseconds. */
constexpr std::size_t TransmissionDelay_us{10U};

// -----------------------------------------------------------------------------
serial::Interface& initSerial() noexcept
{
    // Initialize and enable serial instance.
    serial::Interface& serial{serial::Atmega328p::getInstance()};
    serial.setEnabled(true);
    return serial;
}

// -----------------------------------------------------------------------------
void delay_us(const std::size_t duration_us) noexcept
{
    std::this_thread::sleep_for(std::chrono::microseconds(duration_us));
}

// -----------------------------------------------------------------------------
void simulateDataReg(const bool& stop) noexcept
{
    std::uint8_t prevByte{};

    // Initialize UDR0 to 0 and set UDRE0 (data register empty, ready to write).
    UDR0 = 0U;
    utils::set(UCSR0A, UDRE0);

    // Clear UDRE0 when UDR0 changes to simulate that the data register is full.
    while (!stop)
    {
        constexpr std::size_t loopInterval_us{1U};

        // Read the current byte from UDR0.
        const std::uint8_t byte{UDR0};

        // Check if UDR0 has changed and is non-zero (new data written).
        if ((byte != prevByte) && (0U != byte))
        {
            prevByte = byte;
            
            // Simulate transmission delay, then mark that the data register is full.
            delay_us(TransmissionDelay_us);
            utils::clear(UCSR0A, UDRE0);
        }
        delay_us(loopInterval_us);
    }
}

// -----------------------------------------------------------------------------
void printThread(serial::Interface& serial, const std::string& msg, bool& stop) noexcept
{
    serial.print(msg.c_str());
    stop = true;
}

// -----------------------------------------------------------------------------
void readDataRegThread(const std::string& msg, const bool& stop) noexcept
{
    for (const auto& c : msg)
    {
        while (utils::read(UCSR0A, UDRE0) && !stop) { delay_us(TransmissionDelay_us); }

        if (stop) { break; }

        EXPECT_EQ(static_cast<char>(UDR0), c);

        utils::set(UCSR0A, UDRE0);
    }
}

/**
 * @brief Serial initialization test.
 * 
 *        Verify that the serial driver is initialized correctly.
 */
TEST(Serial_Atmega328p, Initialization)
{
    serial::Interface& serial{serial::Atmega328p::getInstance()};

    EXPECT_TRUE(serial.isInitialized());

    serial.setEnabled(true);
    EXPECT_TRUE(serial.isEnabled());

    serial.setEnabled(false);
    EXPECT_FALSE(serial.isEnabled());

    EXPECT_EQ(serial.baudRate_bps(), 9600U);
}

/**
 * @brief Serial print test.
 * 
 *        Verify that messages are transmitted correctly.
 */
TEST(Serial_Atmega328p, Transmit)
{
    // Initialize and enable the serial driver.
    serial::Interface& serial{initSerial()};
    
    // Message to transmit.
    const std::string msg{"This is a message!\n"};
    
    // Stop flag shared between threads.
    bool stopFlag{false};
    
    // Create threads to simulate data transmission.
    std::thread t1{simulateDataReg, std::ref(stopFlag)};
    std::thread t2{printThread, std::ref(serial), std::ref(msg), std::ref(stopFlag)};
    std::thread t3{readDataRegThread, std::ref(msg), std::ref(stopFlag)};

    // Synchronize the threads.
    t1.join();
    t2.join();
    t3.join();
}

//! @todo Add more tests here!

} // namespace
} // namespace driver.


#endif /** TESTSUITE */
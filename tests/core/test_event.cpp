#include <autophage/core/event.hpp>

#include <catch2/catch_test_macros.hpp>


using namespace autophage;

struct TestEvent
{
    int value;
};

struct AnotherEvent
{
    float x;
};

TEST_CASE("EventBus operations", "[core][event]")
{
    EventBus bus;

    SECTION("Subscribe and Publish")
    {
        int receivedValue = 0;
        bus.subscribe<TestEvent>([&](const TestEvent& e) { receivedValue = e.value; });

        bus.publish(TestEvent{42});
        REQUIRE(receivedValue == 42);
    }

    SECTION("Multiple Listeners")
    {
        int count = 0;
        bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });
        bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });

        bus.publish(TestEvent{1});
        REQUIRE(count == 2);
    }

    SECTION("Unsubscribe")
    {
        int count = 0;
        auto id = bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });

        bus.publish(TestEvent{1});
        REQUIRE(count == 1);

        bus.unsubscribe<TestEvent>(id);
        bus.publish(TestEvent{1});
        REQUIRE(count == 1);  // Should not increase
    }

    SECTION("Different Events")
    {
        bool testReceived = false;
        bool anotherReceived = false;

        bus.subscribe<TestEvent>([&](const TestEvent&) { testReceived = true; });
        bus.subscribe<AnotherEvent>([&](const AnotherEvent&) { anotherReceived = true; });

        bus.publish(TestEvent{1});
        REQUIRE(testReceived);
        REQUIRE_FALSE(anotherReceived);

        bus.publish(AnotherEvent{1.0f});
        REQUIRE(anotherReceived);
    }
}

// Unit tests for the facts shown in the application's information panel.

#include <string>
#include <vector>

#include "app/planet_facts.h"
#include "scene/solar_system_scene.h"
#include "test_framework.h"

TEST_CASE("FACT-01 lists the Sun, the eight planets in scene order, then the Moon") {
    std::vector<BodyFacts> facts = solarSystemFacts();
    std::vector<Planet> planets = solarSystemPlanets();

    CHECK_EQ(facts.size(), planets.size() + 2);
    CHECK_EQ(facts.front().name, std::string("Sun"));
    CHECK_EQ(facts.back().name, std::string("Moon"));

    for (std::size_t i = 0; i < planets.size(); i++) {
        CHECK_EQ(facts.at(i + 1).name, std::string(planets[i].name));
    }
}

TEST_CASE("FACT-02 every body has every field filled in") {
    for (const BodyFacts& body : solarSystemFacts()) {
        CHECK(!body.type.empty());
        CHECK(!body.diameter.empty());
        CHECK(!body.distance.empty());
        CHECK(!body.year.empty());
        CHECK(!body.day.empty());
        CHECK(!body.moons.empty());
        CHECK(!body.fact.empty());
        CHECK(body.rotationDays != 0.0);
    }
}

TEST_CASE("FACT-03 Venus and Uranus spin backwards, the others forwards") {
    for (const BodyFacts& body : solarSystemFacts()) {
        bool backwards = body.name == "Venus" || body.name == "Uranus";
        CHECK_EQ(body.rotationDays < 0.0, backwards);
    }
}

TEST_CASE("FACT-04 rotation periods match known values") {
    CHECK_NEAR(factsFor("Earth").rotationDays, 1.0, 0.01);
    CHECK_NEAR(factsFor("Mars").rotationDays, 1.03, 0.01);
    CHECK_NEAR(factsFor("Venus").rotationDays, -243.0, 1.0);
    CHECK(factsFor("Jupiter").rotationDays < 0.5);   // fastest spinner
}

TEST_CASE("FACT-05 moon counts for the rocky planets") {
    CHECK_EQ(factsFor("Mercury").moons, std::string("0"));
    CHECK_EQ(factsFor("Venus").moons, std::string("0"));
    CHECK_EQ(factsFor("Earth").moons, std::string("1"));
    CHECK_EQ(factsFor("Mars").moons, std::string("2"));
}

TEST_CASE("FACT-06 factsFor finds bodies by name and returns empty for others") {
    CHECK_EQ(factsFor("Saturn").type, std::string("Gas giant"));
    CHECK(factsFor("Pluto").name.empty());
    CHECK(factsFor("").name.empty());
}

TEST_CASE("FACT-07 text only uses characters the bitmap font can draw") {
    const std::string allowed =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        " .,:;-+=/()[]%'!?*<>_|~";

    for (const BodyFacts& body : solarSystemFacts()) {
        for (const std::string& text : {body.type, body.diameter, body.distance,
                                        body.year, body.day, body.moons, body.fact}) {
            CHECK_EQ(text.find_first_not_of(allowed), std::string::npos);
        }
    }
}

int main() {
    return testing::runAllTests();
}

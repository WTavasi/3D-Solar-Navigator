#ifndef APP_PLANET_FACTS_H
#define APP_PLANET_FACTS_H

// Facts shown in the application's information panel, and the rotation
// periods used for axial spin.
//
// Sources: NASA planetary fact sheets for sizes, distances and periods;
// moon counts as listed by Live Science, March 2026.

#include <string>
#include <vector>

struct BodyFacts {
    std::string name;
    std::string type;
    std::string diameter;        // equatorial diameter
    std::string distance;        // average distance from the Sun (or Earth)
    std::string year;            // time for one orbit
    std::string day;             // time for one spin
    std::string moons;
    std::string fact;            // one sentence, wrapped by the panel

    // Sidereal rotation period in Earth days; negative = spins backwards.
    double rotationDays;
};

// The Sun, the eight planets in order, then Earth's Moon.
inline std::vector<BodyFacts> solarSystemFacts() {
    return {
        {"Sun", "Star (yellow dwarf)", "1,392,700 km", "Centre of the system",
         "Orbits the galaxy every 230 million years", "About 25 days at the equator",
         "None (8 planets orbit it)",
         "Holds 99.8 percent of all the mass in the Solar System.", 25.4},
        {"Mercury", "Rocky planet", "4,879 km", "57.9 million km from the Sun",
         "88 days", "58.6 Earth days", "0",
         "Smallest planet. With almost no air it swings from -180 to 430 ~C.", 58.6},
        {"Venus", "Rocky planet", "12,104 km", "108.2 million km from the Sun",
         "225 days", "243 Earth days (spins backwards)", "0",
         "Hottest planet at about 465 ~C, under thick clouds of acid.", -243.0},
        {"Earth", "Rocky planet", "12,742 km", "149.6 million km from the Sun",
         "365.25 days", "23.9 hours", "1",
         "The only world known to support life. Oceans cover 71 percent.", 0.997},
        {"Mars", "Rocky planet", "6,779 km", "227.9 million km from the Sun",
         "687 days", "24.6 hours", "2",
         "Home of Olympus Mons, the tallest volcano in the Solar System.", 1.026},
        {"Jupiter", "Gas giant", "139,820 km", "778.5 million km from the Sun",
         "11.9 years (4,333 days)", "9.9 hours", "101",
         "Its Great Red Spot is a storm wider than the whole Earth.", 0.414},
        {"Saturn", "Gas giant", "116,460 km", "1.43 billion km from the Sun",
         "29.5 years (10,759 days)", "10.7 hours", "285 or more",
         "Its rings are mostly ice and are under 1 km thick.", 0.444},
        {"Uranus", "Ice giant", "50,724 km", "2.87 billion km from the Sun",
         "84 years (30,687 days)", "17.2 hours (spins backwards)", "28",
         "Tipped 98 ~ on its side, it rolls around the Sun.", -0.718},
        {"Neptune", "Ice giant", "49,244 km", "4.50 billion km from the Sun",
         "165 years (60,190 days)", "16.1 hours", "16",
         "Has the fastest winds in the Solar System, over 2,000 km/h.", 0.671},
        {"Moon", "Earth's moon", "3,474 km", "384,400 km from Earth",
         "27.3 days around Earth", "27.3 days", "-",
         "Always shows the same face to Earth.", 27.3}
    };
}

// Facts for one body by name; an empty name means none was found.
inline BodyFacts factsFor(const std::string& name) {
    for (const BodyFacts& facts : solarSystemFacts()) {
        if (facts.name == name) {
            return facts;
        }
    }

    return BodyFacts{};
}

#endif

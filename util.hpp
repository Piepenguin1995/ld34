#ifndef UTIL_HPP
#define UTIL_HPP

#include <SFML/Graphics.hpp>

namespace ld
{
// Return a random float on [a, b) (hopefully)
float rand(float a, float b);
// Convert a hsv colour to an SFML-compatible rgb version
sf::Color hsvToRgb(float h, float s, float v);
sf::Color hsvToRgb(const sf::Vector3f& hsv);
// Convert degrees to radians
float toRad(float deg);
}

#endif /* UTIL_HPP */

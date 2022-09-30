#ifndef COORDS_H
#define COORDS_H

#include <iostream>
#include <random>
#include <utility>
#include <cmath>

std::pair<double, double> getLocation(double x0, double y0, int radius) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0,49);
    // Convert radius from meters to degrees
    double radiusInDegrees = radius / 111000.0;

    double u = dist(rd);
    double v = dist(rd);
    double w = radiusInDegrees * sqrt(u);
    double t = 2 * M_PI * v;
    double x = w * cos(t);
    double y = w * sin(t);

    // Adjust the x-coordinate for the shrinking of the east-west distances
    double new_x = x / cos(M_PI * y0);
    double foundLongitude = new_x + x0;
    double foundLatitude = y + y0;
    //std::cout << "Longitude: " ;
    return std::make_pair(foundLongitude, foundLatitude);
}
#else
#endif
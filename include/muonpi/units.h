#ifndef MUONPI_UNITS_H
#define MUONPI_UNITS_H

#include "muonpi/global.h"

namespace muonpi {

namespace units {
static constexpr double pi = 3.14159265358979323846;

static constexpr double giga = 1.0e9;
static constexpr double mega = 1.0e6;
static constexpr double kilo = 1.0e3;

static constexpr double centi = 1.0e-2;
static constexpr double milli = 1.0e-3;
static constexpr double micro = 1.0e-6;
static constexpr double nano  = 1.0e-9;

static constexpr double radian = 1.0;
static constexpr double degree = (pi / 180.0) * radian;

static constexpr double meter     = 1.0;
static constexpr double kilometer = kilo * meter;

static constexpr double nanosecond = 1.0;
static constexpr double second     = nanosecond / nano;
} // namespace units

namespace consts {
static constexpr double c_0 {299'792'458.0 * units::nano};
} // namespace consts

} // namespace muonpi

#endif // MUONPI_UNITS_H

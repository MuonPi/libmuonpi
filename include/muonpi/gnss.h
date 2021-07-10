#ifndef COORDINATEMODEL_H
#define COORDINATEMODEL_H

#include "muonpi/global.h"

#include <cmath>

namespace muonpi::coordinate {

/**
 * @brief The Geodetic struct, contains geodetic coordinates
 */
template <typename T>
struct LIBMUONPI_PUBLIC geodetic {
    T lat { 0.0 };
    T lon { 0.0 };
    T h { 0.0 };
};

/**
 * @brief The Ecef struct, coordinate data in the Ecef reference system. Earth-relative carthesian coordinates
 */
template <typename T>
struct LIBMUONPI_PUBLIC ecef {
    double x { 0.0 };
    double y { 0.0 };
    double z { 0.0 };
};

/**
 * @brief The Enu struct, coordinate data in the Enu reference system. Carthesian coordinates relative to other Ecef coordinates.
 */
template <typename T>
struct LIBMUONPI_PUBLIC enu {
    double x { 0.0 };
    double y { 0.0 };
    double z { 0.0 };
};

template <typename T>
/**
 * @brief Implementes the WGS84 models for coordinate transformations
 */
struct LIBMUONPI_PUBLIC WGS84 {
    constexpr static double a { 6378137.0 };
    constexpr static double b { 6356752.314245 };
    constexpr static double f { 1.0 / 298.257223563 };
    constexpr static double e_squared { 2.0 * f - f * f };
};

template <typename T>
/**
 * @brief Implementes the GRS80 models for coordinate transformations
 */
struct LIBMUONPI_PUBLIC GRS80 {
    constexpr static double a { 6378137.0 };
    constexpr static double b { 6356752.314140 };
    constexpr static double f { 1.0 / 298.257222100882711 };
    constexpr static double e_squared { 2.0 * f - f * f };
};

template <typename T>
class LIBMUONPI_PUBLIC hash {
public:
    [[nodiscard]] static auto from_geodetic(const geodetic<T>& coords, std::size_t precision) -> std::string;

private:
    static constexpr const char* base32 { "0123456789bcdefghjkmnpqrstuvwxyz" }; // (geohash-specific) base32 map
};

template <typename T, template <typename MT = T> typename Model>
/**
 * @brief The Model class, prototype for conversion between all three coordinate systems. Implementations can be created depending on the specific earth Model used.
 */
class LIBMUONPI_PUBLIC transformation {
public:
    /**
     * @brief to_ecef converts geodetic coordinats to ecef reference system
     * @param coords  Geodetic coordinates
     * @return Ecef coordinates
     */
    [[nodiscard]] static auto to_ecef(const geodetic<T>& coords) -> ecef<T>;

    /**
     * @brief to_ecef converts Enu coordinates to ecef reference system
     * @param coords Enu coordinates
     * @param reference Reference coordinates in Ecef
     * @return Coordinates in Ecef reference system
     */
    [[nodiscard]] static auto to_ecef(const enu<T>& coords, const ecef<T>& reference) -> ecef<T>;

    /**
     * @brief to_enu Convert geodetic coordinates to enu coordinates
     * @param coords geodetic coordinates to convert
     * @param reference The reference ecef coordinates
     * @return
     */
    [[nodiscard]] static auto to_enu(const geodetic<T>& coords, const ecef<T>& reference) -> enu<T>;

    /**
     * @brief to_enu Convert ecef coordinates to enu coordinates
     * @param coords The ecef coordinates to convert
     * @param reference The reference ecef coordinates
     * @return
     */
    [[nodiscard]] static auto to_enu(const ecef<T>& coords, const ecef<T>& reference) -> enu<T>;

    /**
     * @brief to_geodetic Convert enu coordinates to geodetic coordinates
     * @param coords The enu coordinates to convert
     * @param reference The reference coordinates for the enu coordinates
     * @return
     */
    [[nodiscard]] static auto to_geodetic(const enu<T>& coords, const ecef<T>& reference) -> geodetic<T>;

    /**
     * @brief to_geodetic Convert ecef coordinates to geodetic coordinates
     * @param coords The ecef coordinates to convert
     * @return
     */
    [[nodiscard]] static auto to_geodetic(const ecef<T>& coords) -> geodetic<T>;

    /**
     * @brief straight_distance Calculate the straight distance between two geodetic coordinates
     * @param first The first set of coordinates
     * @param second The second set of coordinates
     * @return
     */
    [[nodiscard]] static auto straight_distance(const geodetic<T>& first, const geodetic<T>& second) -> T;
};

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::to_ecef(const geodetic<T>& coords) -> ecef<T>
{
    const T N { Model<T>::a / std::sqrt(1.0 - Model<T>::e_squared * std::pow(std::sin(coords.lat), 2.0)) };
    return {
        (N + coords.h) * std::cos(coords.lat) * std::cos(coords.lon),
        (N + coords.h) * std::cos(coords.lat) * std::sin(coords.lon),
        (N * std::pow(Model<T>::b, 2.0) / std::pow(Model<T>::a, 2.0) + coords.h) * std::sin(coords.lat)
    };
}

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::to_ecef(const enu<T>& coords, const ecef<T>& reference) -> ecef<T>
{
    geodetic ref_g { to_geodetic(reference) };

    return {
        (-std::sin(ref_g.lon) * coords.x - std::sin(ref_g.lat) * std::cos(ref_g.lon) * coords.y + std::cos(ref_g.lat) * std::cos(ref_g.lon) * coords.z) + reference.x,
        (std::cos(ref_g.lon) * coords.x - std::sin(ref_g.lat) * std::sin(ref_g.lon) * coords.y + std::cos(ref_g.lat) * std::sin(ref_g.lon) * coords.z) + reference.y,
        (std::cos(ref_g.lat) * coords.y + std::sin(ref_g.lat) * coords.z) + reference.z,
    };
}

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::to_enu(const geodetic<T>& coords, const ecef<T>& reference) -> enu<T>
{
    return to_enu(to_ecef(coords), reference);
}

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::to_enu(const ecef<T>& coords, const ecef<T>& reference) -> enu<T>
{
    geodetic ref_g { to_geodetic(reference) };
    const double d_x { coords.x - reference.x };
    const double d_y { coords.y - reference.y };
    const double d_z { coords.z - reference.z };
    return {
        -std::sin(ref_g.lon) * d_x + std::cos(ref_g.lon) * d_y,
        -std::sin(ref_g.lat) * std::cos(ref_g.lon) * d_x - std::sin(ref_g.lat) * std::sin(ref_g.lon) * d_y + std::cos(ref_g.lat) * d_z,
        std::cos(ref_g.lat) * std::cos(ref_g.lon) * d_x + std::cos(ref_g.lat) * std::sin(ref_g.lon) * d_y + std::sin(ref_g.lat) * d_z
    };
}

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::to_geodetic(const enu<T>& coords, const ecef<T>& reference) -> geodetic<T>
{
    return to_geodetic(to_ecef(coords, reference));
}

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::to_geodetic(const ecef<T>& coords) -> geodetic<T>
{
    const double r { std::sqrt(std::pow(coords.x, 2.0) + std::pow(coords.y, 2.0)) };
    const double e_squared { (std::pow(Model<T>::a, 2.0) - std::pow(Model<T>::b, 2.0)) / std::pow(Model<T>::b, 2.0) };
    const double F { 54.0 * std::pow(Model<T>::b, 2.0) * std::pow(coords.z, 2.0) };
    const double G { std::pow(r, 2.0) + (1.0 - Model<T>::e_squared) * std::pow(coords.z, 2.0) - Model<T>::e_squared * (std::pow(Model<T>::a, 2.0) - std::pow(Model<T>::b, 2.0)) };
    const double c { std::pow(Model<T>::e_squared * r, 2.0) * F / std::pow(G, 3.0) };
    const double s { std::pow(1.0 + c + std::sqrt(std::pow(c, 2.0) + 2.0 * c), 1.0 / 3.0) };
    const double P { F / (3.0 * std::pow((s + 1.0 + 1.0 / s) * G, 2.0)) };
    const double Q { std::sqrt(1.0 + 2.0 * std::pow(Model<T>::e_squared, 2.0) * P) };
    const double r_0 { -P * Model<T>::e_squared * r / (1.0 + Q) + std::sqrt(0.5 * std::pow(Model<T>::a, 2.0) * (1.0 + 1.0 / Q) - P * (1.0 - Model<T>::e_squared) * std::pow(coords.z, 2.0) / (Q * (1.0 + Q)) - 0.5 * P * std::pow(r, 2.0)) };
    const double U { std::sqrt(std::pow(r - Model<T>::e_squared * r_0, 2.0) + std::pow(coords.z, 2.0)) };
    const double V { std::sqrt(std::pow(r - Model<T>::e_squared * r_0, 2.0) + (1.0 - Model<T>::e_squared) * std::pow(coords.z, 2.0)) };
    const double z_0 { std::pow(Model<T>::b, 2.0) * coords.z / (Model<T>::a * V) };
    return {
        std::atan((coords.z + e_squared * z_0) / r),
        std::atan2(coords.y, coords.x),
        U * (1.0 - std::pow(Model<T>::b, 2.0) / (Model<T>::a * V))
    };
}

template <typename T, template <typename MT = T> typename Model>
auto transformation<T, Model>::straight_distance(const geodetic<T>& first, const geodetic<T>& second) -> T
{
    const auto second_enu { to_enu(second, to_ecef(first)) };
    return std::sqrt(std::pow(second_enu.x, 2.0) + std::pow(second_enu.y, 2.0) + std::pow(second_enu.z, 2.0));
}



template <typename T>
[[nodiscard]] auto hash<T>::from_geodetic(const geodetic<T>& coords, std::size_t precision) -> std::string
{
    precision = std::min(precision, 12UL);
    double latMin = -90.;
    double latMax = 90.;
    double lonMin = -180.;
    double lonMax = 180.;


    if ((coords.lon < lonMin) || (coords.lon > lonMax)) {
        return {};
    }
    if ((coords.lat < latMin) || (coords.lat > latMax)) {
        return {};
    }

    uint8_t idx = 0; // index into base32 map
    uint8_t bit = 0; // each char holds 5 bits
    bool evenBit = true;
    std::string geohash {};

    while (geohash.size() < precision) {
        if (evenBit) {
            // bisect E-W longitude
            const double lonMid = (lonMin + lonMax) / 2.;
            if (coords.lon >= lonMid) {
                idx = idx * 2 + 1;
                lonMin = lonMid;
            } else {
                idx = idx * 2;
                lonMax = lonMid;
            }
        } else {
            // bisect N-S latitude
            const double latMid = (latMin + latMax) / 2.;
            if (coords.lat >= latMid) {
                idx = idx * 2 + 1;
                latMin = latMid;
            } else {
                idx = idx * 2;
                latMax = latMid;
            }
        }
        evenBit = !evenBit;

        if (++bit == 5) {
            // 5 bits gives us a character: append it and start over
            geohash += base32[idx];
            bit = 0;
            idx = 0;
        }
    }
    return geohash;
}

}

#endif // COORDINATEMODEL_H

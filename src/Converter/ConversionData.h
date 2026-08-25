#ifndef CONVERTER_CONVERSIONDATA_H
#define CONVERTER_CONVERSIONDATA_H

#include <TopoDS_Shape.hxx>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct StepModelWrapper
{
    TopoDS_Shape shape;
};

struct MeshDataWrapper
{
    struct MeshPart
    {
        std::vector<std::array<double, 3>> vertices;
        std::vector<std::array<std::uint32_t, 3>> indices;
    };

    std::vector<MeshPart> parts;
    bool hasBounds = false;
    std::array<double, 3> minimumBounds {};
    std::array<double, 3> maximumBounds {};
};

struct PrcDataWrapper
{
    std::string buffer;
    bool hasBounds = false;
    std::array<double, 3> minimumBounds {};
    std::array<double, 3> maximumBounds {};
};

#endif // CONVERTER_CONVERSIONDATA_H

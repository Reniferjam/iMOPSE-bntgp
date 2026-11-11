#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bntgp_cvrptw::gp
{
    // CVRPTW candidate-customer features (all normalized by the decoder).
    enum class FeatureId : std::uint8_t
    {
        DEMAND,                // demand / capacity
        DISTANCE,              // d(current, customer) / max edge
        REMAINING_CAPACITY,    // remaining capacity / capacity
        DISTANCE_FROM_DEPOT,   // d(depot, customer) / max edge
        CUSTOMERS_LEFT,        // unserved customers / (customers + depot)
        VEHICLES_LEFT,         // (fleet - routes closed) / fleet   (can be negative)

        REF_CENTER_DISTANCE,   // distance to the centre of the bounding box
        REF_Q1_DISTANCE,       // distance to the centre of the (min x, min y) quadrant
        REF_Q2_DISTANCE,       // ... (min x, max y)
        REF_Q3_DISTANCE,       // ... (max x, max y)
        REF_Q4_DISTANCE,       // ... (max x, min y)

        CURRENT_TIME,          // current vehicle time / max due time
        TRAVEL_TIME,           // d(current, customer) / max due time
        WAITING_TIME,          // max(0, ready - arrival) / max due time
        DELAY,                 // max(0, arrival - due) / max due time

        COUNT
    };

    inline constexpr std::size_t FeatureCount =
        static_cast<std::size_t>(FeatureId::COUNT);

    enum class NodeKind : std::uint8_t
    {
        CONST,
        FEATURE,
        UNARY,
        BINARY
    };

    enum class UnaryOp : std::uint8_t
    {
        NEG,
        ABS
    };

    enum class BinaryOp : std::uint8_t
    {
        ADD,
        SUB,
        MUL,
        DIV,
        MIN,
        MAX
    };

    using NodeIndex = std::int32_t;

    inline constexpr NodeIndex InvalidNodeIndex = -1;

    struct GPNode final
    {
        double constant{ 0.0 };

        NodeIndex left{ InvalidNodeIndex };
        NodeIndex right{ InvalidNodeIndex };

        FeatureId feature{ FeatureId::DEMAND };
        NodeKind kind{ NodeKind::CONST };
        UnaryOp unaryOperation{ UnaryOp::NEG };
        BinaryOp binaryOperation{ BinaryOp::ADD };
    };

    static_assert(
        std::is_trivially_copyable_v<GPNode>,
        "GPNode must remain trivially copyable.");

    static_assert(
        sizeof(GPNode) <= 32U,
        "GPNode unexpectedly occupies too much memory.");
}
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bntgp_cvrptw::gp
{
    // CVRPTW candidate-customer features (all normalized by the decoder).
    enum class FeatureId : std::uint8_t
    {
        DIST_CUR,         // d(current position, customer)
        DIST_DEPOT,       // d(customer, depot)
        DEMAND,           // demand / capacity
        REM_CAP,          // remaining capacity / capacity
        DEMAND_OVER_REM,  // demand / remaining capacity
        READY,            // ready time / horizon
        DUE,              // due time / horizon
        SERVICE,          // service time / horizon
        TW_WIDTH,         // (due - ready) / horizon
        SLACK,            // (due - arrival) / horizon
        WAIT,             // max(0, ready - arrival) / horizon
        CUR_TIME,         // current vehicle time / horizon
        UNROUTED_RATIO,   // unrouted customers / n

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

        FeatureId feature{ FeatureId::DIST_CUR };
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
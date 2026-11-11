#pragma once

#include "FeatureValues.hpp"
#include "GPTree.hpp"

#include <algorithm>
#include <cmath>

namespace bntgp_cvrptw::gp
{
    inline double evaluateNode(
        const GPTree& tree,
        const NodeIndex index,
        const FeatureValues& values)
    {
        const GPNode& node = tree.node(index);

        switch (node.kind)
        {
            case NodeKind::CONST:
                return node.constant;

            case NodeKind::FEATURE:
                return values[node.feature];

            case NodeKind::UNARY:
            {
                const double a = evaluateNode(tree, node.left, values);

                switch (node.unaryOperation)
                {
                    case UnaryOp::NEG: return -a;
                    case UnaryOp::ABS: return std::fabs(a);
                }
                return a;
            }

            case NodeKind::BINARY:
            {
                const double a = evaluateNode(tree, node.left, values);
                const double b = evaluateNode(tree, node.right, values);

                switch (node.binaryOperation)
                {
                    case BinaryOp::ADD: return a + b;
                    case BinaryOp::SUB: return a - b;
                    case BinaryOp::MUL: return a * b;
                    case BinaryOp::DIV: return b == 0.0 ? 0.0 : a / b;
                    case BinaryOp::MIN: return std::min(a, b);
                    case BinaryOp::MAX: return std::max(a, b);
                }
                return a;
            }
        }

        return 0.0;
    }

    // Priority score of one candidate customer. Higher = served first.
    [[nodiscard]]
    inline double evaluateTree(
        const GPTree& tree,
        const FeatureValues& values)
    {
        return evaluateNode(tree, tree.rootIndex(), values);
    }
}
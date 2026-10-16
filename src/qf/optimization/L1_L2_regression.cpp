#include "qf/core/error.hpp"
#include <qf/optimization/L1_L2_regression.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace qf::optimization {
namespace {

[[nodiscard]] double value(qf::Real x) noexcept{
    return x;
}

[[nodiscard]] bool finite(qf::Real x) noexcept {
    return std::isfinite(value(x));
}

[[nodiscard]] bool validOptions(const RegressionOptions& options) noexcept {
    return finite(options.epsilon) && finite(options.tolerance) && value(options.epsilon) > 0.0 && value(options.tolerance) > 0.0 && options.maxIterations > 0;
}

[[nodiscard]] bool validInput(const Matrix& matrix, const Vector& target, const RegressionOptions& options) noexcept{
    if (!validOptions(options) || matrix.empty() || target.size() != matrix.size())
        return false;

    const qf::Index columns = matrix.front().size();

    if (columns == 0)
        return false;

    for (const auto& row : matrix) {
        if (row.size() != columns)
            return false;

        for (const qf::Real entry : row) {
            if (!finite(entry))
                return false;
        }
    }

    for (const qf::Real entry : target){
        if (!finite(entry))
            return false;
    }

    return true;
}

[[nodiscard]] double residualTolerance(const Matrix& matrix, const Vector& weights, const RegressionOptions& options) noexcept {
    double scale = 1.0;

    for (qf::Index j = 0; j < matrix.front().size(); ++j){
        double residual = 0.0;

        for (qf::Index i = 0; i < matrix.size(); ++i){
            residual += value(matrix[i][j]) * value(weights[i]);
        }

        scale = std::max(scale, std::abs(residual));
    }

    return value(options.tolerance) * scale;
}

[[nodiscard]] bool satisfiesConstraints(const Matrix& matrix, const Vector& weights, const RegressionOptions& options) noexcept{
    const double epsilon = value(options.epsilon);
    const double tolerance = residualTolerance(matrix, weights, options);

    for (const qf::Real weight : weights)
    {
        if (value(weight) < epsilon - value(options.tolerance))
            return false;
    }

    for (qf::Index j = 0; j < matrix.front().size(); ++j) {
        double residual = 0.0;

        for (qf::Index i = 0; i < matrix.size(); ++i){
            residual += value(matrix[i][j]) * value(weights[i]);
        }

        if (std::abs(residual) > tolerance)
            return false;
    }

    return true;
}

[[nodiscard]]
double l1Objective(const Vector& weights, const Vector& target) noexcept{
    double objective = 0.0;

    for (qf::Index i = 0; i < weights.size(); ++i) {
        objective +=
            std::abs(
                value(weights[i])
                - value(target[i]));
    }

    return objective;
}

[[nodiscard]]
double l2Objective(
    const Vector& weights,
    const Vector& target) noexcept
{
    double objective = 0.0;

    for (qf::Index i = 0; i < weights.size(); ++i)
    {
        const double difference =
            value(weights[i])
            - value(target[i]);

        objective += difference * difference;
    }

    return std::sqrt(objective);
}

[[nodiscard]]
bool solveLinearSystem(
    std::vector<std::vector<double>> matrix,
    std::vector<double> rhs,
    std::vector<double>& solution,
    double tolerance) noexcept
{
    const qf::Index n = rhs.size();

    solution.assign(n, 0.0);

    if (matrix.size() != n)
        return false;

    for (const auto& row : matrix)
    {
        if (row.size() != n)
            return false;
    }

    for (qf::Index column = 0; column < n; ++column)
    {
        qf::Index pivot = column;

        double pivotMagnitude =
            std::abs(matrix[pivot][column]);

        for (qf::Index row = column + 1; row < n; ++row)
        {
            const double magnitude =
                std::abs(matrix[row][column]);

            if (magnitude > pivotMagnitude)
            {
                pivot = row;
                pivotMagnitude = magnitude;
            }
        }

        if (pivotMagnitude <= tolerance)
            return false;

        if (pivot != column)
        {
            std::swap(
                matrix[pivot],
                matrix[column]);

            std::swap(
                rhs[pivot],
                rhs[column]);
        }

        for (qf::Index row = column + 1; row < n; ++row)
        {
            const double factor =
                matrix[row][column]
                / matrix[column][column];

            if (std::abs(factor) <= tolerance)
                continue;

            matrix[row][column] = 0.0;

            for (qf::Index j = column + 1; j < n; ++j)
            {
                matrix[row][j] -=
                    factor * matrix[column][j];
            }

            rhs[row] -=
                factor * rhs[column];
        }
    }

    for (qf::Index i = n; i-- > 0;)
    {
        double value = rhs[i];

        for (qf::Index j = i + 1; j < n; ++j)
        {
            value -=
                matrix[i][j] * solution[j];
        }

        if (std::abs(matrix[i][i]) <= tolerance)
            return false;

        solution[i] =
            value / matrix[i][i];
    }

    return true;
}

class SimplexSolver {
public:
    SimplexSolver(
        const std::vector<std::vector<double>>& constraints,
        const std::vector<double>& rhs,
        const std::vector<double>& objective,
        double tolerance,
        qf::Index maxIterations)
        : rows_(static_cast<int>(rhs.size())),
          columns_(static_cast<int>(objective.size())),
          tolerance_(tolerance),
          maxIterations_(maxIterations),
          basis_(static_cast<qf::Index>(rows_)),
          nonBasis_(static_cast<qf::Index>(columns_) + 1),
          tableau_(
              static_cast<qf::Index>(rows_) + 2,
              std::vector<double>(
                  static_cast<qf::Index>(columns_) + 2,
                  0.0))
    {
        for (int i = 0; i < rows_; ++i)
        {
            for (int j = 0; j < columns_; ++j)
            {
                tableau_[static_cast<qf::Index>(i)][static_cast<qf::Index>(j)] =
                    constraints[static_cast<qf::Index>(i)][static_cast<qf::Index>(j)];
            }

            basis_[static_cast<qf::Index>(i)] =
                columns_ + i;

            tableau_[static_cast<qf::Index>(i)][static_cast<qf::Index>(columns_)] =
                -1.0;

            tableau_[static_cast<qf::Index>(i)][static_cast<qf::Index>(columns_ + 1)] =
                rhs[static_cast<qf::Index>(i)];
        }

        for (int j = 0; j < columns_; ++j)
        {
            nonBasis_[static_cast<qf::Index>(j)] = j;

            tableau_[static_cast<qf::Index>(rows_)][static_cast<qf::Index>(j)] =
                -objective[static_cast<qf::Index>(j)];
        }

        nonBasis_[static_cast<qf::Index>(columns_)] = -1;

        tableau_[static_cast<qf::Index>(rows_ + 1)][static_cast<qf::Index>(columns_)] =
            1.0;
    }

    [[nodiscard]]
    bool solve(
        std::vector<double>& solution,
        double& objective,
        qf::Index& iterations) noexcept
    {
        int row = 0;

        if (rows_ > 0)
        {
            for (int i = 1; i < rows_; ++i)
            {
                if (tableau_[static_cast<qf::Index>(i)]
                            [static_cast<qf::Index>(columns_ + 1)]
                    <
                    tableau_[static_cast<qf::Index>(row)]
                            [static_cast<qf::Index>(columns_ + 1)])
                {
                    row = i;
                }
            }
        }

        if (rows_ > 0
            && tableau_[static_cast<qf::Index>(row)]
                       [static_cast<qf::Index>(columns_ + 1)]
                < -tolerance_)
        {
            pivot(row, columns_);

            if (!simplex(1))
                return false;

            if (tableau_[static_cast<qf::Index>(rows_ + 1)]
                       [static_cast<qf::Index>(columns_ + 1)]
                < -tolerance_)
            {
                return false;
            }

            if (std::abs(
                    tableau_[static_cast<qf::Index>(rows_ + 1)]
                            [static_cast<qf::Index>(columns_ + 1)])
                > tolerance_)
            {
                return false;
            }

            for (int i = 0; i < rows_; ++i)
            {
                if (basis_[static_cast<qf::Index>(i)] != -1)
                    continue;

                int column = -1;

                for (int j = 0; j <= columns_; ++j)
                {
                    if (std::abs(
                            tableau_[static_cast<qf::Index>(i)]
                                    [static_cast<qf::Index>(j)])
                        > tolerance_)
                    {
                        column = j;
                        break;
                    }
                }

                if (column >= 0)
                    pivot(i, column);
            }
        }

        if (!simplex(2))
            return false;

        solution.assign(
            static_cast<qf::Index>(columns_),
            0.0);

        for (int i = 0; i < rows_; ++i)
        {
            const int variable =
                basis_[static_cast<qf::Index>(i)];

            if (variable >= 0 && variable < columns_)
            {
                solution[static_cast<qf::Index>(variable)] =
                    tableau_[static_cast<qf::Index>(i)]
                            [static_cast<qf::Index>(columns_ + 1)];
            }
        }

        objective =
            tableau_[static_cast<qf::Index>(rows_)]
                    [static_cast<qf::Index>(columns_ + 1)];

        iterations = iterations_;

        return true;
    }

private:
    void pivot(
        int row,
        int column) noexcept
    {
        const double inverse =
            1.0 /
            tableau_[static_cast<qf::Index>(row)]
                    [static_cast<qf::Index>(column)];

        for (int i = 0; i < rows_ + 2; ++i)
        {
            if (i == row)
                continue;

            for (int j = 0; j < columns_ + 2; ++j)
            {
                if (j == column)
                    continue;

                tableau_[static_cast<qf::Index>(i)]
                        [static_cast<qf::Index>(j)]
                    -=
                    tableau_[static_cast<qf::Index>(row)]
                            [static_cast<qf::Index>(j)]
                    *
                    tableau_[static_cast<qf::Index>(i)]
                            [static_cast<qf::Index>(column)]
                    *
                    inverse;
            }
        }

        for (int j = 0; j < columns_ + 2; ++j)
        {
            if (j != column)
            {
                tableau_[static_cast<qf::Index>(row)]
                        [static_cast<qf::Index>(j)]
                    *= inverse;
            }
        }

        for (int i = 0; i < rows_ + 2; ++i)
        {
            if (i != row)
            {
                tableau_[static_cast<qf::Index>(i)]
                        [static_cast<qf::Index>(column)]
                    *= -inverse;
            }
        }

        tableau_[static_cast<qf::Index>(row)]
                [static_cast<qf::Index>(column)]
            = inverse;

        std::swap(
            basis_[static_cast<qf::Index>(row)],
            nonBasis_[static_cast<qf::Index>(column)]);

        ++iterations_;
    }

    [[nodiscard]]
    bool simplex(int phase) noexcept
    {
        const int objectiveRow =
            phase == 1
                ? rows_ + 1
                : rows_;

        while (true)
        {
            if (iterations_ >= maxIterations_)
                return false;

            int column = -1;

            for (int j = 0; j <= columns_; ++j)
            {
                if (phase == 2
                    && nonBasis_[static_cast<qf::Index>(j)] == -1)
                {
                    continue;
                }

                if (
                    column == -1
                    ||
                    tableau_[static_cast<qf::Index>(objectiveRow)]
                            [static_cast<qf::Index>(j)]
                        <
                    tableau_[static_cast<qf::Index>(objectiveRow)]
                            [static_cast<qf::Index>(column)]
                        - tolerance_
                    ||
                    (
                        std::abs(
                            tableau_[static_cast<qf::Index>(objectiveRow)]
                                    [static_cast<qf::Index>(j)]
                            -
                            tableau_[static_cast<qf::Index>(objectiveRow)]
                                    [static_cast<qf::Index>(column)])
                        <= tolerance_
                        &&
                        nonBasis_[static_cast<qf::Index>(j)]
                        <
                        nonBasis_[static_cast<qf::Index>(column)]
                    )
                )
                {
                    column = j;
                }
            }

            if (
                tableau_[static_cast<qf::Index>(objectiveRow)]
                        [static_cast<qf::Index>(column)]
                >= -tolerance_
            )
            {
                return true;
            }

            int row = -1;

            for (int i = 0; i < rows_; ++i)
            {
                const double coefficient =
                    tableau_[static_cast<qf::Index>(i)]
                            [static_cast<qf::Index>(column)];

                if (coefficient <= tolerance_)
                    continue;

                if (row == -1)
                {
                    row = i;
                    continue;
                }

                const double currentRatio =
                    tableau_[static_cast<qf::Index>(i)]
                            [static_cast<qf::Index>(columns_ + 1)]
                    / coefficient;

                const double bestRatio =
                    tableau_[static_cast<qf::Index>(row)]
                            [static_cast<qf::Index>(columns_ + 1)]
                    /
                    tableau_[static_cast<qf::Index>(row)]
                            [static_cast<qf::Index>(column)];

                if (
                    currentRatio < bestRatio - tolerance_
                    ||
                    (
                        std::abs(
                            currentRatio - bestRatio)
                        <= tolerance_
                        &&
                        basis_[static_cast<qf::Index>(i)]
                        <
                        basis_[static_cast<qf::Index>(row)]
                    )
                )
                {
                    row = i;
                }
            }

            if (row == -1)
                return false;

            pivot(row, column);
        }
    }

    int rows_;
    int columns_;
    double tolerance_;
    qf::Index maxIterations_;
    qf::Index iterations_{0};

    std::vector<int> basis_;
    std::vector<int> nonBasis_;

    std::vector<std::vector<double>> tableau_;
};

[[nodiscard]]
qf::Result<RegressionResult>
failure()
{
    return qf::Result<RegressionResult>(
        qf::Status(qf::ErrorCode::invalid_argument , "failure"));
}

[[nodiscard]]
qf::Result<RegressionResult>
solveL1(
    const Matrix& matrix,
    const Vector& target,
    const RegressionOptions& options)
{
    const qf::Index n = matrix.size();
    const qf::Index k = matrix.front().size();

    const double epsilon =
        value(options.epsilon);

    const qf::Index variableCount = 2 * n;
    const qf::Index constraintCount =
        2 * k + 2 * n;

    std::vector<std::vector<double>> constraints(
        constraintCount,
        std::vector<double>(
            variableCount,
            0.0));

    std::vector<double> rhs(
        constraintCount,
        0.0);

    std::vector<double> objective(
        variableCount,
        0.0);

    for (qf::Index j = 0; j < k; ++j)
    {
        double columnSum = 0.0;

        for (qf::Index i = 0; i < n; ++i)
        {
            const double entry =
                value(matrix[i][j]);

            constraints[j][i] = entry;
            constraints[k + j][i] = -entry;

            columnSum += entry;
        }

        rhs[j] =
            -epsilon * columnSum;

        rhs[k + j] =
            -rhs[j];
    }

    for (qf::Index i = 0; i < n; ++i)
    {
        constraints[2 * k + i][i] = 1.0;

        constraints[2 * k + i][n + i] = -1.0;

        rhs[2 * k + i] =
            value(target[i]) - epsilon;

        constraints[2 * k + n + i][i] = -1.0;

        constraints[2 * k + n + i][n + i] = -1.0;

        rhs[2 * k + n + i] =
            epsilon - value(target[i]);

        objective[n + i] = -1.0;
    }

    SimplexSolver solver(
        constraints,
        rhs,
        objective,
        value(options.tolerance),
        options.maxIterations);

    std::vector<double> solution;
    double ignoredObjective = 0.0;
    qf::Index iterations = 0;

    if (!solver.solve(
            solution,
            ignoredObjective,
            iterations))
    {
        return failure();
    }

    RegressionResult result;

    result.weights.resize(n);

    for (qf::Index i = 0; i < n; ++i)
    {
        result.weights[i] =
            solution[i] + epsilon;
    }

    if (!satisfiesConstraints(
            matrix,
            result.weights,
            options))
    {
        return failure();
    }

    result.objective =
        l1Objective(
            result.weights,
            target);

    result.iterations = iterations;

    return qf::Result<RegressionResult>(
        std::move(result));
}

[[nodiscard]]
qf::Result<RegressionResult>
solveL2(
    const Matrix& matrix,
    const Vector& target,
    const RegressionOptions& options)
{
    const qf::Index n = matrix.size();
    const qf::Index k = matrix.front().size();

    const double epsilon =
        value(options.epsilon);

    const double tolerance =
        value(options.tolerance);

    std::vector<bool> fixed(n, false);

    for (
        qf::Index iteration = 0;
        iteration < options.maxIterations;
        ++iteration)
    {
        std::vector<qf::Index> freeIndices;

        for (qf::Index i = 0; i < n; ++i)
        {
            if (!fixed[i])
                freeIndices.push_back(i);
        }

        Vector weights(
            n,
            options.epsilon);

        if (freeIndices.empty())
        {
            if (!satisfiesConstraints(
                    matrix,
                    weights,
                    options))
            {
                return failure();
            }

            RegressionResult result;

            result.weights =
                std::move(weights);

            result.objective =
                l2Objective(
                    result.weights,
                    target);

            result.iterations =
                iteration;

            return qf::Result<RegressionResult>(
                std::move(result));
        }

        std::vector<std::vector<double>> gram(
            k,
            std::vector<double>(
                k,
                0.0));

        std::vector<double> rhs(
            k,
            0.0);

        for (qf::Index j = 0; j < k; ++j)
        {
            for (qf::Index l = 0; l < k; ++l)
            {
                for (const qf::Index i : freeIndices)
                {
                    gram[j][l] +=
                        value(matrix[i][j])
                        *
                        value(matrix[i][l]);
                }
            }

            for (const qf::Index i : freeIndices)
            {
                rhs[j] +=
                    value(matrix[i][j])
                    *
                    value(target[i]);
            }

            for (qf::Index i = 0; i < n; ++i)
            {
                if (fixed[i])
                {
                    rhs[j] +=
                        epsilon
                        *
                        value(matrix[i][j]);
                }
            }
        }

        std::vector<double> lambda;

        if (!solveLinearSystem(
                gram,
                rhs,
                lambda,
                tolerance))
        {
            return failure();
        }

        for (const qf::Index i : freeIndices)
        {
            double projected =
                value(target[i]);

            for (qf::Index j = 0; j < k; ++j)
            {
                projected -=
                    value(matrix[i][j])
                    *
                    lambda[j];
            }

            weights[i] =
                projected;
        }

        qf::Index mostViolating = n;

        double smallestWeight =
            std::numeric_limits<double>::infinity();

        for (const qf::Index i : freeIndices)
        {
            const double weight =
                value(weights[i]);

            if (weight < smallestWeight)
            {
                smallestWeight = weight;
                mostViolating = i;
            }
        }

        if (
            mostViolating != n
            &&
            smallestWeight < epsilon - tolerance
        )
        {
            fixed[mostViolating] = true;
            continue;
        }

        qf::Index release = n;

        double mostNegativeMultiplier = 0.0;

        for (qf::Index i = 0; i < n; ++i)
        {
            if (!fixed[i])
                continue;

            double multiplier =
                epsilon - value(target[i]);

            for (qf::Index j = 0; j < k; ++j)
            {
                multiplier +=
                    value(matrix[i][j])
                    *
                    lambda[j];
            }

            if (multiplier < mostNegativeMultiplier)
            {
                mostNegativeMultiplier =
                    multiplier;

                release = i;
            }
        }

        if (release != n)
        {
            fixed[release] = false;
            continue;
        }

        if (!satisfiesConstraints(
                matrix,
                weights,
                options))
        {
            return failure();
        }

        RegressionResult result;

        result.weights =
            std::move(weights);

        result.objective =
            l2Objective(
                result.weights,
                target);

        result.iterations =
            iteration + 1;

        return qf::Result<RegressionResult>(
            std::move(result));
    }

    return failure();
}

} // namespace

qf::Result<RegressionResult>
l1Positive(
    const Matrix& matrix,
    const Vector& target,
    const RegressionOptions& options)
{
    if (!validInput(
            matrix,
            target,
            options))
    {
        return failure();
    }

    return solveL1(
        matrix,
        target,
        options);
}

qf::Result<RegressionResult>
l2Positive(
    const Matrix& matrix,
    const Vector& target,
    const RegressionOptions& options)
{
    if (!validInput(
            matrix,
            target,
            options))
    {
        return failure();
    }

    return solveL2(
        matrix,
        target,
        options);
}

} // namespace qf::optimization
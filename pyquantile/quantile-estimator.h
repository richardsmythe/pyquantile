#pragma once
#include <tuple>
#include <vector>
#include <limits>
#include <cmath>

class QuantileEstimator {
public:
    explicit QuantileEstimator(double p);
    void Add(double s);
    double GetQuantile() const;
    std::tuple<double, double, double, double, double> GetCurrentMarkers() const;

private:
    void SetIncrements(double p);
    void SortMarkers();
    void ValidateMarkerPositions();
    void UpdateMarkers(double s);
    void AdjustMarkers();
    double CalculateParabolicValue(
        double markerPrev, double markerCurrent, double markerNext,
        double posPrev, double posCurrent, double posNext, double delta);

    static constexpr double EPSILON = std::numeric_limits<double>::epsilon() * 100;

    double p;
    long long N = 0;
    double Q0 = 0, Q1 = 0, Q2 = 0, Q3 = 0, Q4 = 0;
    int n0 = 0, n1 = 1, n2 = 2, n3 = 3, n4 = 4;
    double pos0 = 0, pos1 = 1, pos2 = 2, pos3 = 3, pos4 = 4;
    double desPos0 = 0, desPos1 = 1, desPos2 = 2, desPos3 = 3, desPos4 = 4;
    double inc0 = 0, inc1 = 0, inc2 = 0, inc3 = 0, inc4 = 0;
    double initial_buffer[5];
};

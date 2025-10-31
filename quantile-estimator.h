#pragma once
#include <tuple>
#include <vector>

class QuantileEstimator {
public:
    QuantileEstimator(double p);
    void Add(double s);
    double GetQuantile() const;
    std::tuple<double, double, double, double, double> GetCurrentMarkers() const;

private:
    double p;
    int N = 0;
    double Q0, Q1, Q2, Q3, Q4;
    double pos0 = 1, pos1 = 2, pos2 = 3, pos3 = 4, pos4 = 5;
    double desPos0, desPos1, desPos2, desPos3, desPos4;
    double inc0, inc1, inc2, inc3, inc4;
    bool debugMode = false;
    int debugInterval = 1000;
    void SetIncrements(double p);
    void SortMarkers();
    void ValidateMarkerPositions();
    void AdjustMarkers();
    double CalculateParabolicValue(
        double markerPrev, double markerCurrent, double markerNext,
        double posPrev, double posCurrent, double posNext, double delta);
    void UpdateMarkers(double s);
};

#include "quantile-estimator.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>

QuantileEstimator::QuantileEstimator(double p)
    : N(0), p(p) {
    if (p < 0 || p > 1) throw std::invalid_argument("p must be between 0 and 1.");
    SetIncrements(p);
    initialValues.reserve(5);
}

void QuantileEstimator::SetIncrements(double p) {
    inc0 = 0.0;
    inc1 = p / 2.0;
    inc2 = p;
    inc3 = (1.0 + p) / 2.0;
    inc4 = 1.0;

    desPos0 = pos0;
    desPos1 = pos1;
    desPos2 = pos2;
    desPos3 = pos3;
    desPos4 = pos4;
}

void QuantileEstimator::Add(double s) {
    if (N < 5) {
        initialValues.push_back(s);
        N++;

        if (N == 5) {
            std::sort(initialValues.begin(), initialValues.end());
            Q0 = initialValues[0];
            Q1 = initialValues[1];
            Q2 = initialValues[2];
            Q3 = initialValues[3];
            Q4 = initialValues[4];

            pos0 = 1.0; pos1 = 2.0; pos2 = 3.0; pos3 = 4.0; pos4 = 5.0;

            desPos0 = 1.0;
            desPos1 = 1.0 + 4.0 * inc1;
            desPos2 = 1.0 + 4.0 * inc2;
            desPos3 = 1.0 + 4.0 * inc3;
            desPos4 = 5.0;

            initialValues.clear();
            initialValues.shrink_to_fit();
        }
        return;
    }
    N++;
    UpdateMarkers(s);
}

void QuantileEstimator::UpdateMarkers(double s) {
    if (s < Q0) {
        Q0 = s;
        pos1++; pos2++; pos3++; pos4++;
    }
    else if (s < Q1) {
        pos1++; pos2++; pos3++; pos4++;
    }
    else if (s < Q2) {
        pos2++; pos3++; pos4++;
    }
    else if (s < Q3) {
        pos3++; pos4++;
    }
    else if (s < Q4) {
        pos4++;
    }
    else {
        Q4 = s;
    }

    desPos0 += inc0;
    desPos1 += inc1;
    desPos2 += inc2;
    desPos3 += inc3;
    desPos4 += inc4;

    AdjustMarkers();
}

double QuantileEstimator::CalculateParabolicValue(
    double markerPrev, double markerCurrent, double markerNext,
    double posPrev, double posCurrent, double posNext, double d) {

    double d0 = posCurrent - posPrev;
    double d1 = posNext - posCurrent;

    double term1 = d * (markerNext - markerCurrent) / d1;
    double term2 = d * (markerCurrent - markerPrev) / d0;
    double estimate = markerCurrent + (term1 + term2) / 2.0;

    if (estimate <= markerPrev || estimate >= markerNext) {
        double slope = (d > 0) ?
            (markerNext - markerCurrent) / d1 :
            (markerCurrent - markerPrev) / d0;

        estimate = markerCurrent + d * slope;
    }
    return std::max(markerPrev, std::min(markerNext, estimate));
}

double QuantileEstimator::CalculateLinearValue(
    double markerPrev, double markerCurrent, double markerNext,
    double posPrev, double posCurrent, double posNext, double d) {
    double slope;
    if (d > 0) {
        slope = (markerNext - markerCurrent) / (posNext - posCurrent);
    }
    else {
        slope = (markerCurrent - markerPrev) / (posCurrent - posPrev);
    }

    return markerCurrent + d * slope;
}


void QuantileEstimator::AdjustMarkers() {
    for (int i = 1; i <= 3; i++) {
        double delta = 0;
        switch (i) {
        case 1: delta = desPos1 - pos1; break;
        case 2: delta = desPos2 - pos2; break;
        case 3: delta = desPos3 - pos3; break;
        }

        double markerPrev, markerCurrent, markerNext;
        double posPrev, posCurrent, posNext;
        switch (i) {
        case 1:
            markerPrev = Q0; markerCurrent = Q1; markerNext = Q2;
            posPrev = pos0; posCurrent = pos1; posNext = pos2;
            break;
        case 2:
            markerPrev = Q1; markerCurrent = Q2; markerNext = Q3;
            posPrev = pos1; posCurrent = pos2; posNext = pos3;
            break;
        default:
            markerPrev = Q2; markerCurrent = Q3; markerNext = Q4;
            posPrev = pos2; posCurrent = pos3; posNext = pos4;
            break;
        }

        if ((delta >= 1.0 && (posNext - posCurrent) > 1.0) || (delta <= -1.0 && (posCurrent - posPrev) > 1.0)) {

            double d = std::copysign(1.0, delta);
            double parabolicValue = CalculateParabolicValue(markerPrev, markerCurrent, markerNext,posPrev, posCurrent, posNext, d);
            double linearValue = CalculateLinearValue(markerPrev, markerCurrent, markerNext,posPrev, posCurrent, posNext, d);

            // N-Dependent factor
            // this is the weight used for the parabolic interpolation, it increases as N grows
            double weight = std::min(1.0, N / N_THRESHOLD);

            // now we blend linear and parabolic values
            double blended_interpolations = (weight * parabolicValue) + ((1.0 - weight) * linearValue);

            // make sure it remains between it's neighbours
            blended_interpolations = std::max(markerPrev, std::min(markerNext, blended_interpolations));

            switch (i) {
            case 1:
                pos1 += d;
                Q1 = std::max(Q0, std::min(Q2, blended_interpolations));
                break;
            case 2:
                pos2 += d;
                Q2 = std::max(Q1, std::min(Q3, blended_interpolations));
                break;
            case 3:
                pos3 += d;
                Q3 = std::max(Q2, std::min(Q4, blended_interpolations));
                break;
            }
        }
    }
}

double QuantileEstimator::GetQuantile() const {
    if (N < 5) {
        if (initialValues.empty()) {
            return 0.0;
        }

        std::vector<double> temp = initialValues;
        std::sort(temp.begin(), temp.end());
        int index = static_cast<int>(p * (temp.size() - 1) + 0.5);
        return temp[std::max(0, std::min((int)temp.size() - 1, index))];
    }
    return Q2;
}

std::tuple<double, double, double, double, double> QuantileEstimator::GetCurrentMarkers() const {
    return { Q0, Q1, Q2, Q3, Q4 };
}
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  
#include "quantile-estimator.h" 

namespace py = pybind11;

PYBIND11_MODULE(pyquantile, m) {
    m.doc() = "A fast C++ P-Square quantile estimator for streaming data.";

    // Expose the QuantileEstimator class and its methods
    py::class_<QuantileEstimator>(m, "QuantileEstimator")
        // Constructor: QuantileEstimator(double p)
        .def(py::init<double>(), py::arg("p"),
            "Initializes the estimator for quantile 'p' (0 to 1).")

        // Method: Add(double s) -> exposed as .add()
        .def("add", &QuantileEstimator::Add, py::arg("value"),
            "Adds a single data point to the stream.")

        // Method: GetQuantile() const -> exposed as .quantile()
        .def("quantile", &QuantileEstimator::GetQuantile,
            "Returns the current estimated quantile for 'p'.")

        // Method: GetCurrentMarkers() const -> exposed as .get_markers()
        .def("get_markers", &QuantileEstimator::GetCurrentMarkers,
            "Returns the current markers (Q0, Q1, Q2, Q3, Q4) as a tuple.");
}
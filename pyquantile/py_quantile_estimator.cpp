//// UNCOMMENT FOR BUILDING AND DISTRIBUTING
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>  
//#include "quantile-estimator.h" 
//
//namespace py = pybind11;
//
//PYBIND11_MODULE(_pyquantile, m) { 
//    m.doc() = "A fast C++ P-Square quantile estimator for streaming data.";
//
//
//    py::class_<QuantileEstimator>(m, "QuantileEstimator")
//
//        .def(py::init<double>(), py::arg("p"),
//            "Initializes the estimator for quantile 'p' (0 to 1).")
//
//
//        .def("add", &QuantileEstimator::Add, py::arg("value"),
//            "Adds a single data point to the stream.")
//
//        .def("quantile", &QuantileEstimator::GetQuantile,
//            "Returns the current estimated quantile for 'p'.")
//
//
//        .def("get_markers", &QuantileEstimator::GetCurrentMarkers,
//            "Returns the current markers (Q0, Q1, Q2, Q3, Q4) as a tuple.");
//}
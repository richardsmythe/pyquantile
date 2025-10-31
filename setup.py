from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "pyquantile",
        ["py_quantile_estimator.cpp", "quantile-estimator.cpp"],
        cxx_std=14,
        extra_compile_args=["/bigobj", "/std:c++14"],
    ),
]

setup(
    name="pyquantile",
    version="0.1.0",
    description="A fast quantile estimator for streamed data without any data points having to be stored.",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    author="Richard Smythe",
    license="MIT",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
    install_requires=["numpy", "pybind11"],
)
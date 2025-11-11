from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "_pyquantile",
        [
            "pyquantile/py_quantile_estimator.cpp",
            "pyquantile/quantile-estimator.cpp"
        ],
        cxx_std=14,
        include_dirs=["pyquantile"],
        extra_compile_args=["/bigobj", "/std:c++14"],
    ),
]

setup(
    name="pyquantile",
    version="0.2.0",  
    description="A fast quantile estimator for streamed data without any data points having to be stored.",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    author="Richard Smythe",
    license="MIT",
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
    install_requires=["numpy", "pybind11"],
    include_package_data=True,
    package_data={"": ["quantile-estimator.h"]},
)
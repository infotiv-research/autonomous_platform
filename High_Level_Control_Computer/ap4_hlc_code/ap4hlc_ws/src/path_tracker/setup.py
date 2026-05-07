from setuptools import find_packages, setup
from glob import glob
import os

package_name = "path_tracker"

setup(
    name=package_name,
    version="0.1.0",
    packages=[package_name],
    data_files=[
        ("share/" + package_name, ["package.xml"]),
        (os.path.join("share", package_name, "launch"), glob("launch/*.py")),
    ],
    install_requires=["setuptools", "numpy", "matplotlib"],
    zip_safe=True,
    maintainer="your_name",
    maintainer_email="your@email.com",
    description="Tracks and compares planned and actual robot paths",
    license="MIT",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "path_recorder = path_tracker.path_recorder:main",
            "odom_recorder = path_tracker.odom_recorder:main",
        ],
    },
)

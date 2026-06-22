from setuptools import find_packages, setup
from glob import glob
import os

package_name = "odom_publisher"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        (os.path.join("share", package_name, "launch"), glob("launch/*.py")),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="ap4-dev-laptop",
    maintainer_email="marten.postma@infotiv.se",
    description="TODO: Package description",
    license="TODO: License declaration",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "odom_publisher = odom_publisher.odom_publisher:main",
            "odom_ackermann = odom_publisher.odom_ackermann:main",
        ],
    },
)

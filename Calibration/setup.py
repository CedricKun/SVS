from pathlib import Path
import pkg_resources as pkg
from setuptools import find_packages, setup

# Settings
FILE = Path(__file__).resolve()
PARENT = FILE.parent  # root directory
REQUIREMENTS = [f'{x.name}{x.specifier}' for x in pkg.parse_requirements((PARENT / 'requirements.txt').read_text())]

setup(
    name='avmg',  # name of pypi package
    version='0.0.1',   
    description='Fisheye Camera Calibration Toolbox',
    python_requires='>=3.8',
    license='AGPL-3.0',
    author='6635120',
    author_email='luo.kuncheng@byd.com',
    packages=find_packages(),  # required
    include_package_data=True,
    install_requires=REQUIREMENTS,
)
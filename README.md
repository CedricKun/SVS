## <div align="center">SVS Documentation</div>
SVS 是一个基于OpenCV的鱼眼相机标定与数据采集系统，包含主目录下用于鱼眼相机内外参标定的Python模块 **Calibration** 与用于数据采集的C++模块。

<details open>
<summary>Calibration</summary>
标定模块中标定参数主要包括*内参（成像矩阵K和畸变参数D）、外参（相机位姿R, t或单应矩阵H）*，内参与相机位姿的标定方法主要参考ZJU的 [easymocap](https://github.com/zju3dv/EasyMocap) 项目，单应矩阵主要参考neozhangliang的 [surround-view-system](https://github.com/neozhaoliang/surround-view-system-introduction) 项目，本项目在上述开源项目的基础上进行了删减以适配鱼眼相机配置。
#### Install
···
# conda环境配置
conda create -n svs python=3.9
conda activate svs
# 模块安装
cd Calibration
pip install -r requirements.txt
python 
···

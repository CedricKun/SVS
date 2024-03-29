# 临时使用网络编程，先禁用再开启网卡通讯。
sudo ifconfig eth0 down
sudo ifconfig eth1 down
sudo ifconfig eth2 down
sudo ifconfig eth3 down
sudo ifconfig eth4 down
sudo ifconfig eth5 down
sleep 1
sudo ifconfig eth4 192.168.55.100/24 up

# 配置相机外触发命令，需要每次AVM项目启动时设置一次。
sudo tztek-jetson-tool-cpld-test -d /dev/ttyTHS1 -t 4 -c 8 -f 20 -w 1000 -o 0
# 配置相机服务命令, 每次AVM项目启动时设置一次。
sudo systemctl restart jetson-cam-cfg.service
sudo systemctl status jetson-cam-cfg.service
# build AVM可执行文件
# mkdir build
# cd build
# cmake ..
# make -j8



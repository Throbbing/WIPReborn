# 重构[WIP](https://github.com/wubugui/WIP)

-  彻底重写渲染系统
-  重写应用类
-  精简帧动画
-  重写场景管理
-  添加必要的系统：文件系统等
-  增强可拓展性
-  支持linux
-  采用CMAKE构建

# TODO

-  修改SoftR并代替OpenGL成为RHI
-  ~重制JavaTank~
-  粒子系统
-  角色阴影
-  反射系统
-  ~事件驱动Object重构，消除各级Update~
-  事件驱动UI
-  以事件驱动方式封装IMGUI
-  ~内存，智能指针~
-  完善动画系统(~帧动画~，状态机，transform动画，动画事件等)
-  Profile与性能优化
-  D3D11支持
-  基本编辑器
-  RPG游戏实例
-  ~高效Text渲染~
-  ~脚本系统~
-  ~物理(已经初步集成Box2D)~
-  ~声音(FMOD)~
-  ~嵌入IMGUI~

# 编译

-  移除所有UI相关的代码.
-  参考CMakelists.txt在对应的目录配置库.
-  使用CMAKE
   -  WIN32生成VS2013.
   -  Linux生成Unix style make,直接make.

# Demo

-  [Release](https://github.com/wubugui/WIPReborn/releases)

-  WASD移动，鼠标滚轮缩放镜头，X清空所有地图碰撞，鼠标左键放置地图碰撞
   
# 截图

![](https://github.com/wubugui/WIPReborn/raw/master/example/1.png)
![](https://github.com/wubugui/FXXKTracer/raw/master/pic/ll.gif)


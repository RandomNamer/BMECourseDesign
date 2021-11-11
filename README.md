<h2 align = "center" style="font-family: fantasy;">BMECourseDesign <a style="font-family: monospace; font-size:28px; font-weight: 500;"href="./README-en.md">English Doc</a></h2>

### 介绍

上海交通大学生物医学工程学院课程设计——影像方向，课程目标包括使用开源软件搭建微型PACS系统和解决生物医学工程实践中的实际问题。

### Task 1

修改并补充一个使用Microsoft Visual C++的位图处理程序。

### Task 2

使用DCMTK的实用工具，利用日志和网络抓包分析DICOM标准。

### Task 3

使用DCMTK的API编写PACS系统。

#### SCU 

**入口文件：**[scu.cpp](3/src/scu.cpp)

**实现功能：**

- 实现C-STORE请求
- 根据文件元数据自动匹配Presentation Context

#### SCP

**入口文件**：[scp.cpp](3/src/scp.cpp)

**实现功能：**

- 实现支持C-STORE请求的SCP客户端
- 实现对图像的几种处理，图像翻转、图像任意角度旋转、图像自动转JPEG压缩。
- 实现sqlite3数据库记录




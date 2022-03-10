# BerryDB

一个以学习为目的的NoSQL分布式数据，使用Java和C/C++构建。

<div align=center>
    <img src="logo.png"/>
</div>

# 概念与特性

BerryDB支持 Openrc 和 Systemd 管理, 通过 `tools` 目录的脚本生成。

## 概念

- 客户端：berrycli通过Hash的方式连接到数据分片。
- 引擎调度单元：执行客户端请求的线程，BerryDB构建了线程池，并且通过主线程实现引擎调度单元的管理。
- 引擎调度单元管理器：是线程池封装，实现对引擎调度单元的申请和归还。包含了正在执行客户端任务的执行队列和空闲的等待队列。
- 操作系统服务层：实现异构系统的跨平台支持，例如提供引擎调度单元管理器中的队列实现、mmap的跨平台实现等。
- 代理线程：处理用户请求的任务单元，是一种引擎调度单元的具体实现。
- 消息转换：BerryDB基于BSON和TCP实现了结构紧凑的数据传输协议。客户端和数据库实例通过消息传输数据。
- 存储引擎：BerryDB旨在教学与学习，因此没有使用任何第三方存储引擎。
    - 数据页：是BerryDB的逻辑单位。 数据页分为数据区和元数据区，元数据区可以用来分辨数据的类型：数据，索引。数据页中的位图表明数据区的哪些槽（大小为4B）被使用。
    - 数据块：不定长的大小，最小不低于一个数据页的大小，最大不超过一个数据块的大小。数据块无法跨数据段。在数据块中的头部会开辟指定的大小（4MB = 1024B x 4096）存储元数据，每一个bit位标识数据页是否被占用。
    - 数据段：现阶段的实现为128MB，由于BerryDB实现了跨平台的mmap，带来的副作用是mmap无法更改已经加载的数据。因此BerryDB将数据库文件切分成多个数据段，以数据段为单位使得mmap增长和换出。
- 索引管理器：用于管理Hash索引（一致性Hash），因此BerryDB无法实现范围匹配。BerryDB的实现为纯内存Hash索引，每次berryd启动后由索引管理器实现数据扫描并加载到内存。
- 运行时：将索引管理器、存储引擎、操作系统服务层等融合。
- 问题诊断：实现客户端与数据库实例的日志输出。
- 驱动程序：客户端的实现并非是跨平台的。驱动使用Java构建，是跨平台的。
- 监控器：BerryDB使用Java和AWT构建了图形化的监控程序，通过图表可以查看数据库实例状态。

## 特性

- 支持简单的数据分片和集群多节点，是一个简化版的MPP架构。
- 不存在表与集合的概念。
- 数据格式为JSON，通过json-cpp库实现JSON到BSON的转换。
- 可以对数据进行增删改查。仅支持 `_id` 方式查询。
- 客户端将数据按照 `_id` 字段发送到相应的节点。

# 构建

- 克隆仓库到本地

```shell
git clone https://github.com/bluemiaomiao/berrydb.git
```

- 执行环境就绪脚本

```shell
# 你需要指定本地Git存储库位置
berrydb/tools/prepare-build-system.sh [LOCAL_BERRYDB_GIT_REPOS_DIR]
```

- 构建可执行目标

```shell
cd berrydb/build
cmake ..
make -j # 使用并行加快编译速度, 如果你不想 make 自动确定并行值, 请使用 make -j [CPU_CORE_NUMBER]
```

# 测试

- 启动服务端

```shell
systemctl start berrydb
```

或者:

```shell
/etc/init.d/berrydb start
```

- 使用客户端连接到数据库实例

```shell
berrycli

# 在BerryCli中执行
Welcome to BerryDB Shell.
db help for help, Ctrl(Command)+C or quit to exit
db> connect [INSTANCE_IPV4] [INSTANCE_PORT]
db> quit
```

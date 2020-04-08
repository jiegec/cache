# Cache Analysis

## 实验要求

用 C/C++ 编写一个 Cache 模拟器，输入访存 trace，输出统计信息和 Hit/Miss 的记录。

## 实现方法

### 运行方式

采用 C++11 实现了实验要求，用 CMake 进行构建管理。编译方法：

```shell
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```

然后调用 `./cache` 获取一个 trace 对应的结果：

```shell
$ ./cache ../trace/astar.trace
```

它会多线程计算各种情况（Block 大小、相连度、替换算法、写策略）下 Cache 的行为，然后写入到和 trace 统一目录的若干文件中，文件名格式如下：

```
trace_${BLOCKSIZE}_${ALGO}_${HIT}_${MISS}_${ASSOC}.{info,trace}
BLOCKSIZE: 块大小，有 8 32 64 三种
ALGO：替换算法，0代表LRU，1代表随机，2代表二叉树（PLRU）
HIT：写命中策略，0代表Writethrough，1代表Writeback
MISS：写缺失策略，0代表Write-Allocate，1代表Write-Nonallocate
ASSOC：相连度，有全相连，直接映射，4-way和8-way
info：描述了Cache的一些参数和信息
trace：描述了Cache的Hit/Miss历史
```

提交的 `trace` 目录里附带了若干个以 `_8_0_1_0_8.{info,trace}` 结尾的文件，即为实验所需要的块大小为 8B，8-way组关联，LRU策略，写分配+写回情况下各个重点 trace 的访问历史。Info 文件内记录了参数、元数据的大小和缺失率等信息。

### 代码组织

代码主要分为两部分，一个是 bit vector 的实现，另一个就是 cache 的实现。

#### Bit Vector

由于实验要求用尽量少的存储空间和位运算，所以编写了一个简单的 BitVec，通过位运算来取出中间的某些位，或者写入中间的某些位。为了保证正确性，也是编写了一个简单的 fuzz 测试，把 BitVec 输出的结果与 `std::vector<bool>` 对比。

#### Cache

Cache 代码都在 `cache.{h,cpp}` 文件中，有非常多的代码注释，对于数据结构和组织和代码的功能都有具体的解释。

##### 数据结构

首先是 CacheLine ，它保存了三个信息：dirty valid 和 tag，存储在一个 BitVec 中，最低位是 dirty，次低位是 valid，其余都是 tag。通过 BitVec 的相关函数进行操作。

然后是 LRUState，它保存了 LRU 的信息，是一个 n * log2(n) 位的 BitVec，其中 n 是相连度。从低到高每 log2(n) 位为一个元素，代表组中的一个位置，每次取最高的 log2(n) 作为被替换的位置，当访问一个 valid 的块的时候，则把这块挪到最前面。初始情况下，当还有剩余的块没有占用的时候，应当从这些块里面选择一个来填入，这里的实现方法是预先按照倒序插入到 LRUState 中，这样每次从最高位取的时候自然就是取得编号最低的未占用的块。

最后一个单独的数据结构是 PLRUState，保存了二叉树替换算法（我还是更习惯叫它 Pseudo LRU）。最低位保存的是是否所有的块都是 valid，如果没有valid则需要找到一个未占用的；之后则是一个二叉树，对于下标为 k 的结点，它的左子树下标是 `2*k` ，右子树下标是 `2*k+1` ，这样一共 PLRUState 的大小就等于相连度。

##### Trace 解析

编写了一个单独的函数 `readTrace` 从文件里解析，把文本格式转换为 Trace 结构题。不用多说。

##### 读写处理

读入 Trace 以后，按照每条 trace 是读是写进行分别处理。

如果是读操作：

1. 先按照地址找到对应的组
2. 在组里面寻找有没有 tag 匹配并且 valid 的块
3. 如果有，就是一个 Hit，然后更新数据结构
4. 如果没有，就是一个 Miss，按照算法寻找一个 victim
5. 更新 tag 为当前这次访问

如果是写操作：

1. 先按照地址找到对应的组
2. 在组里面寻找有没有 tag 匹配并且 valid 的块
3. 如果有，就是一个 Hit，因为不涉及数据，无论是 Writeback 还是 Writethrough ，和读操作一样进行更新。
4. 如果没有，就是一个 Miss，按照 Write Miss Policy 进行处理：如果 No allocate，就没有后续的操作了；如果 Write Allocate，那么先进行一次读操作，再处理 dirty。

在这个过程中，只有两个地方涉及到了替换算法：1. Hit 的情况下状态的更新 2. victim 的选取。只要在这两个地方进行判断即可。

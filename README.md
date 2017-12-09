## Quick Start

### Mac

1. mkdir xcode_build

2. cmake .. -G 'Xcode'

3. set the working directory to project's root directory

4. run


### Windows (32 bits)

1. mkdir vs_build

2. cmake ..

3. open [Configuration Properties -> Debugging]

3. set working directory: $(ProjectDir)/../

4. set environment: PATH=./lib/32/;$(Path)

The third party libraries may need to be re-compile:

SOIL.lib

assimp.lib

freetype.lib

SDL2.lib and SDL2main.lib (SDL2-2.0.4)



render loop:
    设置材质状态
    batched drawcall
render loop end.

同材质的必然是同一批drawcall（减少材质状态切换）
同一批drawcall可能由少数几个drawcall-Instance（3D游戏做mesh batch代价过高，用instance就好）

static的object必然可以合并成一个drawcall，且对象状态不再需要更新
dynamic的object合并成drawcall后，每一帧都可能会修改某些object的对象状态，如位置、朝向
所以动态的object数不能太多

objMesh：绑定了mesh的信息
objScene: 多个sceneObj可引用同一个objMeshes

同材质的sceneObj才考虑合并：

1.先找出同材质的所有sceneObj

2.合并引用同一个objMeshes的sceneObj

比如有2个红茶壶、2个黄茶壶、2个红杯子、2个黄杯子
存在2个材质：红、黄
红材质的有：2个红茶壶、2个红杯子
2个红茶壶可以合并成一个drawcall（instance)
2个红杯子可以合并成一个drawcall（instance)
2个黄茶壶可以合并成一个drawcall（instance)
2个黄杯子可以合并成一个drawcall（instance)
drawcall从8个变成4个

objBatch：将多个同材质、同SubMesh的objScene合并，并创建matrix数组用来存各个objScene的modelMat
batch流程：
1.找出所有要渲染的staic obj，拆出一系列SubMesh、SetttingID(同SubMesh对应的材质不一定一样）, 根据MeshID分类:
    map<SettingID, map<<tuple<MeshID, SubMeshIdx>, sceneObjID>>

2.创建batchObj，并添加batchInfoCom，建立ModelMatrix数组
3.创建InstanceBuffer，把数组数据刷进buffer
4.单个sceneObj的SpatialData改变时，通知batchInfoCom做update，刷数据到buffer
动态和静态：
静态的scene可以同时一起做batch
动态的，每次添加、删除时更新gatch

渲染队列：排序的是batchObj，根据材质ID排


# 图生成与分解器

UI 界面重构版本（FLTK ver.）

## 公共读取器

```cpp
set<int> nodeSet;			// 孤立节点
map<int, vector<edge>> adjListGraph;	// 邻接表图，不包含孤立节点的连通部分，连通点可能为发出空边集合。
```

## 图生成器


## 图分解器

仿照 Kerninghan-Lin 算法，改进为有向边。
对于两节点有向边的损失，可以作为无向边处理。
$$c_{ab} = c_{\overline{ab}}+c_{\overline{ba}}$$

**外损失**：
$$E_a=\sum_{y \in B}c_{ay}$$
**内损失**：
$$I_a=\sum_{x \in A}c_{ax}$$

$$D_z = E_z - I_z, \forall z \in S$$

损失缩减量
$$g_{ab} = D_a + D_b - 2c_{ab}$$

使用一个队列逐渐二分。


## 图预览(选做)
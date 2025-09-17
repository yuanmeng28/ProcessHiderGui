# ProcessHiderGui

> 一个用于隐藏 Windows 进程的工具，并实现驱动与 Win32 窗口交互。

![License](https://img.shields.io/badge/license-MIT-green)  

## 原理
通过将内核对象 EPROCESS 结构从 LIST_ENTRY 双向链表中移除从而达到隐藏进程的目的。

## 关于
第一次做开源，有很多做不好的地方（大佬轻喷）(>_<)  
有一些功能比如：隐藏进程后如何恢复  
因为太麻烦了，要驱动动态内存，所以没做，有时间的话再去更新。  
如果你可以做直接PR就行

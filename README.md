Vulkan Grass Rendering
======================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 2**

* Lijun Qu
  * [LinkedIn](https://www.linkedin.com/in/lijun-qu-398375251/), [personal website](www.lijunqu.com), etc.
* Tested on: Windows 11, i7-14700HX (2.10 GHz) 32GB, Nvidia GeForce RTX 4060 Laptop

![](img/Result.gif)

---

## Overview
This project implements a **real-time grass simulator and renderer** using **Vulkan**, based on  
[Responsive Real-Time Grass Rendering for General 3D Scenes](https://www.cg.tuwien.ac.at/research/publications/2017/JAHRMANN-2017-RRTG/JAHRMANN-2017-RRTG-draft.pdf).

Each grass blade is modeled as a **Bezier curve**, simulated via compute shaders and rendered with tessellation for smooth curvature and dynamic level-of-detail.  
I further implemented **interactive grass control (WASD)** and **LOD visualization** to enhance both realism and debuggability.

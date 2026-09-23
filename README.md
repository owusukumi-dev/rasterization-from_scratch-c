# 3D Software Rasterization Pipeline from Scratch in C

A software rasterizer built from scratch in **C**, using SDL2 only as the window and framebuffer presentation layer.
No OpenGL, Vulkan, DirectX, or external math libraries are used. The geometry processing, transformations, visibility calculations, lighting, and rasterization are all handled by the CPU.


## Features

* Direct framebuffer memory manipulation
* Software alpha blending
* Bresenham line rasterization
* Scanline triangle rasterization
* 2D kinematics and collision physics
* 3D vector mathematics
* Euler rotations (pitch, yaw, roll)
* Perspective projection
* Surface normal calculation
* Backface culling
* Lambertian lighting
* Solid 3D triangle rendering


## Rendering Pipeline

```text
3D Vertices
     ↓
Euler Rotation
     ↓
Perspective Projection
     ↓
Backface Culling
     ↓
Surface Normals
     ↓
Lambertian Lighting
     ↓
Triangle Rasterization
     ↓
Framebuffer
     ↓
SDL2 Texture
     ↓
Display
```

SDL2 is used to create the window and present the final framebuffer. The actual rendering pipeline is implemented manually.

---

## Development Milestones
The renderer was built incrementally, with each file representing a major step in the process.

### `graphics1.c` — Starting with the Framebuffer
Started with raw memory: allocating the framebuffer, calculating pixel positions manually, writing pixels directly, and pushing the result to an SDL2 texture.

### `graphics2.c` — Making Pixels Form Shapes
Moved into 2D rasterization. Added software alpha blending, scanline triangle filling, and basic kinematics and collision physics.

### `graphics3.c` — Going Into 3
Introduced Bresenham line drawing, 3D vectors, Euler rotations, and perspective projection to turn 3D coordinates into something that could actually be drawn on a 2D screen.

### `main.c` — Putting It All Together
The final stage combines the previous work into a solid 3D renderer with surface normals, backface culling, Lambertian lighting, and triangle rasterization.

## Core Mathematics

### Perspective Projection

A 3D point is projected onto the screen using perspective division:

$$
x_{screen} =
\frac{x \times fov}{z}
+
\frac{W}{2}
$$

$$
y_{screen} =
\frac{y \times fov}{z}
+
\frac{H}{2}
$$

The division by `z` produces the perspective effect, making distant objects appear smaller.

### Surface Normals

The normal of a triangle is calculated using the cross product:

$$
N = (B-A) \times (C-A)
$$

### Lambertian Lighting

The surface normal is compared with the light direction using a dot product:

$$
I = \max(0, \hat{N} \cdot \hat{L})
$$

where both vectors are normalized.

---

# Why I Built This

I started out of curiosity.
I was looking at CSS one day and thought: **how does this actually create shapes?**
I can tell a computer to draw something and it just appears on the screen. But what is actually happening underneath?
That question eventually turned into a much bigger one:

> **How does information move from hardware, through software, and eventually become something I can see on a screen?**

This project became my attempt to understand the **"presented back to us"** part of that chain.
Instead of using a graphics API to hide the process, I wanted to build the process myself.

---

## What Surprised Me

Honestly, how much had to be done manually.
Coming from higher-level languages, I wasn't used to thinking about memory at this level. Allocating it, pointing to the right location, manipulating individual pixels, and making sure everything gets cleaned up properly made the difference between *"draw a triangle"* and actually understanding what drawing a triangle means very obvious.
The heap and the stack came at me aggressively during this project.

---

## The Hardest Part

The hardest part was definitely the transition from **2D to 3D**.
A 2D screen gives you pixels at `(x, y)`. Suddenly introducing a third dimension means figuring out how 3D coordinates, rotations, camera position, perspective, and surface orientation eventually become those same two screen coordinates.
Lighting was another major challenge, particularly getting the mathematics and shading behaviour consistent across different faces.
That part exposed some gaps in my mathematical foundation, so rather than hiding the problem behind more code, I decided to strengthen the underlying math and return to it with a better understanding.

---

## What I Learned

The biggest lesson wasn't a specific algorithm.

It was learning to approach problems by asking:

> **"How can this be achieved?"**

rather than immediately asking:

> **"What's the answer?"**

I spent a lot of time breaking problems down, working through the mathematics, debugging the implementation, and using AI as a tutor when I got stuck.

The goal wasn't to copy an implementation. It was to understand the reasoning well enough to reproduce it myself.

That shift in how I approach problems was probably more valuable than the renderer itself.

---

# What's Next

There are still several things I'd like to explore:

* Strengthen the mathematics behind lighting and shading
* Texture mapping
* Z-buffering and depth testing
* View-frustum clipping
* Shadow casting
* More advanced shading
* Performance optimization
* Rendering larger and more complex scenes

---

# Building

## Requirements

* GCC or Clang
* SDL2 development libraries
* Standard C library
* Math library

## Compile

```bash
gcc -Wall -O2 main.c -lSDL2 -lm -o engine
```

## Run

```bash
./engine
```

---

## Project Status

This project is primarily an **educational implementation** rather than a production graphics engine.

The goal was to understand the fundamentals of rasterization by removing as much abstraction as possible and building the pipeline from the ground up.

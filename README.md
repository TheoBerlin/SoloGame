![Välfärden](https://github.com/TheoBerlin/SoloGame/workflows/V%C3%A4lf%C3%A4rden/badge.svg)

## What is this project?
In this repository is an engine being made from scratch using C++. Graphics rendering is performed by either DirectX or Vulkan. This is a project I started working on for two purposes; to learn and to have something to display to employers.

## Highlights in the code
### ECS
The engine uses an Entity Component System (ECS) architecture for storing game object data and performing logic. ECS resides in the
[Engine/ECS folder](https://github.com/TheoBerlin/SoloGame/tree/master/src/Engine/ECS).
It's very much inspired by
various discussions and videos, such as [this talk at GDC](https://www.youtube.com/watch?v=0_Byw9UMn9g).

Short summary of the implementation:
* Entity: An ID represented by an unsigned integer
* Component: A struct of data, is linked to an Entity at creation
* System: Subscribes to entities that have certain component types and performs logic in an `update(float dt)` function
* Component Handler: Stores components in 'IDVectors', vectors that are indexable using entity IDs. Vectors store elements contiguously,
increasing cache friendliness.

When systems subscribe to component types, they include information on whether they will only read the component data or if they will also
write to them. This allows for an easy parallelization of calls to system updates (each system's update function is called each frame). All
that has to be done to prevent data races is to only update systems that do not read or write to a component type that is already being
written to by another system.

The updating of systems is done in
[Engine/ECS/JobScheduler.cpp](https://github.com/TheoBerlin/SoloGame/blob/master/src/Engine/ECS/JobScheduler.cpp).

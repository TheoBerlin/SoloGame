## What is this project?
In this repository is an engine and a game, both being made from scratch using C++ and DirectX, and are far away from being finalized. This is a project I started for two purposes;
to learn and to have something to display to employers.

## Highlights in the code
Probably the best part of the code is the implementation of the Entity Component System (ECS) architecture. This resides in the
[Engine/ECS folder](https://github.com/TheoBerlin/SoloGame/tree/master/GameProject/Engine/ECS).
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
[Engine/ECS/SystemUpdater.cpp](https://github.com/TheoBerlin/SoloGame/blob/master/GameProject/Engine/ECS/SystemUpdater.cpp).

## What will the final game look like?
I'm trying to make something very small, to make sure the game is finished in time for my graduation in 2021.

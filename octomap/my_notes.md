# My notes

## Changes to do:

I want to be able to store semantic labels, average colors, and label colors, in each voxel.

It looks like the base voxel definitition is in: 
- octomap_types
- OccupancyOcTreeBase

As far as I can tell the current version of octomap uses

The octomap_msgs use a serialization to send the octomap.

I've also forked octomap - mapping and server which are the ros things, I think those are actully what I want to be changing.

## Where to start
There is an example which uses color.

- ColorOcTree.h
- ColorOcTree.cpp
- convert_octree.cpp
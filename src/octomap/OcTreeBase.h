#ifndef OCTOMAP_OCTREE_BASE_H
#define OCTOMAP_OCTREE_BASE_H

// $Id$

/**
* Octomap:
* A  probabilistic, flexible, and compact 3D mapping library for robotic systems.
* @author K. M. Wurm, A. Hornung, University of Freiburg, Copyright (C) 2009.
* @see http://octomap.sourceforge.net/
* License: GNU GPL v2, http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <list>

#include "octomap_types.h"
#include "ScanGraph.h"


namespace octomap {


  /**
   * OcTree base class, to be used with with any kind of OcTreeDataNode.
   *
   * This tree implementation has a maximum depth of 16. 
   * At a resolution of 1 cm, values have to be < +/- 327.68 meters (2^15)
   *
   * This limitation enables the use of an efficient key generation 
   * method which uses the binary representation of the data.
   *
   * \note The tree does not save individual points.
   */
  template <class NODE>
  class OcTreeBase {

  public:

    OcTreeBase(double _resolution);
    virtual ~OcTreeBase();

    /// \return The number of nodes in the tree
    unsigned int size() const { return tree_size; }

    void setResolution(double r);
    double getResolution() const { return resolution; }

    /**
     * \return Pointer to the root node of the tree. This pointer should
     * not be modified or deleted externally, the OcTree manages its
     * memory itself.
     */
    NODE* getRoot() const { return itsRoot; }

    /**
     * Search for a 3d point in the tree
     *
     * @return pointer to the corresponding NODE when found, else NULL. This pointer should
     * not be modified or deleted externally, the OcTree manages its memory itself.
     */
    NODE* search (double x, double y, double z) const;

    /// Search for a 3d point in the tree, using a point3d (see above)
    NODE* search (const point3d& value) const;

    /// Lossless compression of OcTree: merge children to parent when there are
    /// eight children with identical values
    void prune();

    /// Expands all pruned nodes (reverse of prune())
    /// \note This is an expensive operation, especially when the tree is nearly empty!
    void expand();


    /// \return Memory usage of a full grid of the same size as the OcTree in bytes (for comparison)
    unsigned int memoryFullGrid();

    /// Size of OcTree in meters for x, y and z dimension
    void getMetricSize(double& x, double& y, double& z);
    /// minimum value in x, y, z
    void getMetricMin(double& x, double& y, double& z);
    void getMetricMin(double& x, double& y, double& z) const;
    /// maximum value in x, y, z
    void getMetricMax(double& x, double& y, double& z);
    void getMetricMax(double& x, double& y, double& z) const;

    /// Traverses the tree to calculate the total number of nodes
    unsigned int calcNumNodes() const;


   /**
    * Traces a ray from origin to end (excluding), returning the
    * coordinates of all nodes traversed by the beam.
    * (Essentially using the DDA algorithm in 3D).
    *
    * @param origin start coordinate of ray
    * @param end end coordinate of ray
    * @param ray center coordinates of all nodes traversed by the ray, excluding "end"
    * @return Success of operation. Returning false usually means that one of the coordinates is out of the OcTree's range
    */
    bool computeRay(const point3d& origin, const point3d& end, std::vector<point3d>& ray) const;


    /**
     * Traverse the tree and collect all leaf nodes
     *
     * @param nodes Leaf nodes as OcTreeVolume
     * @param max_depth Depth limit of query. 0 (default): no depth limit
     */
    void getLeafNodes(std::list<OcTreeVolume>& nodes, unsigned int max_depth = 0) const;

    /**
     * Traverse the tree and collect all nodes, at all levels. Used e.g. in visualization.
     *
     * @param voxels list of all nodes to be returned
     * @param max_depth Depth limit of query. 0 (default): no depth limit
     */
    void getVoxels(std::list<OcTreeVolume>& voxels, unsigned int max_depth = 0) const;

    //---- FILE IO --//

    /// Read complete state of tree from stream
    /// EXPERIMENTAL!
    std::istream& read(std::istream &s);

    /// Write complete state of tree to stream, prune tree first (lossless compression)
    /// EXPERIMENTAL!
    std::ostream& write(std::ostream &s);

    /// Write complete state of tree to stream, no pruning (const version)
    std::ostream& writeConst(std::ostream &s) const;


  protected:

    /**
     * Generates a 16-bit key from/for given value when it is within
     * the octree bounds, returns false otherwise
     *
     * @param val coordinate of one dimension in the octree
     * @param key 16bit key of the given coordinate, returned
     * @return true if val is within the octree bounds
     */
    bool genKey(double val, unsigned short int& key) const;

    /**
     * Generates key for all three dimensions of a given point
     * using genKey().
     *
     * @param point 3d coordinate of a point
     * @param keys values that will be computed, an array of fixed size 3.
     * @return true when point is within the octree, false otherwise
     */
    bool genKeys(const point3d& point, unsigned short int (&keys)[3]) const;

    /// reverse of genKey(), generates center coordinate of cell corresponding to a key
    bool genVal(unsigned short int& key, double& val) const;

    /// generate child index (between 0 and 7) from key at given tree depth
    unsigned int genPos(unsigned short int key[], int i) const;

    /// recursive call of prune()
    void pruneRecurs(NODE* node, unsigned int depth, unsigned int max_depth, unsigned int& num_pruned);

    /// recursive call of expand()
    void expandRecurs(NODE* node, unsigned int depth, unsigned int max_depth, unsigned int& num_expanded);

    /// Recursive call for getLeafNodes()
    void getLeafNodesRecurs(std::list<OcTreeVolume>& nodes, unsigned int max_depth,
          NODE* node, unsigned int depth, const point3d& parent_center) const;

    /// Recursive call for getVoxels()
    void getVoxelsRecurs(std::list<OcTreeVolume>& nodes, unsigned int max_depth,
        NODE* node, unsigned int depth, const point3d& parent_center) const;

    /// recalculates min and max in x, y, z. Does nothing when tree size didn't change.
    void calcMinMax();


    void calcNumNodesRecurs(NODE* node, unsigned int& num_nodes) const;



    NODE* itsRoot;

    // constants of the tree
    unsigned int tree_depth;
    unsigned int tree_max_val;
    double resolution;  ///< in meters
    double resolution_factor; ///< = 1. / resolution
    point3d tree_center;

    unsigned int tree_size; ///< number of nodes in tree
    double maxValue[3]; ///< max in x, y, z
    double minValue[3]; ///< min in x, y, z
    bool sizeChanged;
  };


}

#include "OcTreeBase.hxx"

#endif

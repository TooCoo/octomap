/*
 * OctoMap - An Efficient Probabilistic 3D Mapping Framework Based on Octrees
 * http://octomap.github.com/
 *
 * Copyright (c) 2009-2013, K.M. Wurm and A. Hornung, University of Freiburg
 * All rights reserved.
 * License: New BSD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Freiburg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OCTOMAP_COLOR_PLUS_OCTREE_H
#define OCTOMAP_COLOR_PLUS_OCTREE_H


#include <iostream>
#include <octomap/OcTreeNode.h>
#include <octomap/OccupancyOcTreeBase.h>

namespace octomap {

    // forward declaraton for "friend"
    class ColorPlusOcTree;

    // node definition
    class ColorPlusOcTreeNode : public OcTreeNode {
    public:
        friend class ColorPlusOcTree; // needs access to node children (inherited)

        class Color {
        public:
            Color() : r(255), g(255), b(255) {}
            Color(uint8_t _r, uint8_t _g, uint8_t _b)
                    : r(_r), g(_g), b(_b) {}
            inline bool operator== (const Color &other) const {
                return (r==other.r && g==other.g && b==other.b);
            }
            inline bool operator!= (const Color &other) const {
                return (r!=other.r || g!=other.g || b!=other.b);
            }
            uint8_t r, g, b;
        };

        // TODO : This still is pretty much a color, I want an ID (int8_t)
        class Label {
        public:
            //Label() : label[10] {}
            Label(float_t _prior){
                for (int i = 0; i < 10; i++)
                {
                    label[i] = _prior;
                    // TODO: normalise pdf (label)
                }
            }
            inline bool operator== (const Label &other) const {
                // TODO write an equivalency
                // I image that this can be done by treating label as a vector and finding the
                // euclidean distance between cell state.
                // ||p(a)-p(b)|| < threshold
                return (r==other.r && g==other.g && b==other.b);
            }
            // TODO: also write this
            inline bool operator!= (const Label &other) const {
                return (r!=other.r || g!=other.g || b!=other.b);
            }
            float label[10];
        };

    public:
        ColorPlusOcTreeNode() : OcTreeNode() {}

        // TODO : this needs label to be added to it
        ColorPlusOcTreeNode(const ColorPlusOcTreeNode& rhs) : OcTreeNode(rhs), color(rhs.color) {}

        // TODO : I need to define ColorPlusOcTreeNode
        bool operator==(const ColorPlusOcTreeNode& rhs) const{
            return (rhs.value == value && rhs.color == color && rhs.label == label);
        }

        void copyData(const ColorPlusOcTreeNode& from){
            OcTreeNode::copyData(from);
            this->color =  from.getColor();
            this->label =  from.getLabel();
        }

        inline Color getColor() const { return color; }

        inline void  setColor(Color c) {this->color = c; }

        inline void  setColor(uint8_t r, uint8_t g, uint8_t b) {
            this->color = Color(r,g,b);
        }

        Color& getColor() { return color; }

        // has any color been integrated? (pure white is very unlikely...)
        inline bool isColorSet() const {
            return ((color.r != 255) || (color.g != 255) || (color.b != 255));
        }

        // TODO : still fairly colory
        inline Label getLabel() const { return label; }

        inline void  setLabel(Label l) {this->label = l; }

        inline void  setLabel(uint8_t r, uint8_t g, uint8_t b) {
            this->label = Label(r,g,b);
        }

        Label& getLabel() { return label; }

        // has any color been integrated? (pure white is very unlikely...)
        inline bool isLabelSet() const {
            return ((label.r != 255) || (label.g != 255) || (label.b != 255));
        }

        void updateColorChildren();

        void updateLabelChildren();


        ColorPlusOcTreeNode::Color getAverageChildColor() const;

        ColorPlusOcTreeNode::Label getAverageChildLabel() const;

        // file I/O
        std::istream& readData(std::istream &s);
        std::ostream& writeData(std::ostream &s) const;

    protected:
        Color color;
        Label label;
    };

    // tree definition
    class ColorPlusOcTree : public OccupancyOcTreeBase <ColorPlusOcTreeNode> {

    public:
        /// Default constructor, sets resolution of leafs
        ColorPlusOcTree(double resolution);

        /// virtual constructor: creates a new object of same type
        /// (Covariant return type requires an up-to-date compiler)
        ColorPlusOcTree* create() const {return new ColorPlusOcTree(resolution); }

        std::string getTreeType() const {return "ColorPlusOcTree";}

        /**
        * Prunes a node when it is collapsible. This overloaded
        * version only considers the node occupancy for pruning,
        * different colors of child nodes are ignored.
        * @return true if pruning was successful
        */

        virtual bool pruneNode(ColorPlusOcTreeNode* node);

        virtual bool isNodeCollapsible(const ColorPlusOcTreeNode* node) const;

        // set node color at given key or coordinate. Replaces previous color.
        ColorPlusOcTreeNode* setNodeColor(const OcTreeKey& key, uint8_t r,
                                          uint8_t g, uint8_t b);

        ColorPlusOcTreeNode* setNodeColor(float x, float y,
                                          float z, uint8_t r,
                                          uint8_t g, uint8_t b) {
            OcTreeKey key;
            if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
            return setNodeColor(key,r,g,b);
        }

        // integrate color measurement at given key or coordinate. Average with previous color
        ColorPlusOcTreeNode* averageNodeColor(const OcTreeKey& key, uint8_t r,
                                              uint8_t g, uint8_t b);

        ColorPlusOcTreeNode* averageNodeColor(float x, float y,
                                              float z, uint8_t r,
                                              uint8_t g, uint8_t b) {
            OcTreeKey key;
            if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
            return averageNodeColor(key,r,g,b);
        }

        // integrate color measurement at given key or coordinate. Average with previous color
        ColorPlusOcTreeNode* integrateNodeColor(const OcTreeKey& key, uint8_t r,
                                                uint8_t g, uint8_t b);

        ColorPlusOcTreeNode* integrateNodeColor(float x, float y,
                                                float z, uint8_t r,
                                                uint8_t g, uint8_t b) {
            OcTreeKey key;
            if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
            return integrateNodeColor(key,r,g,b);
        }

        // update inner nodes, sets color to average child color
        void updateInnerOccupancy();

        // uses gnuplot to plot a RGB histogram in EPS format
        void writeColorHistogram(std::string filename);

        // Semantic functions
        // set node label at given key or coordinate. Replaces previous label.
        // TODO : this is still the same as color
        ColorPlusOcTreeNode* setNodeLabel(const OcTreeKey& key, uint8_t r,
                                          uint8_t g, uint8_t b);

        ColorPlusOcTreeNode* setNodeLabel(float x, float y,
                                          float z, uint8_t r,
                                          uint8_t g, uint8_t b) {
            OcTreeKey key;
            if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
            return setNodeLabel(key,r,g,b);
        }

        // integrate label measurement at given key or coordinate.
        ColorPlusOcTreeNode* averageNodeLabel(const OcTreeKey& key, uint8_t r,
                                              uint8_t g, uint8_t b);

        ColorPlusOcTreeNode* averageNodeLabel(float x, float y,
                                              float z, uint8_t r,
                                              uint8_t g, uint8_t b) {
            OcTreeKey key;
            if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
            return averageNodeLabel(key,r,g,b);
        }

        // integrate label measurement at given key or coordinate. Average with previous color
        ColorPlusOcTreeNode* integrateNodeLabel(const OcTreeKey& key, uint8_t r,
                                                uint8_t g, uint8_t b);

        ColorPlusOcTreeNode* integrateNodeLabel(float x, float y,
                                                float z, uint8_t r,
                                                uint8_t g, uint8_t b) {
            OcTreeKey key;
            if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
            return integrateNodeLabel(key,r,g,b);
        }

        // uses gnuplot to plot a RGB histogram in EPS format
        void writeLabelHistogram(std::string filename);

    protected:
        void updateInnerOccupancyRecurs(ColorPlusOcTreeNode* node, unsigned int depth);

        /**
         * Static member object which ensures that this OcTree's prototype
         * ends up in the classIDMapping only once. You need this as a
         * static member in any derived octree class in order to read .ot
         * files through the AbstractOcTree factory. You should also call
         * ensureLinking() once from the constructor.
         */
        class StaticMemberInitializer{
        public:
            StaticMemberInitializer() {
                ColorPlusOcTree* tree = new ColorPlusOcTree(0.1);
                tree->clearKeyRays();
                AbstractOcTree::registerTreeType(tree);
            }

            /**
            * Dummy function to ensure that MSVC does not drop the
            * StaticMemberInitializer, causing this tree failing to register.
            * Needs to be called from the constructor of this octree.
            */
            void ensureLinking() {};
        };
        /// static member to ensure static initialization (only once)
        static StaticMemberInitializer colorPlusOcTreeMemberInit;

    };

    //! user friendly output in format (r g b)
    // TODO : What happens here? I need to also write a label output for this
    std::ostream& operator<<(std::ostream& out, ColorPlusOcTreeNode::Color const& c);

    std::ostream& operator<<(std::ostream& out, ColorPlusOcTreeNode::Label const& l);

} // end namespace

#endif

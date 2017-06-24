#include "project.h"

OctreeNode *octree;

inline bool bounding_boxes_intersect(const vec &min1, const vec &max1, const vec &min2, const vec &max2)
{
    return (min1.x <= max2.x) && (max1.x >= min2.x) &&
           (min1.y <= max2.y) && (max1.y >= min2.y) &&
           (min1.z <= max2.z) && (max1.z >= min2.z);
}

void OctreeNode::insert(Face *f, bool move)
{
    if(!move)
    {
        ++triangle_count;
        this->faces.push_back(f);
        f->nodes.push_back(this);
    }

    if(leaf) // the current node doesn't have any child
    {
        // if the limit is reached, divide the current node.
        if(triangle_count > 1) this->divide();
    }

    else // current node already has children
    {
        // find the appropriate child for the given entity and insert it
        for(int i = 0; i < 8; ++i)
        {
            if (bounding_boxes_intersect(f->min, f->max, children[i]->min, children[i]->max))
                children[i]->insert(f, move);
        }
    }
}
    
void OctreeNode::divide()
{
    if(!this->leaf) return;
    leaf = false;

    for(int i = 0; i < 8; ++i)
    {
        OctreeNode *node = new OctreeNode();
        node->size = this->size/2.0;

        node->midpoint = vec(
            midpoint.x + ((i & 4) ? node->size/2.0 : -node->size/2.0),
            midpoint.y + ((i & 2) ? node->size/2.0 : -node->size/2.0),
            midpoint.z + ((i & 1) ? node->size/2.0 : -node->size/2.0)
        );

        node->min = vec(
            node->midpoint.x - node->size/2.0,
            node->midpoint.y - node->size/2.0,
            node->midpoint.z - node->size/2.0
        );

        node->max = vec(
            node->midpoint.x + node->size/2.0,
            node->midpoint.y + node->size/2.0,
            node->midpoint.z + node->size/2.0
        );

        node->depth = this->depth + 1;
        node->parent = this;

        children[i] = node;
    }

    for(unsigned int i = 0; i < this->faces.size(); ++i)
    {
        this->insert(this->faces[i], true);
    }
}

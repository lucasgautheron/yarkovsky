struct Face;

struct OctreeNode
{
    vec min, max, midpoint;
    
    OctreeNode *children[8];
    OctreeNode *parent;
    int depth;
    bool leaf, solid;
    double size;

    int triangle_count;
    std::vector<Face *> faces;

    OctreeNode() : size(0), parent(NULL), depth(0), leaf(true), triangle_count(0)
    {
        midpoint = vec(0, 0, 0);
        for(int i = 0; i < 8; ++i) children[i] = NULL;
    }
    ~OctreeNode()
    {
        if(!leaf) for(int i = 0; i < 8; ++i) if(children[i]) delete children[i];
    }
    void insert(Face *f, bool move = false);
    void divide();
    void compute_center_of_mass();
};

void faces_intersecting_ray(const vec &x0, const vec &n, vector<Face *> &faces, OctreeNode *octree, Face *ignore = NULL);

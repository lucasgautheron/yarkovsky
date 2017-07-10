#define TEMPDIV 256     // divisions spatiales
                        // (pour resoudre dT/dt = div . (grad T))

struct Face
{
    vec *vertices[3], n, min, max, pos;
    vec x_axis, y_axis; // (local coords, _z_axis = n)
    double area; // aire en m²
    double depth;
    double temp[TEMPDIV], tempn[TEMPDIV]; // temperature;
    double viewfactor;
    double h; // hauteur en m
    bool enlightened;
};

// The 3D-Model structure
struct Model
{
    const char *filename;
    double scale, size;

    std::vector<vec *> vertices; // model vertices (x, y, z)
    std::vector<vec *> normals; // precalculated normals
    std::vector<Face *> faces; // each model face
    std::vector< std::vector<Face *> *> vertices_faces;

    vec bbmin, bbmax, delta, o; // model bounds
    vec bounds[8];

    int tris;

    Model(const char *filename, double scale) : filename(filename), scale(scale)
    {
        bbmin = vec(1e10, 1e10, 1e10);
        bbmax = -bbmin;
        tris = 0;

        for(int i = 0; i < 8; ++i) for(int j = 0; j < 3; ++j)
        {
            bounds[i].v[j] = i&(1<<j) ? -1e10 : +1e10;
        }
    }

    bool load()
    {
        FILE *fp = fopen(filename, "r");
        if(fp == NULL)
        {
            printf("Erreur lors de l'ouverture du fichier \"%s\"", filename);
            return false;
        }
        while(true)
        {
		    char line[128];
		    // read the first word of the line
		    int res = fscanf(fp, "%s", line);
		    if (res == EOF) break;
		    if (strcmp(line, "v") == 0)
            {
			    vec *v = new vec();
			    fscanf(fp, "%lf %lf %lf\n", &v->x, &v->y, &v->z);
                if(scale > 0) (*v) *= scale;
			    vertices.push_back(v);

                std::vector<Face *> *vf = new std::vector<Face *>;
                vertices_faces.push_back(vf);

                if(v->x < bbmin.x) bbmin.x = v->x;
                if(v->y < bbmin.y) bbmin.y = v->y;
                if(v->z < bbmin.z) bbmin.z = v->z;

                if(v->x > bbmax.x) bbmax.x = v->x;
                if(v->y > bbmax.y) bbmax.y = v->y;
                if(v->z > bbmax.z) bbmax.z = v->z;

                for(int i = 0; i < 8; ++i) for(int j = 0; j < 3; ++j)
                {
                    int bit = i&(1<<j);
                    if( (bit != 0) && bounds[i].v[j] < v->v[j]) bounds[i].v[j] = v->v[j];
                    if( (bit == 0) && bounds[i].v[j] > v->v[j]) bounds[i].v[j] = v->v[j];
                }
		    }
            else if (strcmp(line, "vn" ) == 0)
            {
			    vec *n = new vec();
			    fscanf(fp, "%lf %lf %lf\n", &n->x, &n->y, &n->z);
                normals.push_back(n);
		    }
            else if (strcmp(line, "f" ) == 0)
            {
                // chargement d'une face (triangle)
                // nécessite : vecteurs normaux, pas de rectangles, pas de textures
			    unsigned int vi[3], ni[3];
                int pointer = ftell(fp);
                bool has_normal = true;
			    int matches = fscanf(fp, "%d//%d %d//%d %d//%d\n", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2]);
                
                Face *f = new Face();

                if(matches != 6)
                {
                    fseek(fp, pointer, SEEK_SET);
                    matches = fscanf(fp, "%d %d %d\n", &vi[0], &vi[1], &vi[2]);
                    has_normal = false;

                    if(matches != 3)
                    {
                        printf("Erreur lors de la lecture du modele (face incorrecte - %d matches)\n", matches);
                        break;
                    }                    
                }
                ++tris;
                f->vertices[0] = vertices[vi[0]-1];
                f->vertices[1] = vertices[vi[1]-1];
                f->vertices[2] = vertices[vi[2]-1];
                f->pos = (*f->vertices[0] + *f->vertices[1] + *f->vertices[2])/3.0;
                f->area = ((*f->vertices[1]-*f->vertices[0])^(*f->vertices[2]-*f->vertices[0])).norm()/2.0;
                f->depth = 5.0;

                if(has_normal)
                {
                    f->n = *normals[ni[0]-1];
                }
                else
                {
                    f->n = (f->vertices[1]-f->vertices[0])^(f->vertices[2]-f->vertices[0]);
                    f->n.normalize();
                }

                f->h = fabs(f->n.dot(f->pos));
                if (f->h < f->depth)
                {
                    printf("warning: tetrahedron is not deep enough (%.2f, %.2f)\n", f->h, f->depth);
                }

                // FIXME: s'assurer que les vecteurs normaux sont bien normalisés ?
                // L'erreur sur la norme est minime mais existe (blender doit faire ça grossièrement)
                // Par contre il y a risque d'introduire une erreur sur la direction ?
                if(f->n.dot(*f->vertices[0]) < 0) f->n = -f->n;

                vertices_faces[vi[0]-1]->push_back(f);
                vertices_faces[vi[1]-1]->push_back(f);
                vertices_faces[vi[2]-1]->push_back(f);

                // get face bounds
                f->min = f->max = *f->vertices[0];
                for(int j = 0; j < 3; ++j)
                {
                    if(f->vertices[1]->v[j] < f->min[j]) f->min[j] = f->vertices[1]->v[j];
                    if(f->vertices[1]->v[j] > f->max[j]) f->max[j] = f->vertices[1]->v[j];
                    if(f->vertices[2]->v[j] < f->min[j]) f->min[j] = f->vertices[2]->v[j];
                    if(f->vertices[2]->v[j] > f->max[j]) f->max[j] = f->vertices[2]->v[j];
                }
			    faces.push_back(f);

		    }
            else
            {
                // commentaire ou erreur de syntaxe
			    char next[1000];
			    fgets(next, 1000, fp);
		    }
	    }

        o = (bbmax+bbmin)/2.0;
        delta = bbmax-bbmin;
        size = max(delta.x, max(delta.y, delta.z));

        DELETEV(normals);
        return true;
    }

    ~Model()
    {
        DELETEV(vertices);
        DELETEV(normals);
        DELETEV(faces);
    }
};

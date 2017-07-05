#define C_CELERITY (299792458.0)
#define C_GRAVITY (6.6784e-11)
#define C_PLANCK (6.626e-34)
#define C_PLANCKOVER2PI (0)
#define C_STEFANBOLTZMANN (5.670400e-8)
#define C_SUNMASS (1.989e30)
#define C_SOLARFLUX (3.0688116e25)
#define C_SUNCONST (1.32712440018e20) // C_GRAVITY*C_SUNMASS, connu avec une grande precision.

#define TIMESTEP 0.10

struct Force
{
    vec f, p;
    bool torque, gravity;

    Force() { f = p = vec(0, 0, 0); torque = true; gravity = false; }
    Force(vec f, vec p, bool torque, bool gravity) : f(f), p(p), torque(torque), gravity(gravity) {}
};

struct Asteroid
{
    Model *mdl; // obj modele
    bool loaded;
    double scale; // echelle

    // caracteristiques intrinseques du solide
    // calculees une unique fois
    double albedo, emissivity; // optique
    double volume, surface, density, mass;
    double conductivity, diffusivity, capacity; // proprietes thermiques
    double initialtemperature;

    double period;

    // variables internes
    double time, timestep; // (s)
    double nextvisibilitycompute;

    vec centerofmass; // centre de masse (m)
    matrix inertia; // matrice d'inertie (kg.m²)
    double inertia_det;
    double size;

    // caracteristiques dynamiques
    vec pos; // centre de masse ref heliocentrique (m)
    vec angle; // orientation du solide (tangage/roulis/lacet) (rad)
    matrix rotmatrix; // matrice de rotation

    vec vel; // d(pos)/dt
    vec rotvel; // d(angle)/dt

    // rotation
    // x = roll (roulis), y = pitch (tangage) , z  = yaw (lacet)
    vec accel; // d(vel)/dt (m/s²)
    vec rotaccel; // d(rotvel)/dt (rad/s²)

    std::vector<Force> forces; // forces appliques au solide
    vec momentum; // dans le referentiel du solide, / au centre de masse

    // orbite autour du soleil
    double semiaxis, excentricity;
    vec plan;
    // energie :
    double totalenergy, energyvariation; // (J)
    
    int loops;
    stream *log;
    stream *templog;

    void calcsolid(); // calcule le volume, le centre de masse, etc.
    void calcimatrix(); // calcule la matrice d'inertie

    void calcviewfactors();
    inline void calctemperature(Face *f);

    // calcul des forces
    void calcemitted();
    void calcgravity(); // calcul de la gravite

    void move(double dt); // met a jour la vitesse angulaire et lineaire puis la position.

    void calcloop();
    void calcperiod();

    Asteroid() : loaded(false),
        albedo(0.5), emissivity(0.5), density(2500.0),
        conductivity(2.9), diffusivity(1.2e-6), initialtemperature(249.0),
        period(18720.0),
        timestep(TIMESTEP),
        semiaxis(UA),
        energyvariation(0.0),
        loops(0)
    {
        rotvel = plan = vec(0, 0, 1);
        pos = vec(1, 0, 0);
        loop(i, 3)
        {
            vel[i] = accel[i] = rotaccel[i] = 0.0;
            momentum[i] = 0.0;
        }

        nextvisibilitycompute = 0;
    }

    bool loadmdl(const char *mdlname, const char *logfile)
    {
        //copystring(name, mdlname);
        string logfullfilename = string("../output/") + string(logfile);
        log = openrawfile(logfullfilename.c_str(), "w+");
        templog = openrawfile("../output/temp.csv", "w+");

        readcfg(mdlname);

        char filename[256] = "";
        sprintf(filename, "../models/%s/model.obj", mdlname);

        mdl = new Model(filename, scale);
        mdl->scale = scale;

        loaded = mdl->load();
        if(!loaded) return false;

        for(unsigned int i = 0; i < mdl->faces.size(); ++i) for(int k = 0; k < TEMPDIV; ++k)
            mdl->faces[i]->temp[k] = mdl->faces[i]->tempn[k] = initialtemperature;

        calcsolid();
        calcimatrix();
        size = (mdl->bbmax-mdl->bbmin).norm();

        pos.normalize(semiaxis);
        setvel();
        setrotvel();

        printf("chargement modele \"%s\" : %d faces, %d sommets.\n",
            filename,
            (int)mdl->faces.size(),
            (int)mdl->vertices.size());
        printf(" volume: %e m3; surface: %e m2; masse: %e kg;\n", volume, surface, mass);
        printf(" dx = %.4f km, dy = %.4f km, dz = %.4f km, taille = %.4f km\n",
            (mdl->bbmax-mdl->bbmin).x/1000.0,
            (mdl->bbmax-mdl->bbmin).y/1000.0,
            (mdl->bbmax-mdl->bbmin).z/1000.0,
            size/1000.0);
        printf("Distance Soleil: %.2f UA, Vel: %.3f km/s, RotVel: %.3f rad/j\n", pos.norm()/UA, vel.norm() / 1000.0, rotvel.norm() * 86400.0);

        inertia.print();
        return true;
    }

    void readcfg(const char *mdlname)
    {
        char filename[256] = "";
        sprintf(filename, "../models/%s/model.cfg", mdlname);
        stream *f = openrawfile(filename, "r");
        if(!f) return;

        char line[256] = "";
        while(f->getline(line, sizeof(line)))
        {
            if(sscanf(line, "emissivity = %lf", &emissivity)) continue;
            if(sscanf(line, "temp = %lf", &initialtemperature)) continue;
            if(sscanf(line, "excentricity = %lf", &excentricity)) continue;
            if(sscanf(line, "semiaxis = %lf", &semiaxis)) continue;
            if(sscanf(line, "albdedo = %lf", &albedo)) continue;
            if(sscanf(line, "scale = %lf", &scale)) continue;
            if(sscanf(line, "conductivity = %lf", &conductivity)) continue;
            if(sscanf(line, "diffusivity = %lf", &diffusivity)) continue;
            if(sscanf(line, "density = %lf", &density)) continue;
            if(sscanf(line, "pos = (%lf, %lf, %lf)", &pos.x, &pos.y, &pos.z)) continue;
            if(sscanf(line, "plan = (%lf, %lf, %lf)", &plan.x, &plan.y, &plan.z)) continue;
            if(sscanf(line, "rot = (%lf, %lf, %lf)", &rotvel.x, &rotvel.y, &rotvel.z)) continue;
            if(sscanf(line, "period = %lf", &period)) continue;
            if(sscanf(line, "timestep = %lf", &timestep)) continue;
        }
    }

    vec &setvel()
    {
        vel = (pos^plan).normalize(sqrt(C_SUNCONST/pos.norm()));
        totalenergy = mass*(0.5*vel.squaredlen()-C_SUNCONST/pos.norm());
        return vel;
    }

    vec &setrotvel()
    {
        rotvel.normalize(2*D_PI/period);
        momentum = rotvel;
        momentum.mul(inertia);
        rotmatrix = MATRIX_IDENTITY(3);
        return rotvel;
    }

    ~Asteroid()
    {
        if(log) log->close();
        if(templog) templog->close();
        DELETEP(log);
        DELETEP(templog);
        DELETEP(mdl);
    }
};

extern Asteroid asteroid1;

#include "project.h"

Asteroid asteroid1;

// Calculs lies a la forme du solide
// ----------------------------------------
// Ces fonctions utilisent des algorithmes tres simples,
// Exploitant le format type "surface mesh" des objets sources
// Ils pourraient etre preicses dans le cas d'objets non convexes
// en utilisant la direction des vecteurs normaux aux faces, qui indiquent
// si la masse / le volume doit etre compte negativement ou positivement
// NB: La densite est supposee uniforme

// Asteroid::calcsolid()
// Calcule quelques caracteristiques intrinseques au solide
// (En gros le solide est division en tetrahedres)
// Ne prend pas en compte l'intersection eventuelle de tetrahedre *pour l'instant*
// Devrait le faire en fonction de la direction du vecteur normal a chaque surface
void Asteroid::calcsolid()
{
    vec tmp_cm(0, 0, 0);
    double totalvolume = 0.0;
    surface = 0.0;
    for(unsigned int i = 0; i < mdl->faces.size(); ++i)
    {
        surface += mdl->faces[i]->area;
        double vol = det3x3(*mdl->faces[i]->vertices[0], *mdl->faces[i]->vertices[1], *mdl->faces[i]->vertices[2]);
        tmp_cm += (*mdl->faces[i]->vertices[0]+*mdl->faces[i]->vertices[1]+*mdl->faces[i]->vertices[2])*vol;
        totalvolume += vol;
    }
    centerofmass = (tmp_cm/(4.0*totalvolume));
    volume = fabs(totalvolume/6.0); 
    mass = volume*density;
}

// Asteroid::calcsolid()
// Calcule la matrice d'inertie du solide, a partir du centre de masse.
// Divise le solide en tetrahedre. La matrice de covariance de chaque tetrahedre
// est calculee et sommee, puis la matrice d'inertie est calculee
// (Tres precis)

void Asteroid::calcimatrix()
{
    inertia.resize(3, 3);

    matrix tmp_inertia;
    for(int i = 0; i < 3; ++i) for(int j = 0; j < 3; ++j) inertia[i][j] = 0.0;
    
    for(unsigned int i = 0; i < mdl->faces.size(); ++i)
    {
        tmp_inertia += tetrahedronmatrix(*mdl->faces[i]->vertices[0], *mdl->faces[i]->vertices[1], *mdl->faces[i]->vertices[2]);
    }

    // FIXME: translation a verifier
    matrix cmass(3, 1); cmass[0][0] = centerofmass[0]; cmass[1][0] = centerofmass[1]; cmass[2][0] = centerofmass[2];
    matrix cmass_transp = cmass; cmass_transp.transpose();
    matrix translate = (cmass*cmass_transp)*3.0;
    tmp_inertia += translate;
    double t = tmp_inertia.trace();
    inertia = ((MATRIX_IDENTITY(3) * t) - tmp_inertia) * density;

    inertia_det = det3x3(inertia);
}

// Asteroid::calcsolid()
// (Ancienne version)
// Assimile les tetrahedres a des points. Trop approximatif.
// De toute facon, ce calcul peut etre lent, car executé une unique fois.
/*
void Asteroid::calcimatrix()
{
    matrix tmp_inertia;
    for(int i = 0; i < 3; ++i) for(int j = 0; j < 3; ++j) inertia[i][j] = 0.0;
    
    for(int i = 0; i < mdl->faces.size(); ++i)
    {
        double vol = fabs(det3x3(*mdl->faces[i]->vertices[0], *mdl->faces[i]->vertices[1], *mdl->faces[i]->vertices[2]));
        vec g = (*mdl->faces[i]->vertices[0]+*mdl->faces[i]->vertices[1]+*mdl->faces[i]->vertices[2])/4.0;
        for(int u = 0; u < 3; ++u) for(int v = 0; v < 3; ++v)
        {
            if(u != v) tmp_inertia[u][v] -= vol*(g-centerofmass)[u]*(g-centerofmass)[v];
            else tmp_inertia[u][v] += vol*(
                (g-centerofmass)[(u-1)<0?2:(u-1)]*(g-centerofmass)[(u-1)<0?2:(u-1)]+
                (g-centerofmass)[(v+1)>2?0:(v+1)]*(g-centerofmass)[(v+1)>2?0:(v+1)]);
        }
    }

    inertia = tmp_inertia * density/6.0;
    inertia_det = det3x3(inertia);
}*/

void Asteroid::calcviewfactors()
{
    double distance_to_sun = pos.norm();
    vec raydir = pos;
    raydir.normalize(1);

#ifdef GUI
    for(int i = 0; i < mdl->faces.size(); ++i) mdl->faces[i]->enlightened = false;

    uchar *buf = new uchar[4*w*h];
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    for(int i = 0; i < w; ++i) for(int j = 0; j < h; ++j)
    {
        uchar *rgb = &buf[4*(j*w+i)];
        int f = rgb[0] | (rgb[1] << 8) | (rgb[2] << 16);
        if (!f) continue;
        f -= 1;

        if (f >= 0 && f < mdl->faces.size())
        {
            mdl->faces[f]->enlightened = true;
        }
    }

    delete[] buf;

#endif

    int mismatch = 0,
        enlightened = 0;

    #pragma omp parallel for
    for(int i = 0; i < mdl->faces.size(); ++i)
    {
        Face *f = mdl->faces[i];
        //f->enlightened = true;
        vec n = f->n;

        n.mul(rotmatrix);

        if(f->enlightened) enlightened++;
        if(f->enlightened && n.dot(pos) > 0) mismatch++;

        f->viewfactor = f->enlightened ? n.dot(pos)/distance_to_sun : 0;
        //f->viewfactor = f->enlightened ? 1 : -1;
    }
    printf("MISMATCH: %d\nENLIGHTENED: %d\n", mismatch, enlightened);
}

// Calcul de la temperature.
inline void Asteroid::calctemperature(Face *f)
{
    const double depth = 0.5;
    const double dz = depth/double(TEMPDIV);

    //#define DERIV(i) ((f->temp[i+1]-f->temp[i] + f->temp[i]-f->temp[i-1])/(2*dz))
    #define DERIV(i) ((f->temp[i+1]-f->temp[i])/dz)
    for(int i = 1; i < TEMPDIV - 1; ++i)
    {
        const double surf_derivative = 1/(f->h * (1-i*dz/f->h));
        f->tempn[i] = f->temp[i] + timestep * diffusivity * ( (f->temp[i+1] - 2*f->temp[i] + f->temp[i-1]) / (dz * dz) + surf_derivative * DERIV(i));
    }
    #undef DERIV

    double light = f->viewfactor >= 0 ? (1-albedo) * (f->viewfactor * C_SOLARFLUX / (pos.squaredlen())) : 0;
    f->tempn[0] = f->tempn[1] + (dz/conductivity) * (light - pow(f->temp[0], 4) * C_STEFANBOLTZMANN);
    f->tempn[TEMPDIV-1] = f->tempn[TEMPDIV-2] = initialtemperature;

    for(int i = 0; i < TEMPDIV; ++i)
    {
        f->temp[i] = f->tempn[i];
    }
}

// Calculate the emitted rays
void Asteroid::calcemitted()
{
    // calcule la temperature
    //mdl->faces[i]->temperature = mdl->faces[i]->eqtemperature + 50 * mdl->faces[i]->viewangle.back();
    #pragma omp parallel for
    for(int i = 0; i < mdl->faces.size(); ++i) calctemperature(mdl->faces[i]);
    
    for(int i = 0; i < TEMPDIV; ++i)
    {
        templog->printf("%.3f %d %.5f\n", time, i, mdl->faces[mdl->faces.size()/4]->temp[i]);
    }
    templog->printf("\n");

    for(int i = 0; i < mdl->faces.size(); ++i)
    {
        // calcul de la force
        Force f;
        f.f = -mdl->faces[i]->n;

        f.f.normalize().mul((2.0/3.0) * emissivity * C_STEFANBOLTZMANN * pow(mdl->faces[i]->temp[0], 4.0) * mdl->faces[i]->area / C_CELERITY);
        f.p = (*mdl->faces[i]->vertices[0] + *mdl->faces[i]->vertices[1] + *mdl->faces[i]->vertices[2]) / 3.0 - centerofmass;
        forces.push_back(f);

        /*if(mdl->faces[i]->viewfactor >= 0)
        {
            Force p;
            p.f = -mdl->faces[i]->n;
            p.f.normalize((1+albedo) * mdl->faces[i]->viewfactor * C_SOLARFLUX / (C_CELERITY * pos.squaredlen()));
            f.p = (*mdl->faces[i]->vertices[0] + *mdl->faces[i]->vertices[1] + *mdl->faces[i]->vertices[2])/3.0 - centerofmass;
            p.torque = true;
        }*/
    }
}

// Forces de gravite
void Asteroid::calcgravity()
{
    Force gravity;
    gravity.f = -pos;
    gravity.f.normalize(C_SUNCONST*mass/pos.squaredlen());
    gravity.p = vec(0, 0, 0);
    gravity.torque = false;
    gravity.gravity = true;
    forces.push_back(gravity);
}

// Apppliques les forces pour deplacer le solide
void Asteroid::move(double dt)
{
    matrix omegamatrix = matrix(3, 3);
    omegamatrix[0][0] = omegamatrix[1][1] = omegamatrix[2][2] = 0;
    omegamatrix[0][1] = -rotvel.z;
    omegamatrix[1][0] = +rotvel.z;
    omegamatrix[0][2] = +rotvel.y;
    omegamatrix[2][0] = -rotvel.y;
    omegamatrix[2][1] = -rotvel.x;
    omegamatrix[1][2] = -rotvel.x;
    omegamatrix.mul(omegamatrix, rotmatrix);
    rotmatrix = rotmatrix + omegamatrix * dt;

    //if(forces.size() <= 0) return;
    vec dL(0, 0, 0); // variation du moment angulaire
    vec dP(0, 0, 0); // variation du moment lineaire (qte de mouvement)
    
    vec push = vec(0, 0, 0);
    double distance_to_sun = pos.norm();
    for(unsigned int i = 0; i < forces.size(); ++i)
    {
        if(forces[i].torque) dL += (forces[i].p^forces[i].f)*dt;
        
        if(!forces[i].gravity)
        {
            vec f = forces[i].f;
            f.mul(rotmatrix);
            push += f;
            dP += f;
        }
        else
        {
            dP += forces[i].f*dt;
        }
    }
    double dE = push.dot(vel) * dt;
    totalenergy += dE;

    vel += dP/mass;
    momentum += dL;

    // calcule la nouvelle vitesse angulaire
    // en resolvant le systeme suivant a trois inconnues (avec rotvel = (wx, wy, wz)) :
    // Lx = Ixx * wx + Ixy . wy + Ixz . wz
    // Ly = Ixy * wx + Iyy . wy + Iyz . wz
    // Lx = Ixz * wx + Iyz . wy + Izz . wz

    vec rot = rotvel;
    for(int i = 0; i < 3; ++i)
    {
        matrix m = inertia;
        m[0][i] = momentum[i]; m[1][i] = momentum[i]; m[2][i] = momentum[i];
        rotvel[i] = det3x3(m)/inertia_det;
    }
    rotaccel = (rotvel-rot)/dt;

    calcviewfactors();

    accel = dP/mass;

    pos += vel * dt;
    angle += rotvel * dt;

    forces.clear();

    // log :
    vec rotdir = rot; rotdir.normalize();
    if((loops % 10) == 0)
    {
        double da = 2 * dE * semiaxis * semiaxis / (C_SUNCONST * mass);
        log->printf("%.3f %e %e %e %e %e %e %e %e\n", time, rotaccel.norm(), rotaccel.dot(rotdir), dE, da/dt, mdl->faces[0]->temp[0], mdl->faces[mdl->faces.size()/4]->temp[0], mdl->faces[2*mdl->faces.size()/4]->temp[0], mdl->faces[3*mdl->faces.size()/4]->temp[0]);
        rotmatrix.print();
    }
}

void Asteroid::calcloop()
{
    calcgravity();
    calcemitted();
    matrix m(3, 3);
    getrotationmatrix(m, pos, vec(0, 0, -1));
    //m.print();

    move(timestep);
    time += timestep;
    ++loops;
#ifdef GUI
    if(!(loops%10))
    {
        char title[128] = "";
        sprintf(title, "Asteroid Geographos (%d %%, %.2f K)", (int)(100.0*asteroid1.time/asteroid1.period), asteroid1.mdl->faces[10]->temp[0], asteroid1.mdl->faces[10]->temp[0]);
        SDL_WM_SetCaption(title, NULL);
    }
#endif GUI    
}

void Asteroid::calcperiod()
{
    time = 0.0;
    angle = vec(0, 0, 0);

    while(time < period)
    {
        calcloop();
    }
}

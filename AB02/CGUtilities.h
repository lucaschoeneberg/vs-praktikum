//
//  CGUtilities.h
//  SimpleRayTracer
//
//  Created by Philipp Lensing on 03.09.14.
//  Copyright (c) 2014 Philipp Lensing. All rights reserved.
//

#ifndef SimpleRayTracer_CGUtilities_h
#define SimpleRayTracer_CGUtilities_h
#include <vector>
#include "vector.h"
#include "color.h"

class Material
{
public:
    Material();
    Material( const Color& Diffuse, const Color& Specular, const Color& Ambient, float SpecularExp,  float Reflectivity);
    virtual Color getDiffuseCoeff(const Vector& Pos) const;
    virtual Color getSpecularCoeff(const Vector& Pos) const;
    virtual Color getAmbientCoeff(const Vector& Pos) const;
    virtual float getSpecularExp(const Vector& Pos) const;
    virtual float getReflectivity(const Vector& Pos) const;
    

    static Material DefaultMaterial;
    static Material RedMtrl;
    static Material GreenMtrl;
    static Material BlueMtrl;
    static Material YellowMtrl;
    static Material CyanMtrl;
protected:
    Color m_DiffuseCoeff;
    Color m_SpecularCoeff;
    Color m_AmbientCoeff;
    float m_SpecularExp;

    float m_Reflectivity;
};


class Triangle
{
public:
    Triangle();
    Triangle(const Vector& a, const Vector& b, const Vector& c, const Material* mtrl);
    Vector A,B,C; // vertex-positions
    const Material* pMtrl; // pointer to triangle material
    Vector calcNormal( const Vector& PointOnTriangle) const;
protected:
    friend class Scene;
    Vector NA, NB, NC; // vertex-normals

};

class PointLight
{
public:
    PointLight();
    PointLight( const Vector& pos, const Color& intensity);
    Vector Position;
    Color Intensity;
};

class Scene
{
public:
    Scene( unsigned int SceneComplexity ); // Set SceneComplexity to zero for a scene with only 10 triangles, for a more complex scene set Scenecompleyity to 20.
    virtual ~Scene();
    const Triangle& getTriangle(unsigned int Index) const;
    unsigned int getTriangleCount() const;
    const PointLight& getLight(unsigned int Index) const;
    unsigned int getLightCount() const;
    
protected:
    Triangle* m_Triangles;
    unsigned int m_TriangleCount;
    PointLight* m_Lights;
    unsigned int m_LightCount;
    void createInnerModel( std::vector<Triangle>& Triangles, float Radius, int Slices, int Stacks) const;

};




#endif

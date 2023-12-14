#ifndef GRAPH_H
#define GRAPH_H

# include <iostream>
# include <glm/glm.hpp>
# include <vector>

using vec3 = glm::vec3;

const vec3 O = vec3(0., 0.35, -1.);
const vec3 light_point = vec3(5., 5., -10.);
const vec3 light_color = vec3(1., 1., 1.);
const float ambient = 0.05;

vec3 normalize(vec3 x);

class Object {
public:

    const vec3 position;
    const vec3 color;
    const float reflection;
    const float diffuse;
    const float specular_c;
    const float specular_k;

    Object(
        vec3 position, 
        vec3 color, 
        float reflection, 
        float diffuse, 
        float specular_c, 
        float specular_k
    );

    virtual float intersect(const vec3& origin, const vec3& dir) = 0;
    virtual vec3 get_normal(const vec3& point) = 0;
    vec3 get_color();
    virtual ~Object() {}
};

class Sphere : public Object {
public:

    const float radius;

    Sphere(
        vec3 position, 
        float radius, 
        vec3 color, 
        float reflection = .85, 
        float diffuse = 1., 
        float specular_c = .6, 
        float specular_k = 50
    );

    float intersect(const vec3& origin, const vec3& dir) override;

    vec3 get_normal(const vec3& point) override;
};

class Plane : public Object {
public:

    const vec3 normal;

    Plane(
        vec3 position, 
        vec3 normal, 
        vec3 color = vec3(1., 1., 1.), 
        float reflection = 0.15, 
        float diffuse = .75, 
        float specular_c = .3, 
        float specular_k = 50
    );

    float intersect(const vec3& origin, const vec3& dir) override;

    vec3 get_normal(const vec3& point) override;

    virtual vec3 get_color(const vec3& point);
};

// Add the CheckerboardPlane class definition after the Plane class definition
class CheckerboardPlane : public Plane {
public:
    CheckerboardPlane(
        vec3 position, 
        vec3 normal, 
        vec3 color1,  
        vec3 color2,  
        float square_size,
        float reflection = 0.15, 
        float diffuse = .75, 
        float specular_c = .3, 
        float specular_k = 50
    );

    vec3 get_color(const vec3& point) override;

private:
    vec3 color2;
    float square_size;
};

vec3 intersect_color(vec3 origin, vec3 dir, float intensity, std::vector<Object*> &scene);

void rendering(int w, int h, std::vector<Object*> &scene, std::string filename = "test.png", int numThreads = 1);

#endif // GRAPH_H
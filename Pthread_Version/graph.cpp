# include "graph.h"

# include <cmath>
# include <limits>
#include <algorithm>
# include <opencv2/opencv.hpp>
#include <pthread.h>

vec3 normalizes(const vec3 &x) { return glm::normalize(x); }

/* class Object */
Object::Object(
    vec3 position, 
    vec3 color, 
    float reflection, 
    float diffuse, 
    float specular_c, 
    float specular_k
): position(position), color(color), reflection(reflection), diffuse(diffuse), specular_c(specular_c), specular_k(specular_k) {}

vec3 Object::get_color() { return color; }

/* class Sphere */
Sphere::Sphere(
    vec3 position, 
    float radius, 
    vec3 color, 
    float reflection, 
    float diffuse, 
    float specular_c, 
    float specular_k
): Object(position, color, reflection, diffuse, specular_c, specular_k), radius(radius) {}

float Sphere::intersect(const vec3& origin, const vec3& dir) {

    vec3 OC = position - origin;

    if (glm::length(OC) < radius || glm::dot(OC, dir) < 0) return std::numeric_limits<float>::infinity();

    float l = glm::length(glm::dot(OC, dir));
    float m_square = glm::length(OC) * glm::length(OC) - l * l;
    float q_square = radius * radius - m_square;
    return (q_square >= 0) ? (l - sqrt(q_square)) : std::numeric_limits<float>::infinity();
}

vec3 Sphere::get_normal(const vec3& point) { return normalizes(point - position); }

/* class Plane */
Plane::Plane(
    vec3 position, 
    vec3 normal, 
    vec3 color, 
    float reflection, 
    float diffuse, 
    float specular_c, 
    float specular_k
): Object(position, color, reflection, diffuse, specular_c, specular_k), normal(normal) {}

float Plane::intersect(const vec3& origin, const vec3& dir) {
    float dn = glm::dot(dir, normal);
    if (std::abs(dn) < 1e-6) {
        return std::numeric_limits<float>::infinity();
    }
    float d = glm::dot(position - origin, normal) / dn;
    return d > 0 ? d : std::numeric_limits<float>::infinity();
}

vec3 Plane::get_normal(const vec3& point) { return normal; }

vec3 Plane::get_color(const vec3& point) {
    return color;  // or any default color logic you want
}

// Add the CheckerboardPlane class definition after the Plane class definition
CheckerboardPlane::CheckerboardPlane(
    vec3 position, 
    vec3 normal, 
    vec3 color1,  
    vec3 color2,  
    float square_size,
    float reflection, 
    float diffuse, 
    float specular_c, 
    float specular_k
): Plane(position, normal, color1, reflection, diffuse, specular_c, specular_k), color2(color2), square_size(square_size) {}

// Modify the CheckerboardPlane class implementation
vec3 CheckerboardPlane::get_color(const vec3& point) {
    float x = point.x - position.x;
    float z = point.z - position.z;
    int squareX = static_cast<int>(std::floor(x / square_size));
    int squareZ = static_cast<int>(std::floor(z / square_size));

    if ((squareX + squareZ) % 2 == 0) {
        return color;
    } else {
        return color2;
    }
}

/* Other */
vec3 intersect_color(const vec3 origin, vec3 dir, float intensity, std::vector<Object*> &scene) {
    float min_distance = std::numeric_limits<float>::infinity();
    size_t obj_index = -1;

    for (size_t i = 0; i < scene.size(); ++i) {
        float current_distance = scene[i]->intersect(origin, dir);
        if (current_distance < min_distance) {
            min_distance = current_distance;
            obj_index = i;
        }
    }

    if (min_distance == std::numeric_limits<float>::infinity() || intensity < 0.01) return vec3(0., 0., 0.);

    Object* obj = scene[obj_index];
    const vec3 P = origin + dir * min_distance;

    vec3 color = vec3(0., 0., 0.);  // Default color

    if (obj != nullptr) {
        CheckerboardPlane* checkerboardObj = dynamic_cast<CheckerboardPlane*>(obj);
        if (checkerboardObj != nullptr) {
            color = checkerboardObj->get_color(P);
        } else {
            color = obj->get_color();
        }
    }

    const vec3 N = obj->get_normal(P);
    const vec3 PL = normalizes(light_point - P);
    const vec3 PO = normalizes(origin - P);

    vec3 c = ambient * color;
    std::vector<float> l;
    for (size_t i = 0; i < scene.size(); ++i) {
        if (i != obj_index)
            l.push_back(scene[i]->intersect(P + N * .0001f, PL));
    }
    if (!(l.size() > 0 && *std::min_element(l.begin(), l.end()) < glm::length(light_point - P))) {
        c += obj->diffuse * std::max(glm::dot(N, PL), 0.f) * color * light_color;
        c += obj->specular_c * powf(std::max(glm::dot(N, normalizes(PL + PO)), 0.f), obj->specular_k) * light_color;
    }
    vec3 reflect_ray = dir - 2 * glm::dot(dir, N) * N;
    c += obj->reflection * intersect_color(P + N * .0001f, reflect_ray, obj->reflection * intensity, scene);
    return glm::clamp(c, 0.f, 1.f);
}

void* renderThread(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    data->startTime = std::chrono::high_resolution_clock::now();
    
    float r = float(data->width) / data->height;
    glm::vec4 S = glm::vec4(-1., -1. / r + .25, 1., 1. / r + .25);
    vec3 Q = vec3(0., 0., 0.);

    for (int i = 0; i < data->width; ++i) {
        for (int j = data->startRow; j < data->endRow; ++j) {
            Q.x = S.x + i * (S.z - S.x) / (data->width - 1);
            Q.y = S.y + j * (S.w - S.y) / (data->height - 1);
            vec3 color = intersect_color(O, normalizes(Q - O), 1, *data->scene);
            data->image.at<cv::Vec3f>(data->height - j - 1, i) = cv::Vec3f(color.x, color.y, color.z);
        }
    }
    data->endTime = std::chrono::high_resolution_clock::now();
    pthread_exit(nullptr);
}

void rendering(int w, int h, std::vector<Object*> &scene, std::string filename, int numThreads) {
    cv::Mat img(h, w, CV_32FC3);
    //int numThreads = 3;  // Change the number of threads as needed

    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];

    for (int i = 0; i < numThreads; ++i) {
        threadData[i].width = w;
        threadData[i].height = h;
        threadData[i].startRow = i * (h / numThreads);
        threadData[i].endRow = (i + 1) * (h / numThreads);
        threadData[i].scene = &scene;
        threadData[i].image = img;

        pthread_create(&threads[i], nullptr, renderThread, &threadData[i]);
    }

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], nullptr);
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(threadData[i].endTime - threadData[i].startTime).count();
        std::cout << "Thread " << i << " execution time: " << duration << " milliseconds" << std::endl;
    }

    img *= 255;
    img.convertTo(img, CV_8UC3);
    cv::imwrite(filename, img);
}

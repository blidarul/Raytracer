#pragma once

#include "hittable_list.h"
#include "camera.h"

class Scene
{
public:
    // Scene parameters
    CameraPosition cam_pos;
    double focus_distance;
    double defocus_angle;
    HittableList world;

    Scene(int scene_number)
    {
        switch (scene_number)
        {
            case 1:
            three_spheres();
            break;
            case 2:
            multiple_spheres();
            break;

            default:
            three_spheres();
            break;
        }
    }


private:
    // Scene creators

    void three_spheres()
    {
        auto material_ground = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
        auto material_center = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
        auto material_left = std::make_shared<Dielectric>(1.50);
        auto material_bubble = std::make_shared<Dielectric>(1.00 / 1.50);
        auto material_right = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 1.0);

        world.add(std::make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground));
        world.add(std::make_shared<Sphere>(Point3(0.0, 0.0, -1.2), 0.5, material_center));
        world.add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));
        world.add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.4, material_bubble));
        world.add(std::make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));

        cam_pos = CameraPosition(Point3(-2, 2, 1), Point3(0, 0, -1), Vec3(0, 1, 0));
        focus_distance = 3.4;
        defocus_angle = 2;
    }

    void multiple_spheres()
    {
        auto ground_material = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
        world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

        for (int a = -11; a < 11; a++)
        {
            for (int b = -11; b < 11; b++)
            {
                auto choose_mat = random_double();
                Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

                if ((center - Point3(4, 0.2, 0)).length() > 0.9)
                {
                    std::shared_ptr<Material> sphere_material;

                    if (choose_mat < 0.8)
                    {
                        // diffuse
                        auto albedo = Color::random() * Color::random();
                        sphere_material = std::make_shared<Lambertian>(albedo);
                        world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    }
                    else if (choose_mat < 0.95)
                    {
                        // metal
                        auto albedo = Color::random(0.5, 1);
                        auto fuzz = random_double(0, 0.5);
                        sphere_material = std::make_shared<Metal>(albedo, fuzz);
                        world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    }
                    else
                    {
                        // glass
                        sphere_material = std::make_shared<Dielectric>(1.5);
                        world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    }
                }
            }
        }

        auto material1 = std::make_shared<Dielectric>(1.5);
        world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

        auto material2 = std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
        world.add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

        auto material3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
        world.add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

        cam_pos = CameraPosition(Point3(13, 2, 3), Point3(0, 0, 0), Vec3(0, 1, 0));
        focus_distance = 10;
        defocus_angle = 0.6;
    }

};
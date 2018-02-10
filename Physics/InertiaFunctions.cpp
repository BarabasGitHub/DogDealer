#include "InertiaFunctions.h"

#include <Math\FloatMatrixOperators.h>
#include <Math\TransformFunctions.h>
#include <BoundingShapes\OrientedBoxFunctions.h>
#include <BoundingShapes\SphereFunctions.h>

using namespace Math;

namespace Physics
{
    Inertia CreateInertia( 
        Math::Float3 axis_moment, 
        float mass )
    {
        return{ {
                axis_moment.x, 0, 0,
                0, axis_moment.y, 0,
                0, 0, axis_moment.z
            }, {
             mass
            }
        };
    }


    Inertia CreateSphereInertia( 
        float radius, 
        float mass 
        )
    {
        return CreateInertia(2.f/5.f * mass * radius * radius, mass);
    }


    Inertia CreateInertia( 
        BoundingShapes::Sphere const & sphere, 
        float mass 
        )
    {
        auto sphere_inertia = CreateSphereInertia(sphere.radius, mass);
        sphere_inertia = TranslateAwayFromCenterOfMass(sphere_inertia, sphere.center);
        return sphere_inertia;
    }


    Inertia CreateBoxInertia( 
        Math::Float3 size, 
        float mass 
        )
    {
        auto box_axis_inertia = CalculateInertiaAroundAxisForBox( size, mass );
        return CreateInertia(box_axis_inertia, mass);
    }


    Inertia CreateInertia( 
        BoundingShapes::OrientedBox const & box, 
        float mass 
        )
    {
        auto box_inertia = CreateBoxInertia( box.extent + box.extent, mass );
        box_inertia.moment = RotateMomentOfInertia(box_inertia.moment, box.rotation);
        box_inertia = TranslateAwayFromCenterOfMass(box_inertia, box.center);
        return box_inertia;
    }


    Inertia CreateInverseBoxInertia( 
        Math::Float3 size, 
        float mass 
        )
    {
        auto box_axis_inertia = CalculateInertiaAroundAxisForBox( size, mass );
        auto inverse_box_axis_inertia = 1 / box_axis_inertia;
        return CreateInertia( inverse_box_axis_inertia, 1 / mass );
    }


    Math::Float3 CalculateInertiaAroundAxisForBox( 
        Math::Float3 box_size, 
        float mass 
        )
    {
        auto size_squared = box_size * box_size;
        auto result = Math::Float3{ size_squared.y + size_squared.z, size_squared.z + size_squared.x, size_squared.x + size_squared.y };
        return result * (mass / 12);
    }


    Inertia Invert( Inertia i )
    {
        return{ Inverse( i.moment ), 1 / i.mass };
    }


    Math::Float3x3 RotateMomentOfInertia( 
        Math::Float3x3 moment_of_inertia, 
        const Math::Quaternion& rotation 
        )
    {
        auto rotation_matrix = RotationToFloat3x3( rotation );
        moment_of_inertia = MultiplyTransposed(rotation_matrix, MultiplyTransposed(rotation_matrix, moment_of_inertia));
        return moment_of_inertia;
    }


    Inertia Add(
        Inertia a, 
        Inertia b
        )
    {
        a.mass += b.mass;
        a.moment += b.moment;
        return a;
    }


    Inertia TranslateAwayFromCenterOfMass(
        Inertia i, 
        Math::Float3 translation
        )
    {
        auto tt = translation * translation;
        auto xy = translation.x * translation.y;
        auto yz = translation.y * translation.z;
        auto zx = translation.z * translation.x;
        i.moment += i.mass * Float3x3(
            tt.y + tt.z, xy, zx,
            xy, tt.x + tt.z, yz,
            zx, yz, tt.y + tt.x
            );
        return i;
    }

    namespace
    {
        template<typename ShapeType>
        float TotalVolume(Range<ShapeType const*> shapes)
        {
            float total_volume = 0;
            for( auto i = 0; i < Size(shapes); ++i )
            {
                total_volume += Volume(shapes[i]);
            }
            return total_volume;
        }


        template <typename ShapeType>
        void CalculateTotalInertiaImpl(
            Range<ShapeType const *> shapes, 
            float total_volume, 
            float total_mass, 
            Inertia & total_inertia_output, 
            Math::Float3 & center_of_mass_output
            )
        {
            auto density = total_mass / total_volume;
            Inertia total_inertia = CreateInertia(First(shapes), Volume(First(shapes)) * density);
            auto center_of_mass = First(shapes).center;
            for( auto i = 1; i < Size(shapes); ++i )
            {
                auto & sphere = shapes[i];
                auto volume = Volume(sphere);
                auto mass = density * volume;
                auto inertia = CreateInertia(sphere, mass);
                Combine(inertia, sphere.center, total_inertia, center_of_mass, total_inertia, center_of_mass);
            }
            total_inertia_output = total_inertia;
            center_of_mass_output = center_of_mass;
        }


        template <typename ShapeType>
        void CalculateTotalInertiaImpl(
            Range<ShapeType const *> shapes, 
            float total_mass, 
            Inertia & total_inertia_output, 
            Math::Float3 & center_of_mass_output
            )
        {
            assert(!IsEmpty(shapes));
            float total_volume = TotalVolume(shapes);
            CalculateTotalInertiaImpl(shapes, total_volume, total_mass, total_inertia_output, center_of_mass_output);
        }
    }


    void CalculateTotalInertia(
        Range<BoundingShapes::Sphere const *> spheres, 
        float total_mass, 
        Inertia & total_inertia, 
        Math::Float3 & center_of_mass
        )
    {
        CalculateTotalInertiaImpl(spheres, total_mass, total_inertia, center_of_mass);
    }


    void CalculateTotalInertia(
        Range<BoundingShapes::OrientedBox const *> boxes, 
        float total_mass, 
        Inertia & total_inertia_output
        )
    {
        Math::Float3 center_of_mass;
        CalculateTotalInertia(boxes, total_mass, total_inertia_output, center_of_mass);
    }


    void CalculateTotalInertia(
        Range<BoundingShapes::OrientedBox const *> boxes, 
        float total_mass, 
        Inertia & total_inertia_output, 
        Math::Float3 & center_of_mass_output
        )
    {
        CalculateTotalInertiaImpl(boxes, total_mass, total_inertia_output, center_of_mass_output);
    }


    void CalculateTotalInertia(
        Range<BoundingShapes::Sphere const *> spheres, 
        Range<BoundingShapes::OrientedBox const *> boxes, 
        float total_mass, 
        Inertia & total_inertia, 
        Math::Float3 & center_of_mass
        )
    {
        auto total_volume = TotalVolume(spheres) + TotalVolume(boxes);
        Inertia spheres_inertia{};
        Math::Float3 spheres_center_of_mass{};
        auto no_spheres = IsEmpty(spheres);
        auto no_boxes = IsEmpty(boxes);
        assert(!no_spheres || !no_boxes);
        if(!no_spheres)
        {
            CalculateTotalInertiaImpl(spheres, total_volume, total_mass, spheres_inertia, spheres_center_of_mass);
        }
        Inertia boxes_inertia{};
        Math::Float3 boxes_center_of_mass{};
        if(!no_boxes)
        {
            CalculateTotalInertiaImpl(boxes, total_volume, total_mass, boxes_inertia, boxes_center_of_mass);
        }

        if(!no_spheres && !no_boxes)
        {
            Combine(spheres_inertia, spheres_center_of_mass, boxes_inertia, boxes_center_of_mass, total_inertia, center_of_mass);
        }
        else if(!no_spheres)
        {
            total_inertia = spheres_inertia;
            center_of_mass = spheres_center_of_mass;
        }
        else //if(!no_boxes)
        {
            total_inertia = boxes_inertia;
            center_of_mass = boxes_center_of_mass;
        }
    }


    void Combine(
        Inertia a, 
        Math::Float3 center_of_mass_a, 
        Inertia b,
        Math::Float3 center_of_mass_b, 
        Inertia & i_out, 
        Math::Float3 & center_of_mass_out
        )
    {
        auto new_center_of_mass = center_of_mass_a * a.mass + center_of_mass_b * b.mass;
        new_center_of_mass /= a.mass + b.mass;
        center_of_mass_out = new_center_of_mass;
        a = TranslateAwayFromCenterOfMass(a, new_center_of_mass - center_of_mass_a);
        b = TranslateAwayFromCenterOfMass(b, new_center_of_mass - center_of_mass_b);
        i_out = Add(a, b);
    }


    Inertia Combine(
        Inertia const & a, 
        Math::Float3 const & center_of_mass_a, 
        Inertia const & b, 
        Math::Float3 const & center_of_mass_b
        )
    {
        Inertia result;
        Math::Float3 new_center_of_mass;
        Combine(a, center_of_mass_a, b, center_of_mass_b, result, new_center_of_mass);
        return result;
    }
}
#include "CppUnitTest.h"

#include <GameLogic\AINavigation.h>

#include <Math\MathFunctions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Logic;

namespace DogDealerLogic
{
    TEST_CLASS(AINavigationTest)
    {
    public:

        // ####################
        // ########## Triangles in X-Y layer
        // ####################

        // Ensure that the distance between a triangle 
        // and a point placed exactly on its surface is zero
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Flat)
        {
            auto position = Math::Float3(0, 0, 0);
            auto vertex_0 = Math::Float3(1, -1, 0);
            auto vertex_2 = Math::Float3(0, -1, 0);
            auto vertex_1 = Math::Float3(0, 1, 0);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            Assert::AreEqual(distance, 0.0f);
        }

        // Same as TestGetPointDistanceFromTriangle3D_Flat,
        // but has the triangle normal face downward 
        // so that it must be flipped
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_FlatFlipped)
        {           
            auto position = Math::Float3(0, 0, 0);
            auto vertex_0 = Math::Float3(1, -1, 0);
            auto vertex_1 = Math::Float3(0, -1, 0);
            auto vertex_2 = Math::Float3(0, 1, 0);            

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            Assert::AreEqual(distance, 0.0f);
        }

        // Ensure that the distance between a point and a triangle
        // placed above it is the same as their z offset
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Z_Offset)
        {
            auto position = Math::Float3(0, 0, 0);
            auto vertex_0 = Math::Float3(1, -1, 1);
            auto vertex_1 = Math::Float3(0, -1, 1);
            auto vertex_2 = Math::Float3(0, 1, 1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            Assert::AreEqual(distance, 1.0f);
        }

        // Same as TestGetPointDistanceFromTriangle3D_Z_Offset,
        // but also shift the point down
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Z_Offset2)
        {
            auto position = Math::Float3(0, 0, -1);
            auto vertex_0 = Math::Float3(1, -1, 1);
            auto vertex_1 = Math::Float3(0, -1, 1);
            auto vertex_2 = Math::Float3(0, 1, 1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            Assert::AreEqual(distance, 2.0f);
        }

        // Same as TestGetPointDistanceFromTriangle3D_Z_Offset,
        // but also shift the point sideways
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Offset)
        {
            auto position = Math::Float3(0.1f, 0.5f, 0);
            auto vertex_0 = Math::Float3(1, -1, 1);
            auto vertex_1 = Math::Float3(0, -1, 1);
            auto vertex_2 = Math::Float3(0, 1, 1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            Assert::AreEqual(distance, 1.0f);
        }

        // ####################
        // ########## Triangles at an angle
        // ####################
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Angled)
        {
            auto position = Math::Float3(0, 0, 0);
            auto vertex_0 = Math::Float3(0, 1, 0);
            auto vertex_1 = Math::Float3(0, -1, 1);
            auto vertex_2 = Math::Float3(0, -1, -1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            Assert::AreEqual(distance, 0.0f);
        }

        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Angled2)
        {
            auto position = Math::Float3(0, 0, 0);
            auto vertex_0 = Math::Float3(1, 1, 0);
            auto vertex_1 = Math::Float3(1, -1, 1);
            auto vertex_2 = Math::Float3(1, -1, -1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);
            
            // Rounding issues lead to low precision
            auto error = abs(distance - 1.0);
            Assert::IsTrue( error < 0.01f);            
        }

        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Angled3)
        {
            auto position = Math::Float3(-1, 0, 0);
            auto vertex_0 = Math::Float3(1, 1, 0);
            auto vertex_1 = Math::Float3(1, -1, 1);
            auto vertex_2 = Math::Float3(1, -1, -1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            // Rounding issues lead to low precision
            auto error = abs(distance - 2.0);
            Assert::IsTrue(error < 0.01f);
        }

        // Use a triangle tilted by 45 degrees, so that the distance
        // to the origin should be Norm(0.5, 0.5)
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Angled4)
        {
            auto position = Math::Float3(0, 0, 0);
            auto vertex_0 = Math::Float3(-1, -1, 0);
            auto vertex_1 = Math::Float3(-1, 1, 0);
            auto vertex_2 = Math::Float3(0, 0, 1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            auto target_distance = Math::Norm(Math::Float2(0.5, 0.5));

            // Rounding issues lead to low precision
            auto error = abs(distance - target_distance);
            Assert::IsTrue(error < 0.01f);
        }

        // Tilt and shift the triangle in a way that should
        // have one corner closest to a point that is not the origin
        TEST_METHOD(TestGetPointDistanceFromTriangle3D_Angled5)
        {
            auto position = Math::Float3(1, 0, 0);
            auto vertex_0 = Math::Float3(-1, 0, 0);
            auto vertex_1 = Math::Float3(-2, 0, 0);
            auto vertex_2 = Math::Float3(-1, 1, -1);

            auto distance = GetPointDistanceFromTriangle3D(position, vertex_0, vertex_1, vertex_2);

            // Vertex 0 should be closest regardless of angle
            auto target_distance = 2;

            // Rounding issues lead to low precision
            auto error = abs(distance - target_distance);
            Assert::IsTrue(error < 0.01f);
        }
    };
}
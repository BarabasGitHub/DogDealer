#pragma warning(disable: 4505)
#include "CppUnitTest.h"

#include <Math\UnitTests\MathToStringForUnitTest.h>
#include <Conventions\OrientationFunctions.h>

#include <Math\MathFunctions.h>
#include <Math\FloatOperators.h>

#include <Physics\ManifoldFunctions.h>

#include <Utilities\UnitTest\ToString.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace Math;
using namespace Physics;

namespace DogDealerPhysicsUnitTests
{
    TEST_CLASS( CreateManifoldUnitTest )
    {
    public:

        TEST_METHOD( ManifoldTest1 )
        {

            // First we determine what edges are involved in the collision.
            // Then we identify the reference edge and incident edge. The reference edge is the edge most perpendicular to the separation normal.
            // The reference edge will be used to clip the incident edge vertices to generate the contact manifold.
            // uint8_t FindContactPoints( Math::Float3 extent, BoundingShapes::OrientedBox oriented_box, Math::Float3 separation_axis, Range<Manifold::ContactPoint*> output_contact_points )

            Math::Float3 extend = 1;
            BoundingShapes::OrientedBox box;
            box.center = { 0, 0, 1 };
            box.extent = 1;
            box.rotation = EulerToQuaternion( { 0, c_PI_div_4.f / 2, 0 } );

            Math::Float3 separation_axis = { 0, 0, -1 };

            BoundingShapes::OrientedBox extent_box;
            extent_box.center = 0;
            extent_box.extent = extend;
            extent_box.rotation = Math::Identity();
            auto manifold = CreateManifold( extent_box, box );

            // transform the separating axis
            separation_axis = InverseRotate( -separation_axis, box.rotation );
            // swap the boxes from place and such
            box.center = -InverseRotate( box.center, box.rotation );
            box.rotation = Conjugate( box.rotation );
            swap( extend, box.extent );

            extent_box.center = 0;
            extent_box.extent = extend;
            extent_box.rotation = Math::Identity();
            auto manifold2 = CreateManifold( extent_box, box );

            Assert::AreEqual( manifold.contact_point_count, manifold2.contact_point_count);
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
                Assert::IsTrue(manifold2.penetration_depths[i] >= 0);
            }

            //auto contact_points1 = CreateRange( output_contact_points, 0, number_of_contact_points );
            //auto contact_points2 = CreateRange( output_contact_points2, 0, number_of_contact_points2 );

            //for( auto & point : contact_points2 )
            //{
            //    point -= box.center;
            //    point = InverseRotate( point, box.rotation );
            //}

            //sort( begin( contact_points1 ), end( contact_points1 ), LessXYZ );
            //sort( begin( contact_points2 ), end( contact_points2 ), LessXYZ );

            //auto message = L"\nFirst: " + ToString( contact_points1 ) + L"\n" + L"Second: " + ToString( contact_points2 );
            //Assert::IsTrue( Math::Equal( contact_points1, contact_points2, 0.00001f ), message.c_str() );
        }


        TEST_METHOD( ManifoldTest2 )
        {

            // First we determine what edges are involved in the collision.
            // Then we identify the reference edge and incident edge. The reference edge is the edge most perpendicular to the separation normal.
            // The reference edge will be used to clip the incident edge vertices to generate the contact manifold.
            // uint8_t FindContactPoints( Math::Float3 extent, BoundingShapes::OrientedBox oriented_box, Math::Float3 separation_axis, Range<Manifold::ContactPoint*> output_contact_points )

            BoundingShapes::OrientedBox box1, box2;
            box1.center = 0;
            box1.extent = { 0.183846995f, 0.764730990f, 0.0249205008f };
            box1.rotation = Identity();
            box2.center = { -0.419835031f, -1.09512067f, -0.334004045f };
            box2.extent = { 0.734443009f, 0.150303006f, 0.913121521f };
            box2.rotation = { -0.681869388f, -0.435961664f, -0.449241132f, 0.378383160f };

            Math::Float3 separation_axis = { 0.310016870f, -0.000000000f, 0.282726437f };

            auto manifold = CreateManifold( box1, box2 );

            Assert::AreEqual( uint8_t(2), manifold.contact_point_count );
            Assert::IsTrue( Equal(Normalize(separation_axis), manifold.separation_axes[0], 1e-6f) );
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
        }


        TEST_METHOD( ManifoldTest3 )
        {

            BoundingShapes::OrientedBox box1, box2;
            box1.center = 0;
            box1.extent = { 0.734443009f, 0.150303006f, 0.913121521f };
            box1.rotation = { 0.000000000f, 0.000000000f, 0.313453108f, -0.949603677f };
            box2.center = { -0.0462598801f, 0.00931847095f, -0.0198591650f };
            box2.extent = { 0.734443009f, 0.150303006f, 0.913121521f };
            box2.rotation = { 0, 0, -0.738910556f, -0.673803627f };

            //Math::Float3 separation_axis = { 0.310016870f, -0.000000000f, 0.282726437f };

            auto manifold = CreateManifold( box1, box2 );
            Assert::IsTrue( manifold.contact_point_count > 0 );
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
            //Assert::AreEqual( uint8_t( 2 ), manifold.contact_point_count );
            //Assert::AreEqual( separation_axis, manifold.contact_points[0].separation_axis );
        }


        TEST_METHOD( ManifoldTest4 )
        {

            BoundingShapes::OrientedBox box1, box2;
            box1.center = 0;
            box1.extent = { 1.00000000f, 1.00000048f, 1.00000000f };
            box1.rotation = Math::Identity();
            box2.center = { 0.185758829f, 0.256420851f, 2.53377104f };
            box2.extent = { 1.00000000f, 1.00000048f, 1.00000000f };
            box2.rotation = { -0.533405721f, 0.536373258f, -0.00258099684f, 0.654074967f };

            Math::Float3 separation_axis = { 0.f, 0.f, -1.f };

            auto manifold = CreateManifold( box1, box2 );
            Assert::IsTrue( manifold.contact_point_count > 0 );
            Assert::AreEqual( uint8_t( 1 ), manifold.contact_point_count );
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::AreEqual( separation_axis, manifold.separation_axes[i] );
            }
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
        }


        TEST_METHOD( ManifoldTest5 )
        {

            BoundingShapes::OrientedBox box1, box2;
            box1.center = 0;
            box1.extent = { 1.00000000f, 1.00000048f, 1.00000000f };
            box1.rotation = {-1.20940531e-005f, -0.000499999267f, 0.707108617f, 0.707103610f};
            box2.center = { 8.61170338e-006f, 3.58423858e-006f, -1.80175698f};
            box2.extent = { 1.00000000f, 1.00000048f, 1.00000000f };
            box2.rotation = {-4.56918679e-005f, -6.69681031e-005f, 0.707107127f, 0.707104981f};

            auto manifold = CreateManifold( box1, box2 );

            Assert::IsTrue( manifold.contact_point_count > 0 );
            for(auto i = 0u; i < manifold.contact_point_count; ++i)
            {
                Assert::IsTrue( manifold.penetration_depths[i] >= 0);
            }

            auto manifold2 = CreateManifold(box2, box1);
            Assert::IsTrue(manifold2.contact_point_count > 0);
            for(auto i = 0u; i < manifold2.contact_point_count; ++i)
            {
                Assert::IsTrue(manifold2.penetration_depths[i] >= 0);
            }
        }



        TEST_METHOD(ManifoldTest6)
        {

            BoundingShapes::OrientedBox box1, box2;
            box1.center = 0;
            box1.extent = {1.00000000f, 1.00000048f, 1.00000000f};
            box1.rotation = Math::Identity();
            box2.center = {-1.89713395f, 0.718905747f, 1.59242380f};
            box2.extent = {1.00000000f, 1.00000048f, 1.00000000f};
            box2.rotation = {-0.656450927f, -0.593110263f, 0.265075654f, 0.382595748f};

            Math::Float3 separation_axis = {0.575861931f, -0.00238585472f, -0.816748857f};

            auto manifold = CreateManifold(box1, box2);
            // I don't know what the correct outcome is.
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
            //Assert::IsTrue( manifold.contact_point_count > 0 );
            //Assert::AreEqual( uint8_t( 1 ), manifold.contact_point_count );
            //for( auto i = 0u; i < manifold.contact_point_count; ++i )
            //{
            //    Assert::AreEqual( separation_axis, manifold.separation_axes[i] );
            //}
        }


        TEST_METHOD(ManifoldTest7)
        {

            BoundingShapes::OrientedBox box1, box2;
            box1.center = 0;
            box1.extent = {0.151770532f, 0.241953611f, 1.11494708f};
            box1.rotation = {0,0,0,1};
            box2.center = {-0.585899651f, -0.263753980f, -1.22962248f};
            box2.extent = {1.00000012f, 1.00000000f, 1.00000000f};
            box2.rotation = {-0.135179490f, -0.0420587882f, -0.00173929892f, 0.989737272f};

            auto manifold = CreateManifold(box1, box2);

            Assert::IsTrue(manifold.contact_point_count > 0);
            for(auto i = 0u; i < manifold.contact_point_count; ++i)
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }

            auto manifold2 = CreateManifold(box2, box1);
            Assert::IsTrue(manifold2.contact_point_count > 0);
            for(auto i = 0u; i < manifold2.contact_point_count; ++i)
            {
                Assert::IsTrue(manifold2.penetration_depths[i] >= 0);
            }
        }


        TEST_METHOD( ManifoldTestTriangleBox1 )
        {
            BoundingShapes::AxisAlignedBox box;
            box.center = 0;
            box.extent = { 0.581016541f, 0.407437027f, 0.899468958f };

            BoundingShapes::Triangle triangle;
            triangle.corners[0] = { 0.775226653f, 2.58082056f, 0.793138385f };
            triangle.corners[1] = { 1.97719550f, 0.982365370f, 0.807621717f };
            triangle.corners[2] = { 0.405173361f, -0.202916026f, 0.455693960f };

            auto manifold = CreateManifold( box, triangle );
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
        }


        TEST_METHOD( ManifoldTestTriangleBox2 )
        {
            BoundingShapes::AxisAlignedBox box;
            box.center = 0;
            box.extent = { 0.581016541f, 0.407437027f, 0.899468958f };

            BoundingShapes::Triangle triangle;
            triangle.corners[0] = { 0.545741796f, 0.0324633121f, -0.583809257f };
            triangle.corners[1] = { 0.0535955429f, 0.110085726f, -0.504532218f };
            triangle.corners[2] = { 0.0476493835f, 0.0917367935f, -0.516541481f };

            auto manifold = CreateManifold( box, triangle );
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
        }


        TEST_METHOD(ManifoldTestTriangleBox3)
        {
            BoundingShapes::OrientedBox box;
            box.center = { 0.315837860f, 0.821987152f, 0.865023494f };
            box.extent = { 0.734443009f, 0.150303006f, 0.913121521f };
            box.rotation = { 0.000000000f, 0.000000000f, 0.0657236055f, -0.997837901f };

            BoundingShapes::Triangle triangle;
            triangle.corners[0] = { 1.00000000f, 0.999998987f, 0.000000000f };
            triangle.corners[1] = { -1.00000000f, 1.00000000f, -1.00000000f };
            triangle.corners[2] = { -1.00000000f, 1.00000000f, 0.000000000f };

            auto manifold = CreateManifold(box, triangle);
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                Assert::IsTrue(manifold.penetration_depths[i] >= 0);
            }
        }
    };
}
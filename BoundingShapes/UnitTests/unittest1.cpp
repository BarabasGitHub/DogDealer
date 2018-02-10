#include "CppUnitTest.h"

#include <BoundingShapes\IntersectionTests.h>

#include <Math/FloatOperators.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace Math;
using namespace BoundingShapes;
namespace DogDealerBoundingShapesUnitTests
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestTestPlanePoint)
		{
            {
                Plane plane{ { 0, 0, 1 }, 0 };

                Float3 point_in_front{ 0, 0, 1 };

                Assert::IsTrue( InFront( point_in_front, plane ) );

                Float3 point_behind{ 0, 0, -1 };

                Assert::IsFalse( InFront( point_behind, plane ) );
            }

            {
                Plane plane{ { 0, 1, 0 }, 0 };

                Float3 point_in_front{ 0, 1, 0 };

                Assert::IsTrue( InFront( point_in_front, plane ) );

                Float3 point_behind{ 0, -1, 0 };

                Assert::IsFalse( InFront( point_behind, plane) );
            }

            {
                Plane plane{ { 1, 0, 0 }, 0 };

                Float3 point_in_front{ 1, 0, 0 };

                Assert::IsTrue( InFront( point_in_front, plane ) );

                Float3 point_behind{ -1, 0, 0 };

                Assert::IsFalse( InFront( point_behind, plane) );
            }
            
            {
                Plane plane{ { 1, 0, 0 }, 0 };

                Float3 point_in_front{ 1, 0, 0 };

                Assert::IsTrue( InFront( point_in_front, plane) );

                Float3 point_behind{ -1, 0, 0 };

                Assert::IsFalse( InFront( point_behind, plane) );
            }

		}

	};
}
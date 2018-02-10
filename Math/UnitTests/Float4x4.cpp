#include "CppUnitTest.h"

#include <Math\FloatMatrixOperators.h>
#include <Math\MathFunctions.h>
#include "MathToStringForUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Math;

namespace UnitTest
{		
	TEST_CLASS(UnitTestFloat4x4)
	{
	public:
        TEST_METHOD( Float4x4Multiplication )
        {
            Float4x4 a(
                1, 0, 1, 0,
                0, 2, 3, 0,
                0, 0, 1, 2,
                3, 2, 0, 1 );
            
            Float4x4 b(
                0, 1, 2, 0,
                2, 1, 0, 1,
                1, 0, 1, 1,
                0, 1, 1, 0 );

            Float4x4 expected_result(
                1, 1, 3, 1,
                7, 2, 3, 5,
                1, 2, 3, 1,
                4, 6, 7, 2 );

            auto result = a * b;

            Assert::AreEqual( ToString(expected_result), ToString(result) );
        }

        TEST_METHOD( Float4x4Transpose )
        {
            Float4x4 m(
                1, 0, 1, 0,
                0, 2, 3, 0,
                0, 0, 1, 2,
                3, 2, 0, 1 );

            Float4x4 expected(
                1, 0, 0, 3,
                0, 2, 0, 2,
                1, 3, 1, 0,
                0, 0, 2, 1 );

            auto t = Transpose( m );

            Assert::AreEqual( ToString( expected ), ToString( t ) );
        }

        TEST_METHOD(Float4x4Determinant)
        {
            using namespace Math;
            Float4x4 a(
                1.000, 2.000, 3.000, 4.000,
                2.000, 3.000, 5.000, 1.000,
                9.000, 6.000, 0.000, 4.000,
                1.000, 1.000, 1.000, 1.000);

            float a_determinant = 8.000;

            auto result = Determinant(a);

            Assert::AreEqual( result, a_determinant, 0.0001f );
        }

        TEST_METHOD(Float4x4Inversion)
        {
            using namespace Math;
            Float4x4 identity = Identity();

            auto result = Inverse(identity);

            Assert::IsTrue( Equal( result, identity, 0.0001f) );

            Float4x4 a(
                1.000, 2.000, 3.000, 4.000,
                2.000, 3.000, 5.000, 1.000,
                9.000, 6.000, 0.000, 4.000,
                1.000, 1.000, 1.000, 1.000);

            Float4x4 a_inverse(
                 -2.000, -1.250 , -0.750 , 12.250,
                 3.000 ,  2.125 ,  1.375 ,-19.625,
                 -1.000,  -0.500,  -0.500,  6.500,
                 0.000 , -0.375 , -0.125 ,  1.875);

            result = Inverse(a);

            Assert::IsTrue( Equal( result, a_inverse, 0.0001f ) );
        }

	};
}
#include "CppUnitTest.h"

#include <Math\FixedPoint.h>
#include <Math\FixedPointOperators.h>
#include <Math\Conversions.h>
#include "MathToStringForUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Math;

namespace UnitTest
{
    TEST_CLASS(UnitTestFixedPoint)
    {
    public:


        TEST_METHOD(TestFixedPointFromFloat)
        {
            float floating = 256.125f;
            FixedPoint expected;
            expected.integer = 256;
            expected.fraction = FixedPoint::c_fraction_factor / 8u;

            auto result = FixedPointFromFloat(floating);

            Assert::AreEqual(expected, result);
        }


        TEST_METHOD(TestFixedPointFromFloatNegative)
        {
            float floating = -256.125f;
            FixedPoint expected;
            expected.integer = -257;
            expected.fraction = FixedPoint::c_fraction_factor - (FixedPoint::c_fraction_factor / 8u);

            auto result = FixedPointFromFloat(floating);

            Assert::AreEqual(expected, result);
        }


        TEST_METHOD(TestFloatFromFixedPoint)
        {
            FixedPoint fixed;
            fixed.integer = 256;
            fixed.fraction = FixedPoint::c_fraction_factor / 8u;
            float expected = 256.125f;

            auto result = FloatFromFixedPoint(fixed);

            Assert::AreEqual(expected, result);
        }


        TEST_METHOD(TestFloatFromFixedPointNegative)
        {
            FixedPoint fixed;
            fixed.integer = -256;
            fixed.fraction = FixedPoint::c_fraction_factor / 8u;
            float expected = -255.875f;

            auto result = FloatFromFixedPoint(fixed);

            Assert::AreEqual(expected, result);
        }


        TEST_METHOD(TestAddition)
        {
            FixedPoint a = FixedPointFromFloat(123.125f);
            FixedPoint b = FixedPointFromFloat(-123.875f);

            FixedPoint expected; // -0.750
            expected.integer = -1;
            expected.fraction = (FixedPoint::c_fraction_factor / 8u) * 2u;

            auto result = a + b;

            Assert::AreEqual(expected, result);
        }


        TEST_METHOD( TestAddition2 )
        {
            FixedPoint a = FixedPointFromFloat( 0.5f );
            FixedPoint b = FixedPointFromFloat( 0.5f );

            FixedPoint expected;
            expected.integer = 1;
            expected.fraction = 0;

            auto result = a + b;

            Assert::AreEqual( expected, result );
        }


        TEST_METHOD( TestAddition3 )
        {
            FixedPoint a = FixedPointFromFloat( 0.5f );
            FixedPoint b = FixedPointFromFloat( 0.75f );

            FixedPoint expected;
            expected.integer = 3;
            expected.fraction = 0;

            auto result = a + b + a + b + a; // should be 3

            Assert::AreEqual( expected, result );
        }


        //TEST_METHOD( TestAddition4 )
        //{
        //    FixedPoint smallest;
        //    smallest.integer = -1;
        //    smallest.fraction = FixedPoint::c_fraction_factor - 1;

        //    FixedPoint expected;
        //    expected.integer = -1;
        //    expected.fraction = 0;

        //    FixedPoint result;
        //    result.integer = 0;
        //    result.fraction = 0;

        //    for( size_t i = 0u; i < FixedPoint::c_fraction_factor; ++i )
        //    {
        //        result += smallest;
        //        Assert::IsTrue( result >= expected );
        //    }

        //    Assert::AreEqual( expected, result );
        //}



        TEST_METHOD( TestSubstraction )
        {
            FixedPoint a = FixedPointFromFloat( 123.125f );
            FixedPoint b = FixedPointFromFloat( 123.875f );

            FixedPoint expected; // -0.750
            expected.integer = -1;
            expected.fraction = ( FixedPoint::c_fraction_factor / 4u );

            auto result = a - b;

            Assert::AreEqual( expected, result );
        }

    };
}
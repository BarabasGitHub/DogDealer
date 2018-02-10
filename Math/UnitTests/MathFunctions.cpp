#include "CppUnitTest.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\MathConstants.h>
#include "MathToStringForUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(UnitTestMathFunctions)
    {
    public:

        TEST_METHOD(TestRotationBetweenAxes)
        {
            using namespace Math;
            Float3 a(1,2,3);
            Float3 b(1,2,3.0001f);

            auto rotation = RotationBetweenAxes(a, b);

            Float3 expected = Normalize(b) * Norm(a);
            Float3 result = Rotate(a, rotation);

            Assert::IsTrue( Equal(expected, result, 0.0001f) );
        }


        TEST_METHOD(TestRotationBetweenAxesLargeAngle)
        {
            using namespace Math;
            Float3 a(1,2,3);
            Float3 b(3,2,1);

            auto rotation = RotationBetweenAxes(a, b);

            Float3 expected = Normalize(b) * Norm(a);
            Float3 result = Rotate(a, rotation);

            Assert::IsTrue( Equal(expected, result, 0.001f) );
        }


        TEST_METHOD(TestRotationBetweenAxesLargeAngle2)
        {
            using namespace Math;
            Float3 a(1,2,3);
            Float3 b(3,4,5);

            auto rotation = RotationBetweenAxes(a, b);

            Float3 expected = Normalize(b) * Norm(a);
            Float3 result = Rotate(a, rotation);

            Assert::IsTrue(Equal(expected, result, 0.001f), (L"Expected: " + ToString(expected) + L" Result: " + ToString(result)).c_str());
        }


        //TEST_METHOD(TestQuaternionLog)
        //{
        //    using namespace Math;

        //    Quaternion input = {0.5f, 0.3f, 0.1f, -.2f};

        //    auto output = Log(input);

        //    Quaternion expected;

        //    Assert::IsTrue(Equal(expected, output, 0.001f), (L"Expected: " + ToString(expected) + L" Result: " + ToString(output)).c_str());
        //}


        //TEST_METHOD(TestQuaternionExp)
        //{
        //    using namespace Math;

        //    Quaternion input = {0.5f, 0.3f, 0.1f, -.2f};

        //    auto output = Exp(input);

        //    Quaternion expected = {};

        //    Assert::IsTrue(Equal(expected, output, 0.001f), (L"Expected: " + ToString(expected) + L" Result: " + ToString(output)).c_str());
        //}


        TEST_METHOD(TestQuaternionLogExp)
        {
            using namespace Math;

            Quaternion input = {0.5f, 0.3f, 1.f, -.2f};

            auto output = Exp(Log(input));

            Quaternion expected = input;

            Assert::IsTrue(Equal(expected, output, 0.001f), (L"Expected: " + ToString(expected) + L" Result: " + ToString(output)).c_str());
        }

        TEST_METHOD(TestQuaternionPow)
        {
            using namespace Math;

            Quaternion input = {0.5f, 0.3f, 0.1f, -.2f};

            auto output = Pow(input, 1);

            Quaternion expected = input;

            Assert::IsTrue(Equal(expected, output, 0.001f), (L"Expected: " + ToString(expected) + L" Result: " + ToString(output)).c_str());

            //output = Pow(input, 3);

            //expected;

            //Assert::IsTrue(Equal(expected, output, 0.001f), (L"Expected: " + ToString(expected) + L" Result: " + ToString(output)).c_str());
        }

    };
}
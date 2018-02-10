#include "CppUnitTest.h"

#include <Math\TransformFunctions.h>
#include <Math\MathFunctions.h>
#include <Math\UnitTests\MathToStringForUnitTest.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Math;

namespace UnitTest
{
    TEST_CLASS( UnitTestTransformFunctions )
    {
    public:
        TEST_METHOD( TestTranslationToFloat3x3 )
        {
            Math::Float2 translation = {2, 3};
            auto transformation = TranslationToFloat3x3(translation);

            Math::Float2 point = {1, 4};

            auto expected = point + translation;

            auto result = TransformPosition(point, transformation);

            Assert::AreEqual( ToString(expected), ToString(result) );
        }


        TEST_METHOD( TestRotationToFloat3x3 )
        {
            auto rotation = XAngleToQuaternion(2);

            auto transformation = RotationToFloat3x3(rotation);

            Math::Float3 point = {1, 2, 3};

            auto expected = Rotate(point, rotation);

            auto result = transformation * point;

            Assert::IsTrue(Equal(expected, result, 0.001f));
        }


        TEST_METHOD( TestScaleToFloat3x3 )
        {
            Math::Float3 scale = {3, 5, 7};

            auto transformation = ScaleToFloat3x3(scale);

            Math::Float3 point = {1, 2, 3};

            auto expected = point * scale;

            auto result = transformation * point;

            Assert::AreEqual( ToString(expected), ToString(result));
        }


        TEST_METHOD( TestCombineFloat3x3AndTranslation )
        {
            Math::Float3 translation = {3, 5, 2};

            auto rotation = RotationToFloat3x3(XAngleToQuaternion(3));

            auto transformation = CombineFloat3x3AndTranslation(rotation, translation);

            Math::Float3 point = {3,2,1};

            auto expected = rotation * point + translation;

            auto result = TransformPosition(point, transformation);

            Assert::IsTrue(Equal(expected, result, 0.001f));
        }


        TEST_METHOD( TestTranslationToFloat4x4 )
        {
            Math::Float3 translation = {5, 3, 7};

            auto transformation = TranslationToFloat4x4(translation);

            Math::Float3 point = {1, 2, 3};

            auto expected = point + translation;

            auto result = TransformPosition(point, transformation);

            Assert::AreEqual( ToString(expected), ToString(result));
        }


        TEST_METHOD( TestRotationToFloat4x4 )
        {
            auto rotation = XAngleToQuaternion(3);

            auto transformation = RotationToFloat4x4(rotation);

            Math::Float3 point =  {3, 1, 2};

            auto expected = Rotate( point, rotation );

            auto result = TransformPosition( point, transformation );

            Assert::IsTrue(Equal(expected, result, 0.001f));
        }


        TEST_METHOD( TestScaleToFloat4x4 )
        {
            Math::Float3 scale = {4, 6, 2};

            auto transformation = ScaleToFloat4x4(scale);

            Math::Float3 point = {3, 1, 2};

            auto expected = scale * point;

            auto result = TransformPosition( point, transformation );

            Assert::AreEqual( ToString(expected), ToString(result));
        }

        TEST_METHOD( TestRotationAroundPositionToFloat4x4 )
        {
            auto rotation = XAngleToQuaternion(2);
            Math::Float3 rotation_point = {6, 1, 2};

            auto transformation = RotationAroundPositionToFloat4x4(rotation, rotation_point);
            auto expected_transformation = TranslationToFloat4x4( rotation_point ) * RotationToFloat4x4( rotation ) * TranslationToFloat4x4( -rotation_point );
            Assert::IsTrue( Equal( expected_transformation, transformation, 0.001f ) );

            Math::Float3 point = {2, 3, 1};
            auto expected = Rotate(point - rotation_point, rotation) + rotation_point;
            auto result = TransformPosition(point, transformation);

            Assert::IsTrue(Equal(expected, result, 0.001f));
        }


        TEST_METHOD( TestAffineTransform )
        {
            Math::Float3 translation = {4, 2, 1};
            auto rotation = XAngleToQuaternion(2);
            Math::Float3 scale = {5, 7, 9};
            Math::Float3 rotation_point = {6, 1, 2};

            Math::Float3 point = {2, 3, 1};

            {
                auto transformation = AffineTransform(translation, rotation);
                auto expected_transformation = TranslationToFloat4x4(translation) * RotationToFloat4x4(rotation);
                Assert::IsTrue(Equal(expected_transformation, transformation, 0.001f));

                auto expected = Rotate(point, rotation) + translation;
                auto result = TransformPosition(point, transformation);
                Assert::IsTrue(Equal(expected, result, 0.001f));
            }

            {
                auto transformation = ReverseAffineTransform(translation, rotation);
                auto expected_transformation = RotationToFloat4x4(Conjugate(rotation)) * TranslationToFloat4x4(-translation);
                Assert::IsTrue(Equal(expected_transformation, transformation, 0.001f));

                auto expected = InverseRotate(point - translation, rotation);
                auto result = TransformPosition(point, transformation);
                Assert::IsTrue(Equal(expected, result, 0.001f));
            }

            {
                auto transformation = AffineTransform(translation, scale);
                auto expected_transformation = TranslationToFloat4x4(translation) * ScaleToFloat4x4(scale);
                Assert::IsTrue(Equal(expected_transformation, transformation, 0.001f));

                auto expected = scale * point + translation;
                auto result = TransformPosition(point, transformation);
                Assert::IsTrue(Equal(expected, result, 0.001f));
            }

            {
                auto transformation = AffineTransform(translation, rotation_point, rotation);
                auto expected_transformation = TranslationToFloat4x4(translation) * TranslationToFloat4x4(rotation_point) * RotationToFloat4x4(rotation) * TranslationToFloat4x4(-rotation_point);
                Assert::IsTrue(Equal(expected_transformation, transformation, 0.001f));

                auto expected = Rotate(point - rotation_point, rotation) + rotation_point + translation;
                auto result = TransformPosition(point, transformation);
                Assert::IsTrue(Equal(expected, result, 0.001f));
            }

            {
                auto transformation = AffineTransform(translation, rotation, scale);
                auto expected_transformation = TranslationToFloat4x4(translation) * RotationToFloat4x4(rotation) * ScaleToFloat4x4(scale);
                Assert::IsTrue(Equal(expected_transformation, transformation, 0.001f));

                auto expected = Rotate(scale * point, rotation) + translation;
                auto result = TransformPosition(point, transformation);
                Assert::IsTrue(Equal(expected, result, 0.001f));
            }

            {
                auto transformation = AffineTransform(translation, rotation_point, rotation, scale);
                auto expected_transformation = TranslationToFloat4x4(translation) * TranslationToFloat4x4(rotation_point) * RotationToFloat4x4(rotation) * TranslationToFloat4x4(-rotation_point) * ScaleToFloat4x4(scale);
                Assert::IsTrue(Equal(expected_transformation, transformation, 0.001f));

                auto expected = Rotate((scale * point) - rotation_point, rotation) + rotation_point + translation;
                auto result = TransformPosition(point, transformation);
                Assert::IsTrue(Equal(expected, result, 0.001f));
            }
        }
    };
}
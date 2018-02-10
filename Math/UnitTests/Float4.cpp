#include "CppUnitTest.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>
#include <Math\MathConstants.h>
#include "MathToStringForUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTestFloat4)
	{
	public:
		
		TEST_METHOD(Float4Addition)
		{
            using namespace Math;
            Float4 a(1,2,3,4);
            Float4 b(2,1,2,1);

            auto c = a + b;

            Float4 c_correct(3,3,5,5);

            Assert::AreEqual( c, c_correct );

		}

        TEST_METHOD(Float4Tranformation)
        {
            using namespace Math;

            Float4 a( 1, 1, 1, 1 );

            auto matrix = TranslationToFloat4x4( Float3( 10, -5, 3 ) );

            auto b =  matrix * a;

            Float4 b_correct( 11, -4, 4, 1 );

            Assert::AreEqual( b_correct, b);

            matrix = RotationToFloat4x4( EulerToQuaternion( { Math::c_PI.f, 0.f, 0.f } ) );

            auto c = matrix * a;

            Float4 c_correct( 1, -1, -1, 1 );

            Assert::IsTrue( Equal( c_correct, c, 0.0001f) );

            matrix = RotationToFloat4x4( EulerToQuaternion( { Math::c_PI, Math::c_PI, 0 } ) );

            auto d = matrix * a;

            Float4 d_correct( -1, -1, 1, 1 );

            Assert::IsTrue( Equal( d_correct, d, 0.0001f) );

            matrix = RotationToFloat4x4( EulerToQuaternion( { Math::c_PI, Math::c_PI, Math::c_PI } ) );

            auto e = matrix * a;

            Float4 e_correct( 1, 1, 1, 1 );

            Assert::IsTrue( Equal( e_correct, e, 0.0001f) );


            matrix = ScaleToFloat4x4( Float3( 10, -5, 3 ) );

            auto f =  matrix * a;

            Float4 f_correct( 10, -5, 3, 1 );

            Assert::IsTrue( Equal( f_correct, f, 0.0001f) );

            matrix = AffineTransform( Math::Float3( 9, 9, 9 ), EulerToQuaternion( 0 ), Math::Float3( 10, 10, 10 ) );

            auto g = matrix * a;

            Float4 g_correct( 100, 100, 100, 1);

            Assert::IsTrue( Equal( f_correct, f, 0.0001f) );
        }

	};
}
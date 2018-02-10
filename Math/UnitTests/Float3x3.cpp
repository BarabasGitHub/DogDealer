#include "CppUnitTest.h"

#include <Math\FloatMatrixTypes.h>
#include <Math\FloatMatrixOperators.h>
#include <Math\MathFunctions.h>
#include <Math\MathConstants.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTestFloat3x3)
	{
	public:

        //TEST_METHOD(TestTranformation)
        //{
        //    using namespace Math;

        //    Float4 a( 1, 1, 1, 1 );

        //    auto matrix = Float4x4::FromTranslation( Float3( 10, -5, 3 ) );

        //    auto b =  matrix * a;

        //    Float4 b_correct( 11, -4, 4, 1 );

        //    Assert::IsTrue( ( b_correct == b ).All() );

        //    matrix = Float4x4::FromRotation( Math::Quaternion::FromEulerAngles( Math::c_PI, 0, 0 ) );

        //    auto c = matrix * a;

        //    Float4 c_correct( 1, -1, -1, 1 );

        //    Assert::IsTrue( Equal( c_correct, c, 0.0001f) );

        //    matrix = Float4x4::FromRotation( Math::Quaternion::FromEulerAngles( Math::c_PI, Math::c_PI, 0 ) );

        //    auto d = matrix * a;

        //    Float4 d_correct( -1, -1, 1, 1 );

        //    Assert::IsTrue( Equal( d_correct, d, 0.0001f) );

        //    matrix = Float4x4::FromRotation( Math::Quaternion::FromEulerAngles( Math::c_PI, Math::c_PI, Math::c_PI ) );

        //    auto e = matrix * a;

        //    Float4 e_correct( 1, 1, 1, 1 );

        //    Assert::IsTrue( Equal( e_correct, e, 0.0001f) );


        //    matrix = Float4x4::FromScale( Float3( 10, -5, 3 ) );

        //    auto f =  matrix * a;

        //    Float4 f_correct( 10, -5, 3, 1 );

        //    Assert::IsTrue( Equal( f_correct, f, 0.0001f) );

        //    matrix = Float4x4::AffineTransform( Math::Float3( 9, 9, 9 ), Math::Quaternion::FromEulerAngles( 0 ), Math::Float3( 10, 10, 10 ) );

        //    auto g = matrix * a;

        //    Float4 g_correct( 100, 100, 100, 1);

        //    Assert::IsTrue( Equal( f_correct, f, 0.0001f) );
        //}

        TEST_METHOD(Float3x3Determinant)
        {
            using namespace Math;
            Float3x3 a(
                1.000, 2.000, 3.000,
                2.000, 3.000, 5.000,
                9.000, 6.000, 0.000);

            float a_determinant = 15.000;

            auto result = Determinant(a);

            Assert::AreEqual( result, a_determinant, 0.0001f );
        }

        TEST_METHOD(Float3x3Minor)
        {   
            using namespace Math;
            Float3x3 a(
                1, 2, 3,
                2, 3, 5,
                9, 6, 0);

            Float2x2 minor00( 
                3, 5,
                6, 0);

            auto result = Minor(a, 0,0);

            Assert::IsTrue( Equal( result, minor00, 0.0001f ) );
            
            Float2x2 minor10(
                2, 3,
                6, 0);
            
            result = Minor(a, 1,0);

            Assert::IsTrue( Equal( result, minor10, 0.0001f ) );

            Float2x2 minor12(
                1, 2,
                9, 6);
            
            result = Minor(a, 1,2);

            Assert::IsTrue( Equal( result, minor12, 0.0001f ) );

        }

        TEST_METHOD(Float3x3CoFactor)
        {
            using namespace Math;
            Float3x3 a(
                1.000, 2.000, 3.000,
                2.000, 3.000, 5.000,
                9.000, 6.000, 0.000);
            
            float cofactor00 = -30;
            
            Assert::AreEqual( CoFactor(a, 0,0), cofactor00, 0.0001f );

            float cofactor01 = 45;

            Assert::AreEqual( CoFactor(a, 0,1), cofactor01, 0.0001f );

            float cofactor02 = -15;

            Assert::AreEqual( CoFactor(a, 0,2), cofactor02, 0.0001f );

            float cofactor10 = 18;

            Assert::AreEqual( CoFactor(a, 1,0), cofactor10, 0.0001f );

            float cofactor11 = -27;

            Assert::AreEqual( CoFactor(a, 1,1), cofactor11, 0.0001f );

            float cofactor12 = 12;

            Assert::AreEqual( CoFactor(a, 1,2), cofactor12, 0.0001f );

            float cofactor20 = 1;

            Assert::AreEqual( CoFactor(a, 2,0), cofactor20, 0.0001f );

            float cofactor21 = 1;

            Assert::AreEqual( CoFactor(a, 2,1), cofactor21, 0.0001f );

            float cofactor22 = -1;

            Assert::AreEqual( CoFactor(a, 2,2), cofactor22, 0.0001f );
        }

        
        TEST_METHOD(Float3x3CoFactorMatrix)
        {
            using namespace Math;
            Float3x3 a(
                1.000, 2.000, 3.000,
                2.000, 3.000, 5.000,
                9.000, 6.000, 0.000);

            Float3x3 a_cofactor_matrix(
                  -30,  45, -15,
                   18, -27,  12,
                    1,   1,  -1);

            auto result = CoFactorMatrix(a);

            Assert::IsTrue( Equal( result, a_cofactor_matrix, 0.0001f) );
        }

        TEST_METHOD(Float3x3Adjugate)
        {
            using namespace Math;
            Float3x3 a(
                1.000, 2.000, 3.000,
                2.000, 3.000, 5.000,
                9.000, 6.000, 0.000);

            Float3x3 a_adjugate(
                  -30,  18,   1,
                   45, -27,   1,
                  -15,  12,  -1);

            auto result = Adjugate(a);

            Assert::IsTrue( Equal( result, a_adjugate, 0.0001f) );
        }

        TEST_METHOD(Float3x3Inversion)
        {
            using namespace Math;
            Float3x3 identity = Identity();

            auto result = Inverse(identity);

            Assert::IsTrue( Equal( result, identity, 0.0001f) );

            Float3x3 a(
                1.000, 2.000, 3.000,
                2.000, 3.000, 5.000,
                9.000, 6.000, 0.000);

            Float3x3 a_inverse(
                -2.000000000000000f,  1.200000000000000f,  0.066666666666667f,
                3.000000000000000f, -1.800000000000000f,  0.066666666666667f,
                -1.000000000000000f,  0.800000000000000f, -0.066666666666667f);

            result = Inverse(a);

            Assert::IsTrue( Equal( result, a_inverse, 0.0001f ) );
        }
 	};
}
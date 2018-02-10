
#include "CppUnitTest.h"

#include <Math\FloatMatrixOperators.h>
#include <Math\MathFunctions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTestFloat2x2)
	{
	public:
		
		TEST_METHOD(Float2x2Determinant)
		{
            using namespace Math;
			Float2x2 a( 
                1, 2,
                3, 4);

            float a_determinant = -2;

            auto result = Determinant(a);
            
            Assert::AreEqual( result, a_determinant, 0.0001f );
		}

        
        TEST_METHOD(Float2x2Minor)
        {   
            using namespace Math;
            Float2x2 a( 
                1, 2,
                3, 4);

            Assert::AreEqual( Minor(a, 0,0), a(1,1), 0.0001f );
            Assert::AreEqual( Minor(a, 0,1), a(1,0), 0.0001f );
            Assert::AreEqual( Minor(a, 1,0), a(0,1), 0.0001f );
            Assert::AreEqual( Minor(a, 1,1), a(0,0), 0.0001f );

        }

        TEST_METHOD(Float2x2CoFactor)
        {
            using namespace Math;
            Float2x2 a( 
                1, 2,
                3, 4);

            Assert::AreEqual( CoFactor(a, 0,0),  a(1,1), 0.0001f );
            Assert::AreEqual( CoFactor(a, 0,1), -a(1,0), 0.0001f );
            Assert::AreEqual( CoFactor(a, 1,0), -a(0,1), 0.0001f );
            Assert::AreEqual( CoFactor(a, 1,1),  a(0,0), 0.0001f );
        }

        
        TEST_METHOD(Float2x2CoFactorMatrix)
        {
            using namespace Math;
            Float2x2 a( 
                1, 2,
                3, 4);

            Float2x2 a_cofactor_matrix(
                  4, -3,
                 -2,  1);

            auto result = CoFactorMatrix(a);

            Assert::IsTrue( Equal( result, a_cofactor_matrix, 0.0001f) );
        }

        TEST_METHOD(Float2x2Adjugate)
        {
            using namespace Math;
            Float2x2 a( 
                1, 2,
                3, 4);

            Float2x2 a_adjugate(
                  4, -2,
                 -3,  1);

            auto result = Adjugate(a);

            Assert::IsTrue( Equal( result, a_adjugate, 0.0001f) );
        }

        TEST_METHOD(Float2x2Inverse)
        {
            using namespace Math;
            Float2x2 a( 
                1, 2,
                3, 4);

            Float2x2 a_inverse(
                -2.0,  1.0,
                 1.5, -0.5);

            auto result = Inverse(a);

            Assert::IsTrue( Equal( result, a_inverse, 0.0001f) );
        }
        
	};
}
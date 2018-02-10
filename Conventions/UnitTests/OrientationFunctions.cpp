#include "CppUnitTest.h"

#include <Conventions\OrientationFunctions.h>
#include <Math\TransformFunctions.h>
#include <Math\MathFunctions.h>
#include <Math\UnitTests\ToString.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DogDealerConventionsUnitTests
{		
	TEST_CLASS(OrientationFunctionsUnitTest)
	{
	public:
		
        const float epsilon = 0.000001f;

		TEST_METHOD(ToParentFromLocalTest)
		{
            Orientation identity = Math::Identity();
            Orientation parent = { { 1, 3, 4 }, Math::XAngleToQuaternion( 2 ) };

            Orientation expected = parent;
            Orientation result = ToParentFromLocal( parent, identity );
            Assert::IsTrue( Equal( expected, result, epsilon ) );

            expected = parent;
            result = ToParentFromLocal( identity, parent );
            Assert::IsTrue( Equal( expected, result, epsilon) );

            auto inverse_parent = Orientation{ -InverseRotate( parent.position, parent.rotation ), Conjugate( parent.rotation ) };

            expected = identity;
            result = ToParentFromLocal( parent, inverse_parent );
            Assert::IsTrue( Equal( expected, result, epsilon ) );

            Math::Float3 point = { 2, 3, 5 };
            Orientation local = { { 4, 1, 6 }, Math::ZAngleToQuaternion( -1 ) };

            auto total = ToParentFromLocal( parent, local );

            auto expected_point = Rotate( Rotate( point, local.rotation ) + local.position, parent.rotation ) + parent.position;
            auto result_point = Rotate( point, total.rotation ) + total.position;
            Assert::IsTrue( Equal( expected, result, epsilon ) );
		}


        TEST_METHOD( GetOffsetTest )
        {
            Orientation identity = Math::Identity();
            Orientation a = { { 2, 1, 3 }, Math::XAngleToQuaternion( -2 ) };

            Orientation expected = a;
            Orientation result = GetOffset( a, identity );

            Assert::IsTrue( Equal( expected, result, epsilon ) );


            Orientation default = { { 1, 3, 2 }, Math::YAngleToQuaternion( 1.3f ) };
            Orientation current = { { 5, -2, 1 }, Math::ZAngleToQuaternion( 0.5f ) };
            Orientation offset = GetOffset( current, default );

            Math::Float3 point = { 2, 5, 3 };

            // point relative to 'global'
            {
                auto expected_point = Rotate( point, current.rotation ) + current.position;
                auto result_point = Rotate( Rotate( point, default.rotation ) + default.position, offset.rotation ) + offset.position;
                Assert::IsTrue( Equal(expected_point, result_point, epsilon ) );
            }

            // point in relative to default
            {
                auto expected_point = Rotate( InverseRotate( point - default.position, default.rotation ), current.rotation ) + current.position;
                auto result_point = Rotate( point, offset.rotation ) + offset.position;
                Assert::IsTrue( Equal( expected_point, result_point, epsilon ) );
            }

        }


        TEST_METHOD( TestInvert )
        {
            Orientation a = { { 3, 4, 1 }, Math::ZAngleToQuaternion( -1.5 ) };

            Orientation inverted_a = Invert( a );

            {
                auto result = ToParentFromLocal( a, inverted_a );
                Assert::IsTrue( Equal( result, Math::Identity(), epsilon ) );
            }

            Math::Float3 point = { 2, 3, 7 };

            {
                auto result = Rotate( Rotate( point, a.rotation ) + a.position, inverted_a.rotation ) + inverted_a.position;
                Assert::IsTrue( Equal( point, result, epsilon ) );
            }
            
        }
	};
}
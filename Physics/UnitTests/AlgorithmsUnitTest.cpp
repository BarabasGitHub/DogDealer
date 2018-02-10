#include "CppUnitTest.h"

#include "Conventions\OrientationFunctions.h"
#include "Math\MathFunctions.h"
#include "Math\UnitTests\ToString.h"
#include "Physics\Movement.h"
#include "Physics\Algorithms.h"
#include "Physics\InertiaFunctions.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Physics
{
	TEST_CLASS(AlgorithmsUnitTest)
	{
	public:
		
		TEST_METHOD(TestApplyImpulse)
		{

            Movement input_movement;
            input_movement.momentum = 0;
            input_movement.angular_momentum = 0;

            Math::Float3 input_impulse = 0;
            Math::Float3 position = 0;

            Movement expected_output;
            expected_output.momentum = 0;
            expected_output.angular_momentum = 0;

            auto output = ApplyImpulse( input_movement, input_impulse, position );

            Assert::IsTrue( Equal( output, expected_output, 0.0 ) );

            input_impulse = { 0, 1, 0 };
            position = 0;

            expected_output.momentum = { 0, 1, 0 };
            expected_output.angular_momentum = 0;

            output = ApplyImpulse( input_movement, input_impulse, position );

            Assert::IsTrue( Equal( output, expected_output, 0.0 ) );

            input_impulse = { 0, 1, 0 };
            position = { 0, 0, 1 };

            expected_output.momentum = { 0, 1, 0 };
            // L = r × mv
            expected_output.angular_momentum = { -1, 0, 0 };

            output = ApplyImpulse( input_movement, input_impulse, position );

            Assert::IsTrue( Equal( output, expected_output, 1e-5f ) );
		}


        TEST_METHOD( TestApplyMovement )
        {
            Orientation input_orientation;
            input_orientation.position = 0;
            input_orientation.rotation = { 0, 0, 0, 1 };

            Movement input_movement;
            input_movement.momentum = { 0, 1, 0 };
            input_movement.angular_momentum = { -1, 0, 0 };

            Inertia inverse_inertia = CreateInertia( { 1, 1, 1 }, 1 );

            float time_step = 1;
            Orientation output;
            ApplyMovement( CreateRange(&output, 1), CreateRange(&input_orientation, 1), CreateRange(&input_movement, 1), CreateRange(&inverse_inertia, 1), time_step);

            Orientation expected_output;
            expected_output.position = { 0, 1, 0 };
            expected_output.rotation = Math::EulerToQuaternion( { -1, 0, 0 } );

            Assert::IsTrue( Equal( output, expected_output, 1e-1f ) );
        }

        
        TEST_METHOD( TestCalculateVelocity )
        {
            Movement movement;
            movement.momentum = 0;
            movement.angular_momentum = 0;

            Inertia inverse_inertia = CreateInertia( { 1, 1, 1 }, 1 );

            Math::Float3 relative_position = 0;

            Math::Float3 expected_output = { 0, 0, 0 };

            auto output = CalculateVelocity( movement, inverse_inertia, relative_position );

            Assert::AreEqual( expected_output, output );

            movement.momentum = { 0, 1, 0 };
            movement.angular_momentum = { 1, 1, 1 };

            relative_position = 0;

            expected_output = { 0, 1, 0 };

            output = CalculateVelocity( movement, inverse_inertia, relative_position );

            Assert::AreEqual( expected_output, output );

            movement.momentum = { 0, 1, 0 };
            movement.angular_momentum = { -1, 0, 0 };

            relative_position = { 0, 0, 1 };

            expected_output = { 0, 2, 0 };

            output = CalculateVelocity( movement, inverse_inertia, relative_position );

            Assert::IsTrue( Equal( expected_output, output, 1e-5f ) );

            relative_position = { 0, 1, 0 };

            expected_output = { 0, 1, -1 };

            output = CalculateVelocity( movement, inverse_inertia, relative_position );

            Assert::IsTrue( Equal( expected_output, output, 1e-5f ) );

        }

        TEST_METHOD(TestAngularVelocityFromRotation)
        {
            float time_step = 0.01f;

            Math::Quaternion new_rotation, original_rotation = {Math::Identity()};

            Math::Float3 expected_angular_velocity = {10, 0, 0};

            new_rotation = UpdateQuaternionWithAngularVelocity(original_rotation, expected_angular_velocity, time_step);
            auto calculated_angular_velocity = AngularVelocityFromRotation(new_rotation, original_rotation, time_step);

            Assert::IsTrue(Equal(calculated_angular_velocity, expected_angular_velocity, 1e-2f),
                (L"Expected: " + ToString(expected_angular_velocity) + L" Result: " + ToString(calculated_angular_velocity)).c_str());
        }
	};
}
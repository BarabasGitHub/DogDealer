#pragma warning(disable: 4505)
#include "CppUnitTest.h"

#include <Conventions\OrientationFunctions.h>

#include <Math\MathFunctions.h>
#include <Math\UnitTests\MathToStringForUnitTest.h>
#include <Math\FloatOperators.h>

#include <Physics\ContactFunctions.h>

#include <Utilities\UnitTest\ToString.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace Math;
namespace Microsoft
{
    namespace VisualStudio
    {
        namespace CppUnitTestFramework
        {
            using ::ToString;
        }
    }
}

namespace DogDealerPhysicsUnitTests
{
    TEST_CLASS( ContactFunctionsUnitTest )
    {
    public:

        TEST_METHOD( SutherlandHodgmanTest )
        {
            array<Float3, 4> vertices = { {
                { 1, 1, 0 },
                { -1, 1, 0 },
                { -1, -1, 0 },
                { 1, -1, 0 }
            } };

            array<BoundingShapes::Plane, 4> planes = { {
                    { { 1, 0, 0 }, -.5 },
                    { { 0, 1, 0 }, -.5 },
                    { { -1, 0, 0 }, -.5 },
                    { { 0, -1, 0 }, -.5 }
            } };

            auto clipped_vertices = SutherlandHodgman( planes, vertices );

            vector<Float3> expected_output = { {
                { 0.5, 0.5, 0 },
                { -0.5, 0.5, 0 },
                { -0.5, -0.5, 0 },
                { 0.5, -0.5, 0 }
            } };

            sort( begin( expected_output ), end( expected_output ), LessXYZ );
            sort( begin( clipped_vertices ), end( clipped_vertices ), LessXYZ );

            Assert::AreEqual( expected_output, clipped_vertices );

        }

    };
}
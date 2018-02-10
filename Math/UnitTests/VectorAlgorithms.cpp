#include "CppUnitTest.h"

#include <vector>
#include <algorithm>
#include <numeric>

#include "ToString.h"
#include <Utilities\UnitTest\ToString.h>

#include <Math\VectorAlgorithms.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
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


namespace DogDealerMathUnitTests
{
    TEST_CLASS( VectorAlgorithmUnitTest )
    {
    public:

        TEST_METHOD( ScaleTest )
        {
            std::vector<float> input( 100, 100 );

            float scaling = 0.1f;

            std::vector<float> expected_output( 100, 10 );

            auto output = input;

            Scale( scaling, output );

            Assert::AreEqual( output, expected_output );

            output = input;

            Scale( input, scaling, output );

            Assert::AreEqual( output, expected_output );

            output = input;

            for( auto & v : output )
            {
                v = -v;
            }

            Scale( -scaling, output );

            Assert::AreEqual( expected_output, output );
        }


        TEST_METHOD( AddTest )
        {
            using std::vector;

            const uint32_t size = 100;
            vector<float> input( size, 2.f );
            vector<float> output( size, 1.f );

            Add( input, output );

            vector<float> expected_output( size, 3.f );

            Assert::AreEqual( output, expected_output );

            input = vector<float>( size, 2.f );
            output = vector<float>( size, 1.f );

            Add( input, 0.5f, output );

            expected_output = vector<float>( size, 2.f );

            Assert::AreEqual( output, expected_output );

            input = vector<float>( size, 2.f );
            output = vector<float>( size, 1.f );

            Add( input, output, 0.25f );

            expected_output = vector<float>( size, 2.25f );

            Assert::AreEqual( output, expected_output );

            input = vector<float>( size, 2.f );
            output = vector<float>( size, 1.f );

            Add( input, 0.25f, output, 2.f );

            expected_output = vector<float>( size, 2.5f );

            Assert::AreEqual( output, expected_output );
        }



        TEST_METHOD( MultiplyTest )
        {
            const uint32_t size = 100;
            std::vector<float> input( size,  2.f );
            std::vector<float> output( size, .5f );

            Multiply( input, output );

            std::vector<float> expected_output( size, 1.f );

            Assert::AreEqual( output, expected_output );
        }


    };
}
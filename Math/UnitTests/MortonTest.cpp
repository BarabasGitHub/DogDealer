#include "CppUnitTest.h"

#include "../MortonOrder.h"
#include "ToString.h"
#include <Utilities\UnitTest\ToString.h>

#include <array>
#include <vector>
#include <random>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Math;

namespace DogDealerMathUnitTests
{
	TEST_CLASS(MortonTest)
	{
	public:
		
		TEST_METHOD(TestMorton)
		{
            float step = 1.f / float((1 << 21) - 1);
            std::array<Float3, 7> input = {{
                    {0, 0, 0}, 
                    {step, 0.0f, 0.0f}, 
                    {step * 2, 0.f, 0.0f},
                    {0, step, 0}, 
                    {step, step, 0.0}, 
                    {step, step, step},
                    {1,1,1}
                    }};
            std::array<uint64_t, 7> output;
            MinMax<Float3> minmax;
            ComputeMortonOrder(input, minmax, output);


            Assert::AreEqual((uint64_t(1) << uint64_t(63)) - uint64_t(1), output.back());
            Assert::AreEqual(uint64_t(0u), output[0]);
            Assert::AreEqual(uint64_t(1u), output[1]);
            Assert::AreEqual(uint64_t(8u), output[2]);
            Assert::AreEqual(uint64_t(2u), output[3]);
            Assert::AreEqual(uint64_t(1u | 2u), output[4]);
            Assert::AreEqual(uint64_t(1u | 2u | 4u), output[5]);
		}


        //TEST_METHOD(TestMortonSpeed)
        //{
        //    auto size = 100;
        //    std::vector<Float3> input(size);
        //    std::default_random_engine re;
        //    std::uniform_real_distribution<float> distribution(0.f, 1.f);
        //    for(auto &i : input)
        //    {
        //        i = {distribution(re), distribution(re), distribution(re)};
        //    }
        //    std::vector<uint64_t> output(size);
        //    MinMax<Float3> minmax;
        //    for(size_t i = 0; i < 400000; i++)
        //    {
        //        ComputeMortonOrder(input, minmax, output);
        //    }
        //}
	};
}
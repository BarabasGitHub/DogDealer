#include "CppUnitTest.h"

#include <BoundingShapes\OrientedBoxFunctions.h>
#include <Math\FloatTypes.h>
#include <Math\MathFunctions.h>
#include <array>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace BoundingShapes;

namespace DogDealerBoundingShapesUnitTests
{
	TEST_CLASS(OrientedBoundingBoxUnitTest)
	{
	public:

        TEST_METHOD(TestMethod1)
        {
            std::array<Math::Float3, 8> corners = {{
                    {1.00000000f, 2.22140503f, -0.503365993f},
                    {1.00000000f, -1.53736496f, -1.87144697f},
                    {-1.00000000f, -1.53736496f, -1.87144601f},
                    {-1.00000000f, 2.22140598f, -0.503365993f},
                    {1.00000000f, 1.53736401f, 1.37601900f},
                    {-1.00000000f, 1.53736496f, 1.37601900f},
                    {-1.00000000f, -2.22140503f, 0.00793900061f},
                    {0.999998987f, -2.22140598f, 0.00793800037f},
                    }};

            OrientedBox expected;
            expected.center = {0, 0, -.25f};
            expected.extent = {1, 1, 2};
            expected.rotation = {0.82f, 0, 0, 0.574f};

            auto reconstructed = ReconstructOrientedBoxFromCorners(corners);

            Assert::IsTrue(Equal(expected.center, reconstructed.center, 0.01f));
            Assert::IsTrue(Equal(expected.extent, reconstructed.extent, 0.1f));
            if(Dot(GetAxis(expected.rotation), GetAxis(reconstructed.rotation)) < 0) reconstructed.rotation = -reconstructed.rotation;
            Assert::IsTrue(Equal(expected.rotation.operator Math::Float4(), reconstructed.rotation.operator Math::Float4(), 0.01f));
        }


		TEST_METHOD(TestMethod2)
		{
            std::array<Math::Float3, 8> corners = {{
                {4.07653713f, 3.47882104f, 0.0697500035f },
                {7.94023991f, 3.47882104f, -0.965526998f },
                {7.94023895f, 1.47882104f, -0.965526998f },
                {4.07653618f, 1.47882104f, 0.0697489977f },
                {4.33535624f, 3.47882199f, 1.03567505f },
                {4.33535480f, 1.47882104f, 1.03567505f },
                {8.19905758f, 1.47881997f, 0.000399000011f },
                {8.19906044f, 3.47882009f, 0.000399000011f },
                }};

            OrientedBox expected;
            expected.center = {6.13779736f, 2.47882104f, 0.0350741297f};
            expected.extent = {0.5, 1, 2};
            expected.rotation = {0.608761311f, 0, -0.793353498f, 0};

            auto reconstructed = ReconstructOrientedBoxFromCorners(corners);

            Assert::IsTrue(Equal(expected.center, reconstructed.center, 0.01f));
            Assert::IsTrue(Equal(expected.extent, reconstructed.extent, 0.1f));
            if(Dot(GetAxis(expected.rotation), GetAxis(reconstructed.rotation)) < 0) reconstructed.rotation = -reconstructed.rotation;
            Assert::IsTrue(Equal(Math::Float4(expected.rotation), Math::Float4(reconstructed.rotation), 0.01f));
		}

	};
}
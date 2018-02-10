#include "CppUnitTest.h"

#include <Utilities\VectorHelper.h>

#include <Utilities\UnitTest\ToString.h>

#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft
{
    namespace VisualStudio
    {
        namespace CppUnitTestFramework
        {
            template<typename Type>
            std::wstring ToString( const std::vector<Type> & v )
            {
                return ToString( CreateRange( v ) );
            }
        }
    }
}

namespace DogDealerUtilitiesUnitTest
{
	TEST_CLASS(VectorHelperUnitTest)
	{
	public:

        TEST_METHOD( TestAppend )
        {
            std::vector<uint32_t> data = { 1, 2, 3 };
            std::vector<uint32_t> extra_data = { 4, 5, 6, 7 };

            std::vector<uint32_t> expected_output = { 1, 2, 3, 4, 5, 6, 7 };

            Append( data, extra_data );

            Assert::AreEqual( expected_output, data );
        }


        TEST_METHOD( TestSwapAndPrune )
        {
            std::vector<uint32_t> data = { 1, 2, 3, 4, 5 };

            auto index_to_prune = 2;

            std::vector<uint32_t> expected_output = { 1, 2, 5, 4 };

            SwapAndPrune( index_to_prune, data );

            Assert::AreEqual( expected_output, data );
        }


		TEST_METHOD(TestRemoveEntries)
		{
            std::vector<uint32_t> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

            std::vector<uint32_t> indices = { 0, 3, 4, 6 };

            std::vector<uint32_t> expected_output = { 2, 3, 6, 8, 9, 10 };

            RemoveEntries( data, indices );

            Assert::AreEqual( expected_output, data );
		}


        TEST_METHOD( TestReorder )
        {

            std::vector<uint32_t> reordering = { 3, 0, 2, 4, 1 };
            std::vector<uint32_t> test = { 0, 1, 2, 3, 4 };
            std::vector<uint32_t> test_expected = reordering;
            std::vector<uint32_t> test_result( test.size() );
            Reorder<uint32_t>( test, reordering, test_result );
            Assert::AreEqual( test_result, test_expected );
        }


        TEST_METHOD( TestReorder2 )
        {

            std::vector<uint32_t> reordering = { 2, 1, 3, 0 };
            std::vector<uint32_t> data = { 0, 1, 3, 5 };
            std::vector<uint32_t> expected = { 3, 1, 5, 0 };
            std::vector<uint32_t> result( data.size() );
            Reorder<uint32_t>( data, reordering, result );
            Assert::AreEqual( expected, result );
        }

        TEST_METHOD( TestReorderInverseReorder )
        {

            std::vector<uint32_t> reordering = { 3, 0, 2, 4, 1 };
            std::vector<uint32_t> data = { 4, 2, 1, 3, 7, };
            std::vector<uint32_t> result( data.size() );
            std::vector<uint32_t> temp( data.size() );
            Reorder<uint32_t>(data, reordering, temp);
            InverseReorder<uint32_t>(temp, reordering, result);
            Assert::AreEqual( data, result );
         }


        TEST_METHOD( TestInverseReorder3 )
        {

            std::vector<uint32_t> reordering = { 3, 0, 2, 4, 1 };
            std::vector<uint32_t> data = { 4, 2, 1, 3, 7, };
            std::vector<uint32_t> expected = { 2, 7, 1, 4, 3 };
            std::vector<uint32_t> result( data.size() );
            InverseReorder<uint32_t>( data, reordering, result );
            Assert::AreEqual( expected, result );
        }

        TEST_METHOD( TestInverseReorder2 )
        {

            std::vector<uint32_t> reordering = { 2, 1, 3, 0 };
            std::vector<uint32_t> data = { 0, 1, 3, 5 };
            std::vector<uint32_t> expected = { 5, 1, 0, 3 };
            std::vector<uint32_t> result( data.size() );
            InverseReorder<uint32_t>( data, reordering, result );
            Assert::AreEqual( expected, result );
        }


        TEST_METHOD( TestInvertReordering )
        {
            std::vector<uint32_t> reordering = { 3, 0, 2, 4, 1 };
            std::vector<uint32_t> data = { 0, 1, 2, 3, 4 };
            std::vector<uint32_t> expected(reordering.size());
            InverseReorder<uint32_t>( data, reordering, expected );

            std::vector<uint32_t> result( reordering.size() );
            InvertReordering( reordering, result );
            Assert::AreEqual( expected, result );
        }

	};
}
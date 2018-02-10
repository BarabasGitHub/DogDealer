#include "CppUnitTest.h"

#include <vector>
#include <algorithm>
#include <numeric>

#include "ToString.h"

#include <Math\SparseMatrix.h>
#include <Math\SparseMatrixAlgorithms.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Math;

namespace DogDealerMathUnitTests
{
	TEST_CLASS(SparseMatrixUnitTest)
	{
	public:

            TEST_METHOD(SparseMatrixRemoveZeroEntries)
            {
                  // 0, 0, 1, 0, 0,
                  // 0, 2, 0, 3, 0,
                  // 4, 0, 0, 5, 0,
                  // 0, 6, 0, 0, 7,

                  SparseMatrix matrix;
                  matrix.column_count = 5;
                  matrix.column_indices = { 1, 2, 4, 1, 2, 3, 4, 0, 1, 3, 0, 1, 2, 4 };
                  matrix.row_offsets = { 0, 3, 7, 10, 14 };
                  matrix.values = { 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7 };


                  // 0, 0, 4, 0,
                  // 0, 2, 0, 6,
                  // 1, 0, 0, 0,
                  // 0, 3, 5, 0,
                  // 0, 0, 0, 7,

                  SparseMatrix expected;
                  expected.column_count = 5;
                  expected.column_indices = { 2, 1, 3, 0, 3, 1, 4 };
                  expected.row_offsets = { 0, 1, 3, 5, 7 };
                  expected.values = { 1, 2, 3, 4, 5, 6, 7 };

                  RemoveZeroEntries( matrix );

                  Assert::IsTrue( expected == matrix );
            }


		TEST_METHOD(SparseMatrixTranspose)
		{
            // 0, 0, 1, 0, 0,
            // 0, 2, 0, 3, 0,
            // 4, 0, 0, 5, 0,
            // 0, 6, 0, 0, 7,

            SparseMatrix matrix;
            matrix.column_count = 5;
            matrix.column_indices = { 2, 1, 3, 0, 3, 1, 4 };
            matrix.row_offsets = { 0, 1, 3, 5, 7 };
            matrix.values = { 1, 2, 3, 4, 5, 6, 7 };


            // 0, 0, 4, 0,
            // 0, 2, 0, 6,
            // 1, 0, 0, 0,
            // 0, 3, 5, 0,
            // 0, 0, 0, 7,

            SparseMatrix expected;
            expected.column_count = 4;
            expected.column_indices = { 2, 1, 3, 0, 1, 2, 3 };
            expected.row_offsets = { 0, 1, 3, 4, 6, 7 };
            expected.values = { 4, 2, 6, 1, 3, 5, 7 };

            SparseMatrix result;
            Transpose( matrix, result );

            Assert::IsTrue( expected == result );
		}


        TEST_METHOD( SparseMatrixTransposeWithUnusedElements )
        {
            // 0, 0, 1, 0, 0,
            // 0, 2, 0, 3, 0,
            // 4, 0, 0, 5, 0,
            // 0, 6, 0, 0, 7,

            SparseMatrix matrix;
            matrix.column_count = 5;
            matrix.column_indices = { 2, 1, 3, 0, 3, 1, 4 };
            matrix.row_offsets = { 0, 1, 3, 5, 7 };
            matrix.values = { 1, 2, 3, 4, 5, 6, 7 };

            // give it twice as many elements, all the duplicates aren't used and thus shouldn't affect the result
            matrix.column_indices.resize( matrix.column_indices.size() * 2 );
            matrix.values.resize( matrix.values.size() * 2 );

            // 0, 0, 4, 0,
            // 0, 2, 0, 6,
            // 1, 0, 0, 0,
            // 0, 3, 5, 0,
            // 0, 0, 0, 7,

            SparseMatrix expected;
            expected.column_count = 4;
            expected.column_indices = { 2, 1, 3, 0, 1, 2, 3 };
            expected.row_offsets = { 0, 1, 3, 4, 6, 7 };
            expected.values = { 4, 2, 6, 1, 3, 5, 7 };

            SparseMatrix result;
            Transpose( matrix, result );

            Assert::IsTrue( expected == result );
        }


        TEST_METHOD( SparseMatrixMultiplication )
        {
            // 0, 0, 1, 0, 0,
            // 0, 2, 0, 3, 0,
            // 4, 0, 0, 5, 0,
            // 0, 6, 0, 0, 7,

            SparseMatrix matrix_a;
            matrix_a.column_count = 5;
            matrix_a.column_indices = { 2, 1, 3, 0, 3, 1, 4 };
            matrix_a.row_offsets = { 0, 1, 3, 5, 7 };
            matrix_a.values = { 1, 2, 3, 4, 5, 6, 7 };


            // 0, 0, 1,
            // 0, 2, 3,
            // 4, 0, 0,
            // 0, 5, 6,
            // 7, 0, 0,

            SparseMatrix matrix_b;
            matrix_b.column_count = 3;
            matrix_b.column_indices = { 2, 1, 2, 0, 1, 2, 0 };
            matrix_b.row_offsets = { 0, 1, 3, 4, 6, 7 };
            matrix_b.values = { 1, 2, 3, 4, 5, 6, 7 };

            // 4,   0,  0,
            // 0,  19, 24,
            // 0,  25, 34,
            // 49, 12, 18

            SparseMatrix expected;
            expected.column_count = 3;
            expected.column_indices = { 0, 1, 2, 1, 2, 0, 1, 2 };
            expected.row_offsets = { 0, 1, 3, 5, 8 };
            expected.values = { 4, 19, 24, 25, 34, 49, 12, 18 };


            SparseMatrix result;
            Multiply(matrix_a, matrix_b, result);

            // do a double transpose to make the result ordered
            SparseMatrix transposed_result;
            Transpose( result, transposed_result );
            Transpose( transposed_result, result );

            Assert::AreEqual( expected, result );

        }



        TEST_METHOD( SparseMatrixInverseReorderColumns )
        {
            // 0, 0, 1, 0, 0,
            // 0, 2, 0, 3, 0,
            // 4, 0, 0, 5, 0,
            // 0, 6, 0, 0, 7,

            SparseMatrix original;
            original.column_count = 5;
            original.column_indices = { 2, 1, 3, 0, 3, 1, 4 };
            original.row_offsets = { 0, 1, 3, 5, 7 };
            original.values = { 1, 2, 3, 4, 5, 6, 7 };

            std::vector<uint32_t> reordering = { 3, 0, 2, 4, 1 };

            //  0, 0, 1, 0, 0,
            //  2, 0, 0, 0, 3,
            //  0, 0, 0, 4, 5,
            //  6, 7, 0, 0, 0,

            SparseMatrix expected;
            expected.column_count = 5;
            expected.column_indices = { 2, 0, 4, 3, 4, 0, 1 };
            expected.row_offsets = { 0, 1, 3, 5, 7 };
            expected.values = { 1, 2, 3, 4, 5, 6, 7 };

            auto result = original;
            InverseReorderColumns(reordering, result);

            // sort
            SparseMatrix temp;
            Transpose(result, temp);
            Transpose(temp, result);

            Assert::AreEqual(expected, result);
        }


        TEST_METHOD( SparseMatrixInverseReorderSquare )
        {
            // 0, 0, 1, 0,
            // 0, 2, 0, 3,
            // 4, 0, 0, 5,
            // 0, 6, 0, 0,

            SparseMatrix original;
            original.column_count = 4;
            original.column_indices = { 2, 1, 3, 0, 3, 1 };
            original.row_offsets = { 0, 1, 3, 5, 6 };
            original.values = { 1, 2, 3, 4, 5, 6 };

            std::vector<uint32_t> reordering = {3, 0, 2, 1};

            // 2, 3, 0, 0,
            // 6, 0, 0, 0,
            // 0, 5, 0, 4,
            // 0, 0, 1, 0,
            SparseMatrix expected;
            expected.column_count = 4;
            expected.column_indices = { 0, 1, 0, 1, 3, 2 };
            expected.row_offsets = { 0, 2, 3, 5, 6 };
            expected.values = { 2, 3, 6, 5, 4, 1 };

            SparseMatrix result = original;
            SparseMatrix temp;
            InverseReorderSquare(reordering, result, temp);

            Transpose(result, temp);
            Transpose(temp, result);

            Assert::AreEqual( expected, result );
        }
	};
}
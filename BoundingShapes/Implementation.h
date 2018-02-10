#pragma once

#include <Math\FloatTypes.h>
#include <Math\SSETypes.h>


#include <array>
namespace BoundingShapes
{
    namespace Details
    {
        std::array<Math::Float3, 8> const box_corner_factors{ {
            { -1, -1,  1 },
            {  1, -1,  1 },
            {  1,  1,  1 },
            { -1,  1,  1 },
            { -1, -1, -1 },
            {  1, -1, -1 },
            {  1,  1, -1 },
            { -1,  1, -1 },
        } };

        std::array<Math::SSE::Float32Vector, 8> const box_corner_factors_sse{ {
                { { -1, -1, 1, 0 } },
                { { 1, -1, 1, 0 } },
                { { 1, 1, 1, 0 } },
                { { -1, 1, 1, 0 } },
                { { -1, -1, -1, 0 } },
                { { 1, -1, -1, 0 } },
                { { 1, 1, -1, 0 } },
                { { -1, 1, -1, 0 } },
                } };

        
        // Be careful about changing this, some algorithms rely on the order
        std::array<Math::Float3, 6> const axis_aligned_face_normals{ {
            {  1, 0, 0 }, { 0,  1, 0 }, { 0, 0,  1 },
            { -1, 0, 0 }, { 0, -1, 0 }, { 0, 0, -1 }
        } };


        std::array<std::array<Math::Float3, 4> const, 6> const axis_aligned_face_corner_factors{ {
            { {
                {  1,  1,  1 },
                {  1,  1, -1 },
                {  1, -1, -1 },
                {  1, -1,  1 }
            } },
            { {
                {  1,  1,  1 },
                {  1,  1, -1 },
                { -1,  1, -1 },
                { -1,  1,  1 }
            } },
            { {
                {  1,  1, 1 },
                {  1, -1, 1 },
                { -1, -1, 1 },
                { -1,  1, 1 }
            } },
            { {
                { -1,  1,  1 },
                { -1,  1, -1 },
                { -1, -1, -1 },
                { -1, -1,  1 }
            } },
            { {
                {  1, -1,  1 },
                {  1, -1, -1 },
                { -1, -1, -1 },
                { -1, -1,  1 }
            } },
            { {
                {  1,  1, -1 },
                {  1, -1, -1 },
                { -1, -1, -1 },
                { -1,  1, -1 }
            } } 
        } };
    }
}

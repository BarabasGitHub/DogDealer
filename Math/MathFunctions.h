#pragma once

#include "ForwardDeclarations.h"
#include "FloatTypes.h"
#include "MathConstants.h"

#include <cstdint>
#include <type_traits>

// function declarations
namespace Math
{
    template<typename Type> bool Equal( Type const & first, Type const & second );
    template<typename Type, typename ElementComparatorType> bool Equal( Type const & first, Type const & second, ElementComparatorType const & element_comparator);
    template<typename Type> bool Equal( Type const & first, Type const & second, float const tolerance );

    uint8_t GetMaxElementIndex( const Float3 & );
    uint8_t GetMinElementIndex( const Float3 & );

    float GetMaxElement( const Float3 & );
    float GetMinElement( const Float3 & );

    template<typename Type> Type cot( Type input );

    float Dot( Float4 const & a, Float4 const & b );
    float Dot( Float3 const & a, Float3 const & b );
    float Dot( Float2 const & a, Float2 const & b );
    float Dot( Quaternion const & first, Quaternion const & second );
    uint32_t Dot( Unsigned3 const & a, Unsigned3 const & b );

    template<typename Type>
    auto Sum(Type const & input);
    template<typename Type>
    auto Product(Type const & input);

    template<typename Type> Type Sqrt( Type const & input );

    template<typename Type> Type Pow( Type const & input, Type const & power);
    Quaternion Pow( Quaternion const & q, float power);

    Quaternion Exp( Quaternion const & q);
    Quaternion Log( Quaternion const & q);

    template<typename Type> Type Abs( Type const & input );

    template<typename Type> Type AbsoluteDifference( Type a, Type b);

    template<typename Type> Type Max( Type const & first, Type const & second );

    template<typename Type> Type Min( Type const & first, Type const & second );

    template<typename Type> Type Ceil( Type const & first );

    template<typename Type> Type Floor( Type const & first );

    template<typename Type> Type Round( Type const & first );

    template<typename Type> Type Clamp( const Type min, const Type max, const Type input );

    template<typename Type> Type CopySign( Type const & magnitude, Type const & sign );

    template<typename Type> Type Sign( Type const & input );

    // returns number - denominator * floor(number / denominator)
    template<typename Type> Type Modulo( Type number, Type const & denominator );

    // Compares two values, returning 0 or 1 based on which value is greater. Compares a <= b.
    float Step( float a, float b );
    Float2 Step( Float2 a, Float2 b );
    Float3 Step( Float3 a, Float3 b );
    Float4 Step( Float4 a, Float4 b );

    // mathematical length
    template<typename Type> float Norm( const Type input );
    // squared length, same as Dot(a, a)
    template<typename Type> float SquaredNorm( const Type first );
    // distance between a and b
    template<typename Type> float Distance(const Type a, const Type b);
    // squared distance between a and b
    template<typename Type> float SquaredDistance(const Type a, const Type b);
    // normalize such that the length equals 1
    template<typename Type> Type Normalize( const Type first );
    // normalize such that the length equals 1, unless the norm is zero, then return a zero vector.
    template<typename Type> Type ConditionalNormalize( const Type first );
    // linearly interpolate between the first and second argument
    template<typename Type> Type Lerp( Type first, Type second, float blend_factor );
    // linearly interpolate between the first and second argument and normalize
    template<typename Type> Type Nlerp( Type first, Type second, float blend_factor );

    float RadiansToDegrees( const float radians );
    float DegreesToRadians( const float degrees );

    // crossproduct between two vectors
    Float3 Cross( const Float3 first, const Float3 second );
    // fake crossproduct for 2d, gives the signed area
    float Cross( const Float2 first, const Float2 second );
    // area of two 2d vectors
    float Area( const Float2 first, const Float2 second );
    // adds angles keeping them between in the range [-pi, pi)
    float AddAngles( const float first, const float second );
    Float3 AddAngles( const Float3& first, const Float3& second );
    // convert euler angles to a unit quaternion in the order z, x, y
    Quaternion EulerToQuaternion( const Float3& input );
    // rotate a vector by a given unit quaternion
    Float3 Rotate( const Float3 vector, const Quaternion rotation );
    // rotate a vector by the inverse of a given unit quaternion
    Float3 InverseRotate( const Float3 vector, const Quaternion rotation );
    // concatenates two rotations normalizing the results (multiplication order is: second * first)
    Quaternion Concatenate( const Quaternion first, const Quaternion second );
    // spherical interpolation between two unit quaternions
    Quaternion Slerp( const Quaternion first, const Quaternion second, const float factor );

    // Transform the Float2 as if it were Float3( Float2, 1 )
    Float2 TransformPosition( Float2 const position, Float3x3 const & transform );
    // Transform the Float3 as if it were Float4( Float3, 1 )
    Float3 TransformPosition( Float3 const position, Float4x4 const & transform );
    // Transform the Float3 as if it were Float4( Float3, 0 )
    Float3 TransformNormal( Float3 const normal, Float4x4 const & transform );


    // restrain rotation such that the angle is a maximum of max_angle
    //Quaternion ConstrainRotationComponent( const Quaternion rotation, const float max_angle );
    // get component parallel to a normal
    Float3 ComponentParallelToNormal( const Float3 vector, const Float3 normal );
    // get component perpendicular to a normal
    Float3 ComponentPerpendicularToNormal( const Float3 vector, const Float3 normal );
    // get component parallel to an axis
    Float3 ComponentParallelToAxis( const Float3 vector, const Float3 axis );
    // get component perpendicular to an axis
    Float3 ComponentPerpendicularToAxis( const Float3 vector, const Float3 axis );
    // fails to produce a correct result when first = -second
    Quaternion RotationBetweenNormals( const Float3 first, const Float3 second );
    // special case solution
    float ZRotationBetweenAxes(const Float3 first, const Float3 second);
    // fails to produce a correct result when they have opposite directions
    Quaternion RotationBetweenAxes( const Float3 first, const Float3 second );
    // fails to produce a correct result when the rotation is 180° around any axis.
    void DecomposeInSimpleAndTwistRotation( Quaternion& simple, Quaternion& twist, const Quaternion rotation, const Float3 axis );
    float AngleAroundNormal(Quaternion const & rotation, Math::Float3 normal);
    float ZAngle(Quaternion const & rotation);

    bool Even(uint32_t);
    bool Odd(uint32_t);

    // sorting functions
    // sorts first on x and then on y
    bool LessXY( Float2 first, Float2 second );
    // sorts first on x, then on y and then on z
    bool LessXYZ( Float3 first, Float3 second );
    // sorts first on x, then on y, then on z, and then on w
    bool LessXYZW( Float4 first, Float4 second );
}

#include "MathFunctions.inl"
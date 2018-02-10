#include "MathFunctions.h"
#include "FloatTypes.h"
#include "FloatMatrixTypes.h"
#include "IntegerTypes.h"
#include "FloatOperators.h"
#include "FloatMatrixOperators.h"
#include "SSEMathConversions.h"
#include "SSEQuaternionFunctions.h"

#include <DirectXMath.h>

#include <algorithm>

namespace Math
{
    template<> float Clamp<float>( const float min, const float max, const float input )
    {
        using namespace SSE;
        return GetSingle(ClampSingle(SetSingle(min), SetSingle(max), SetSingle(input)));
    }


    float Step( float a, float b )
    {
        return a <= b;
    }


    Float2 Step( Float2 a, Float2 b )
    {
        return{ Step( a.x, b.x ), Step( a.y, b.y ) };
    }


    Float3 Step( Float3 a, Float3 b )
    {
        return{ Step( a.x, b.x ), Step( a.y, b.y ), Step( a.z, b.z ) };
    }


    Float4 Step( Float4 a, Float4 b )
    {
        return{ Step( a.x, b.x ), Step( a.y, b.y ), Step( a.z, b.z ), Step( a.w, b.w ) };
    }


    float Cross( const Float2 first, const Float2 second )
    {
        return Determinant( Float2x2{ first, second } );
    }


    float Area( const Float2 first, const Float2 second )
    {
        return Abs( Cross( first, second ) );
    }


    float RadiansToDegrees( const float radians )
    {
        return radians * 180.0f / c_PI.f;
    }


    float DegreesToRadians( const float degrees )
    {
        return degrees * c_PI.f / 180.0f;
    }

    float Dot( Float4 const & a, Float4 const & b )
    {
        using namespace SSE;
        return GetSingle( Dot4Single( SSEFromFloat4( a ), SSEFromFloat4( b ) ) );
        //return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }


    float Dot( Float3 const & a, Float3 const & b )
    {
        using namespace SSE;
        return GetSingle( Dot3Single( SSEFromFloat3( a ), SSEFromFloat3( b ) ) );
        //return a.x * b.x + a.y * b.y + a.z * b.z;
    }


    float Dot( Float2 const & a, Float2 const & b )
    {
        return a.x * b.x + a.y * b.y;
    }


    float Dot( Quaternion const & first, Quaternion const & second )
    {
        using namespace SSE;
        return GetSingle( Dot4Single( SSEFromQuaternion( first ), SSEFromQuaternion( second ) ) );
        //return first.x * second.x + first.y * second.y + first.z * second.z + first.w * second.w;
    }


    uint32_t Dot( Unsigned3 const & a, Unsigned3 const & b )
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Float3 Cross( const Float3 first, const Float3 second )
    {
        return Float3(
            first.y * second.z - first.z * second.y,
            first.z * second.x - first.x * second.z,
            first.x * second.y - first.y * second.x );
    }


    Float3 Rotate( Float3 const v, const Quaternion r )
    {
        // // v + 2(r.w * Cross(r.xyz, v) + Cross( r.xyz, Cross(r.xyz, v)))
        // auto const cross = Cross( GetAxis( r ), v );
        // auto const temp = r.w * cross + Cross( GetAxis( r ), cross );
        // return v + 2 * temp;
        using namespace SSE;
        return Float3FromSSE(QuaternionRotate3DVector(SSEFromFloat3(v), SSEFromQuaternion(r)));
    }


    Float3 InverseRotate( const Float3 vector, const Quaternion rotation )
    {
        return Rotate( vector, Conjugate( rotation ) );
    }


    uint8_t GetMaxElementIndex(Float3 const & input)
    {
        auto const pred = [&input]( uint8_t a, uint8_t b )
        {
            return input[a] < input[b];
        };
        return std::max(std::max( uint8_t(0), uint8_t(1), pred ), uint8_t(2), pred);
    }


    uint8_t GetMinElementIndex( Float3 const & input )
    {
        auto const pred = [&input]( uint8_t a, uint8_t b )
        {
            return input[a] < input[b];
        };
        return std::min( std::min( uint8_t( 0 ), uint8_t( 1 ), pred ), uint8_t( 2 ), pred );
    }


    float GetMaxElement( const Float3 & a)
    {
        return a[GetMaxElementIndex(a)];
    }


    float GetMinElement( const Float3 & a)
    {
        return a[GetMinElementIndex(a)];
    }


    Float4x4 Lerp( Float4x4 first, Float4x4 second, float blend_factor )
    {
        using namespace SSE;
        return Float4x4FromSSE( Lerp( SSEFromFloat4x4( first ), SSEFromFloat4x4( second ), SetAll( blend_factor ) ) );
    }


    Quaternion Normalize( Quaternion const & first )
    {
        using namespace SSE;
        return QuaternionFromSSE( Normalize4( SSEFromQuaternion( first ) ) );
    }


    /// includes a normalize
    Quaternion Concatenate( const Quaternion first, const Quaternion second )
    {
        return Normalize( second * first );
    }


    Quaternion Nlerp( Quaternion const & first, Quaternion const & second, float blend_factor )
    {
        using namespace SSE;
        return QuaternionFromSSE( Nlerp4( SSEFromQuaternion( first ), SSEFromQuaternion( second ), SetAll( blend_factor ) ) );
    }


    Quaternion Slerp( const Quaternion first, const Quaternion second, const float blend_factor )
    {
        //// if these are true slerp does divide by zero... not sure what to do with it
        //if( Equal( first, second, 0.01f ) ) return first;
        //if( Equal( first, -second, 0.01f ) ) return first;
        auto first_vector = SSE::SSEFromQuaternion( first );
        auto second_vector = SSE::SSEFromQuaternion( second );
        if(SSE::AnySignBitsSet(SSE::Dot4Single( first_vector, second_vector ))) second_vector = SSE::Negate( second_vector );
        auto slerped = DirectX::XMQuaternionSlerp( first_vector, second_vector, blend_factor );
        return SSE::QuaternionFromSSE( slerped );
    }


    Quaternion EulerToQuaternion( const Float3& input )
    {
        return SSE::QuaternionFromSSE( DirectX::XMQuaternionRotationRollPitchYawFromVector( SSE::SSEFromFloat3( input ) ) );
    }


    Float3 AddAngles( const Float3& first, const Float3& second )
    {
        auto last_vector = DirectX::XMVectorAddAngles( SSE::SSEFromFloat3( first ), SSE::SSEFromFloat3( second ) );
        return SSE::Float3FromSSE( last_vector );
    }


    float AddAngles( const float first, const float second )
    {
        return std::remainder(first + second, c_2PI.f);
    }


    Float2 TransformPosition( Float2 const position, Float3x3 const & transform )
    {
        auto v = transform * Math::Float3( position.x, position.y, 1.f );
        return Float2(v.x, v.y);
    }


    Float3 TransformPosition( Float3 const position, Float4x4 const & transform )
    {
        auto v = transform * Math::Float4( position.x, position.y, position.z, 1.f );
        return Float3(v.x, v.y, v.z);
    }


    Float3 TransformNormal( Float3 const normal, Float4x4 const & transform )
    {
        auto v = transform * Math::Float4( normal.x, normal.y, normal.z, 0.f );
        return Float3(v.x, v.y, v.z);
    }


    Float3 ComponentParallelToNormal( const Float3 vector, const Float3 normal )
    {
        return Dot( vector, normal ) * normal;
    }


    Float3 ComponentPerpendicularToNormal( const Float3 vector, const Float3 normal )
    {
        return vector - ComponentParallelToNormal( vector, normal );
    }


    Float3 ComponentParallelToAxis( const Float3 vector, const Float3 axis )
    {
        return ComponentParallelToNormal( vector, Normalize( axis ) );
    }


    Float3 ComponentPerpendicularToAxis( const Float3 vector, const Float3 axis )
    {
        return ComponentPerpendicularToNormal( vector, Normalize( axis ) );
    }


    Quaternion RotationBetweenAxes( const Float3 a, const Float3 b )
    {
        Quaternion result(Cross( a, b ), Sqrt(SquaredNorm(a) * SquaredNorm(b)) + Dot( a, b ));
        return Normalize(result);
    }


    float ZRotationBetweenAxes(Float3 first, Float3 second)
    {
        // set z components to 0, so we always have a rotation around the z-axis
        first.z = 0.0f;
        second.z = 0.0f;
        // normalize the axes
        first = Normalize( first );
        second = Normalize( second );

        // Treat case with 180 degree rotation
        if (first + second == Math::Float3(0.0f, 0.0f, 0.0f)) return Math::c_PI;

        auto rotation = Math::RotationBetweenNormals(first, second);

        // The rotation must either be on the x-y plane or the Identity,
        // so that no decomposition should be necessary
        auto z_angle = GetAngle(rotation);

        // check if the rotation axis is pointing down, in which case we have to flip the angle
        // we can only check z here, because we already know it will be around some z-axis
        if (rotation.z < 0) z_angle = -z_angle;

        return z_angle;
    }


    Quaternion RotationBetweenNormals( const Float3 a, const Float3 b )
    {
        // Quaternion result(Cross( a, b ), 1 + Dot( a, b ));
        // return Normalize(result);
        auto const h = Normalize( a + b );
        return{ Cross( a, h ), Dot( a, h ) };
    }


    // fails to produce a correct result when the rotation is 180° around any axis. 06-10-2014: Is this still true?
    void DecomposeInSimpleAndTwistRotation( Quaternion& simple, Quaternion& twist, const Quaternion rotation, const Float3 axis )
    {
        auto normal = Normalize( axis );
        auto b = Rotate( normal, rotation );
        if(!Equal(normal + b, Float3(0)))
        {
            simple = RotationBetweenNormals(normal, b);
            twist = rotation * Conjugate(simple);
        }
        else
        {
            simple = rotation;
            twist = Identity();
        }

        auto qaxis = GetAxis(rotation);
        auto proj = ComponentParallelToAxis(qaxis, axis);
        auto w = Equal(proj, Float3(0)) ? 1 : rotation.w;
        twist = Normalize(Quaternion(proj, w));
        simple = rotation * Conjugate(twist);
    }


    float AngleAroundNormal(Quaternion const & rotation, Math::Float3 normal)
    {
        Quaternion simple;
        Quaternion twist;
        DecomposeInSimpleAndTwistRotation(simple, twist, rotation, normal);

        auto twist_angle = GetAngle(twist);
        twist_angle = CopySign(twist_angle, Dot(GetAxis(twist), normal));
        twist_angle = std::remainder(twist_angle, c_2PI.f);
        return twist_angle;
    }


    float ZAngle(Quaternion const & rotation)
    {
        Quaternion simple;
        Quaternion twist;
        auto axis = Float3(0, 0, 1);
        DecomposeInSimpleAndTwistRotation(simple, twist, rotation, axis);

        // Only consider offset on Z
        auto twist_angle = GetAngle(twist);
        twist_angle = CopySign(twist_angle, twist.z);
        twist_angle = std::remainder(twist_angle, c_2PI.f);
        return twist_angle;
    }


    bool Even(uint32_t i)
    {
        return !Odd(i);
    }


    bool Odd(uint32_t i)
    {
        return i & 1;
    }

    /// sorting functions
    /// sorts first on x and then on y
    bool LessXY( Float2 first, Float2 second )
    {
        if( first.x == second.x )
        {
            return first.y < second.y;
        }
        return first.x < second.x;
    }


    /// sorts first on x, then on y and then on z
    bool LessXYZ( Float3 first, Float3 second )
    {
        if( first.x == second.x )
        {
            if( first.y == second.y )
            {
                return first.z < second.z;
            }
            return first.y < second.y;
        }
        return first.x < second.x;
    }


    /// sorts first on x, then on y, then on z, and then on w
    bool LessXYZW( Float4 first, Float4 second )
    {
        if( first.x == second.x )
        {
            if( first.y == second.y )
            {
                if( first.z == second.z )
                {
                    return first.w < second.w;
                }
                return first.z < second.z;
            }
            return first.y < second.y;
        }
        return first.x < second.x;
    }


    template<> float Max<float>(float const& first, float const& second)
    {
        using namespace SSE;
        return GetSingle( SSE::MaxSingle( SetSingle( first ), SetSingle(second) ) );
    }


    template<> Float3 Max<Float3>(Float3 const & first, Float3 const & second)
    {
        using namespace SSE;
        return SSE::Float3FromSSE( SSE::Max( SSE::SSEFromFloat3(first), SSE::SSEFromFloat3( second ) ) );
    }


    template<> Float4 Max<Float4>(Float4 const & first, Float4 const & second)
    {
        using namespace SSE;
        return SSE::Float4FromSSE( SSE::Max( SSE::SSEFromFloat4(first), SSE::SSEFromFloat4( second ) ) );
    }


    float Min(float const first, float const second)
    {
        using namespace SSE;
        return GetSingle( SSE::MinSingle( SetSingle( first ), SetSingle(second) ) );
    }


    Float3 Min(Float3 const & first, Float3 const & second)
    {
        using namespace SSE;
        return SSE::Float3FromSSE( SSE::Min( SSE::SSEFromFloat3(first), SSE::SSEFromFloat3( second ) ) );
    }


    Float4 Min(Float4 const & first, Float4 const & second)
    {
        using namespace SSE;
        return SSE::Float4FromSSE( SSE::Min( SSE::SSEFromFloat4(first), SSE::SSEFromFloat4( second ) ) );
    }


    Quaternion Exp(Quaternion const & q)
    {
        auto axis_norm = Norm(GetAxis(q));
        auto sin_axis_norm = std::sin(axis_norm);
        auto cos_axis_norm = std::cos(axis_norm);
        auto exp_w = std::exp(q.w);
        auto axis = GetAxis(q) * sin_axis_norm / axis_norm;
        auto result = Quaternion{axis, cos_axis_norm};
        return result * exp_w;
    }


    Quaternion Log(Quaternion const & q)
    {
        auto squared_axis_norm = SquaredNorm(GetAxis(q));
        auto axis_norm = Sqrt(squared_axis_norm);
        auto norm = Sqrt(squared_axis_norm + q.w * q.w);
        auto w = std::log(norm);
        //auto axis = GetAxis(q) * std::acos(q.w/norm) / axis_norm;
        //auto axis = GetAxis(q) * std::atan2(axis_norm / norm, q.w / norm) / axis_norm;
        auto axis = GetAxis(q) * std::atan2(axis_norm, q.w) / axis_norm;
        return {axis, w};
    }


    Quaternion Pow( Quaternion const & q, float power)
    {
        return Exp(power * Log(q));
    }


    float Abs( float const first )
    {
        using namespace SSE;
        return GetSingle( SSE::AbsSingle( SetSingle( first ) ) );
    }


    Float3 Abs( Float3 const & m )
    {
        return SSE::Float3FromSSE( SSE::Abs( SSE::SSEFromFloat3( m ) ) );
    }


    Float4 Abs( Float4 const & m )
    {
        return SSE::Float4FromSSE( SSE::Abs( SSE::SSEFromFloat4( m ) ) );
    }


    Float3x3 Abs( Float3x3 const & m )
    {
        using namespace SSE;
        float const * __restrict data = begin( m[0] );
        auto t0 = LoadFloat32Vector( data + 0 );
        auto t1 = LoadFloat32Vector( data + 4 );
        auto t2 = SetSingle( data[8] );
        t0 = SSE::Abs( t0 );
        t1 = SSE::Abs( t1 );
        t2 = SSE::AbsSingle( t2 );
        Float3x3 result;
        float * __restrict result_data = begin( result[0] );
        Store( t0, result_data + 0 );
        Store( t1, result_data + 4 );
        result_data[8] = GetSingle( t2 );
        return result;
    }


    Float3 CopySign( Float3 const & magnitude, Float3 const & sign )
    {
        using namespace SSE;
        return Float3FromSSE( CopySign( SSEFromFloat3( magnitude ), SSEFromFloat3( sign ) ) );
    }


    Float4 CopySign( Float4 const & magnitude, Float4 const & sign )
    {
        using namespace SSE;
        return Float4FromSSE( CopySign( SSEFromFloat4( magnitude ), SSEFromFloat4( sign ) ) );
    }


    Quaternion CopySign( Quaternion const & magnitude, Quaternion const & sign )
    {
        using namespace SSE;
        return QuaternionFromSSE( CopySign( SSEFromQuaternion( magnitude ), SSEFromQuaternion( sign ) ) );
    }


    template<> bool Equal<Quaternion>(Quaternion const & first, Quaternion const & second, float const tolerance)
    {
        return Equal(Float4(first), Float4(second), tolerance);
    }

}

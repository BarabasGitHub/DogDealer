#include "Transforms.hlsli"
#include "Types.hlsli"

ProjectedPosition main( Position position_in : POSITION, AffineTransform object_matrix : INSTANCE_WORLDMATRIX ) : SV_POSITION
{
    Position position = TransformPosition( position_in, object_matrix );
    return LightFromRelativeWorldPosition(position);
}
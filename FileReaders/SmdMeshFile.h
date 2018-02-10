#include "SmdFile.h"

#include "FileStructs.h"
#include "..\Math\MathFunctions.h"

#include <set>

class SmdMeshFile	:	public	SmdFile
{

private:

	// Used for sorting of Vertices in the vertex set
	// Sorting first by position, then by UV and finally by normal
	struct VertexComparePosUVNorm{
		bool operator()( FileReading::Vertex v1, FileReading::Vertex v2 ) const {

			if((Math::Equal(v1.pos,v2.pos, 0.00001f)))
			{
				if((Math::Equal(v1.uv,v2.uv, 0.00001f))){
					return(Math::LessXYZ(v1.norm, v2.norm));
				}else{
					return(Math::LessXY(v1.uv,v2.uv));
				}
			}else{
				return(Math::LessXYZ(v1.pos,v2.pos));
			}
		}
	};



	std::vector<FileReading::Vertex>							m_vertices;
	std::set<FileReading::Vertex,VertexComparePosUVNorm>		m_vertex_set;
	std::vector<unsigned>						m_indices;

	int		m_vertex_count;

	FileReading::BoneWeightsAndIndicesContainer		m_bone_weights_and_indices;

	void		ReadVertexData();
	void		ProcessVertexSet();

	void					AddVertexEntry(std::string line);
	FileReading::Vertex		ReadVertexEntry(std::string line);
	void					ReadVertexWeightMapping(std::stringstream& line_stream);

	static Math::Float4 NormalizeWeightset(Math::Float4 weight);

public:
	SmdMeshFile( std::wstring filename );

	FileReading::PositionsContainer		GetVertexPositions() const;
    FileReading::NormalsContainer		GetVertexNormals() const;
    FileReading::UVsContainer			GetVertexUV() const;
    FileReading::ColorsContainer		GetVertexColors() const;

    FileReading::BoneWeightsAndIndicesContainer const & GetBoneWeightsAndIndices() const{return m_bone_weights_and_indices;}

	FileReading::IndicesContainer const & GetIndices() const;
    static D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology();
};
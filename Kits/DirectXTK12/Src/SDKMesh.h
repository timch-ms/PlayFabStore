//--------------------------------------------------------------------------------------
// File: SDKMesh.h
//
// SDKMESH format is generated by the legacy DirectX SDK's Content Exporter and
// originally rendered by the DXUT helper class SDKMesh
//
// http://go.microsoft.com/fwlink/?LinkId=226208
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace DXUT
{
    // .SDKMESH files

    // SDKMESH_HEADER
    // SDKMESH_VERTEX_BUFFER_HEADER header->VertexStreamHeadersOffset
    // SDKMESH_INDEX_BUFFER_HEADER  header->IndexStreamHeadersOffset
    // SDKMESH_MESH                 header->MeshDataOffset
    // SDKMESH_SUBSET               header->SubsetDataOffset
    // SDKMESH_FRAME                header->FrameDataOffset
    // SDKMESH_MATERIAL             header->MaterialDataOffset
    // [header->NonBufferDataSize]
    // { [ header->NumVertexBuffers]
    //      VB data
    // }
    // { [ header->NumIndexBuffers]
    //      IB data
    // }


    // .SDDKANIM files

    // SDKANIMATION_FILE_HEADER
    // uint8_t[] - Length of fileheader->AnimationDataSize

    // .SDKMESH uses Direct3D 9 decls, but only a subset of these is ever generated by the legacy DirectX SDK Content Exporter

    // D3DDECLUSAGE_POSITION / D3DDECLTYPE_FLOAT3
    // (D3DDECLUSAGE_BLENDWEIGHT / D3DDECLTYPE_UBYTE4N
    // D3DDECLUSAGE_BLENDINDICES / D3DDECLTYPE_UBYTE4)?
    // (D3DDECLUSAGE_NORMAL / D3DDECLTYPE_FLOAT3, D3DDECLTYPE_FLOAT16_4, D3DDECLTYPE_SHORT4N, D3DDECLTYPE_UBYTE4N, or D3DDECLTYPE_DEC3N)?
    // (D3DDECLUSAGE_COLOR / D3DDECLTYPE_D3DCOLOR)?
    // (D3DDECLUSAGE_TEXCOORD / D3DDECLTYPE_FLOAT1, D3DDECLTYPE_FLOAT2 or D3DDECLTYPE_FLOAT16_2, D3DDECLTYPE_FLOAT3 or D3DDECLTYPE_FLOAT16_4, D3DDECLTYPE_FLOAT4 or D3DDECLTYPE_FLOAT16_4)*
    // (D3DDECLUSAGE_TANGENT / same as D3DDECLUSAGE_NORMAL)?
    // (D3DDECLUSAGE_BINORMAL / same as D3DDECLUSAGE_NORMAL)?

    enum D3DDECLUSAGE
    {
        D3DDECLUSAGE_POSITION = 0,
        D3DDECLUSAGE_BLENDWEIGHT =1,
        D3DDECLUSAGE_BLENDINDICES =2,
        D3DDECLUSAGE_NORMAL =3,
        D3DDECLUSAGE_TEXCOORD = 5,
        D3DDECLUSAGE_TANGENT = 6,
        D3DDECLUSAGE_BINORMAL = 7,
        D3DDECLUSAGE_COLOR = 10,
    };

    enum D3DDECLTYPE
    {
        D3DDECLTYPE_FLOAT1    =  0,  // 1D float expanded to (value, 0., 0., 1.)
        D3DDECLTYPE_FLOAT2    =  1,  // 2D float expanded to (value, value, 0., 1.)
        D3DDECLTYPE_FLOAT3    =  2,  // 3D float expanded to (value, value, value, 1.)
        D3DDECLTYPE_FLOAT4    =  3,  // 4D float
        D3DDECLTYPE_D3DCOLOR  =  4,  // 4D packed unsigned bytes mapped to 0. to 1. range
                                     // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
        D3DDECLTYPE_UBYTE4    =  5,  // 4D unsigned uint8_t
        D3DDECLTYPE_UBYTE4N   =  8,  // Each of 4 bytes is normalized by dividing to 255.0
        D3DDECLTYPE_SHORT4N   = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
        D3DDECLTYPE_DEC3N     = 14,  // 3D signed normalized (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1.)
                                     // Note: There is no equivalent to D3DDECLTYPE_DEC3N (14) as a DXGI_FORMAT
        D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
        D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values

        D3DDECLTYPE_UNUSED    = 17,  // When the type field in a decl is unused.

        // These are extensions for DXGI-based versions of Direct3D
        D3DDECLTYPE_DXGI_R10G10B10A2_UNORM = 32 + DXGI_FORMAT_R10G10B10A2_UNORM,
        D3DDECLTYPE_DXGI_R11G11B10_FLOAT   = 32 + DXGI_FORMAT_R11G11B10_FLOAT,
        D3DDECLTYPE_DXGI_R8G8B8A8_SNORM    = 32 + DXGI_FORMAT_R8G8B8A8_SNORM,
    };

    #pragma pack(push,4)

    struct D3DVERTEXELEMENT9
    {
        uint16_t Stream;     // Stream index
        uint16_t Offset;     // Offset in the stream in bytes
        uint8_t  Type;       // Data type
        uint8_t  Method;     // Processing method
        uint8_t  Usage;      // Semantics
        uint8_t  UsageIndex; // Semantic index
    };

    #pragma pack(pop)

    //--------------------------------------------------------------------------------------
    // Hard Defines for the various structures
    //--------------------------------------------------------------------------------------
    constexpr uint32_t SDKMESH_FILE_VERSION = 101;
    constexpr uint32_t SDKMESH_FILE_VERSION_V2 = 200;

    constexpr uint32_t MAX_VERTEX_ELEMENTS = 32;
    constexpr uint32_t MAX_VERTEX_STREAMS = 16;
    constexpr uint32_t MAX_FRAME_NAME = 100;
    constexpr uint32_t MAX_MESH_NAME = 100;
    constexpr uint32_t MAX_SUBSET_NAME = 100;
    constexpr uint32_t MAX_MATERIAL_NAME = 100;
    constexpr uint32_t MAX_TEXTURE_NAME = MAX_PATH;
    constexpr uint32_t MAX_MATERIAL_PATH = MAX_PATH;
    constexpr uint32_t INVALID_FRAME = uint32_t(-1);
    constexpr uint32_t INVALID_MESH =  uint32_t(-1);
    constexpr uint32_t INVALID_MATERIAL = uint32_t(-1);
    constexpr uint32_t INVALID_SUBSET = uint32_t(-1);
    constexpr uint32_t INVALID_ANIMATION_DATA = uint32_t(-1);

    //--------------------------------------------------------------------------------------
    // Enumerated Types.
    //--------------------------------------------------------------------------------------
    enum SDKMESH_PRIMITIVE_TYPE
    {
        PT_TRIANGLE_LIST = 0,
        PT_TRIANGLE_STRIP,
        PT_LINE_LIST,
        PT_LINE_STRIP,
        PT_POINT_LIST,
        PT_TRIANGLE_LIST_ADJ,
        PT_TRIANGLE_STRIP_ADJ,
        PT_LINE_LIST_ADJ,
        PT_LINE_STRIP_ADJ,
        PT_QUAD_PATCH_LIST,
        PT_TRIANGLE_PATCH_LIST,
    };

    enum SDKMESH_INDEX_TYPE
    {
        IT_16BIT = 0,
        IT_32BIT,
    };

    enum FRAME_TRANSFORM_TYPE
    {
        FTT_RELATIVE = 0,
        FTT_ABSOLUTE, // This is not currently used but is here to support absolute transformations in the future
    };

    //--------------------------------------------------------------------------------------
    // Structures.
    //--------------------------------------------------------------------------------------
    #pragma pack(push,8)

    struct SDKMESH_HEADER
    {
        //Basic Info and sizes
        uint32_t Version;
        uint8_t  IsBigEndian;
        uint64_t HeaderSize;
        uint64_t NonBufferDataSize;
        uint64_t BufferDataSize;

        //Stats
        uint32_t NumVertexBuffers;
        uint32_t NumIndexBuffers;
        uint32_t NumMeshes;
        uint32_t NumTotalSubsets;
        uint32_t NumFrames;
        uint32_t NumMaterials;

        //Offsets to Data
        uint64_t VertexStreamHeadersOffset;
        uint64_t IndexStreamHeadersOffset;
        uint64_t MeshDataOffset;
        uint64_t SubsetDataOffset;
        uint64_t FrameDataOffset;
        uint64_t MaterialDataOffset;
    };

    struct SDKMESH_VERTEX_BUFFER_HEADER
    {
        uint64_t NumVertices;
        uint64_t SizeBytes;
        uint64_t StrideBytes;
        D3DVERTEXELEMENT9 Decl[MAX_VERTEX_ELEMENTS];
        uint64_t DataOffset;
    };

    struct SDKMESH_INDEX_BUFFER_HEADER
    {
        uint64_t NumIndices;
        uint64_t SizeBytes;
        uint32_t IndexType;
        uint64_t DataOffset;
    };

    struct SDKMESH_MESH
    {
        char Name[MAX_MESH_NAME];
        uint8_t NumVertexBuffers;
        uint32_t VertexBuffers[MAX_VERTEX_STREAMS];
        uint32_t IndexBuffer;
        uint32_t NumSubsets;
        uint32_t NumFrameInfluences; //aka bones

        DirectX::XMFLOAT3 BoundingBoxCenter;
        DirectX::XMFLOAT3 BoundingBoxExtents;

        union
        {
            uint64_t SubsetOffset;
            INT* pSubsets;
        };
        union
        {
            uint64_t FrameInfluenceOffset;
            uint32_t* pFrameInfluences;
        };
    };

    struct SDKMESH_SUBSET
    {
        char Name[MAX_SUBSET_NAME];
        uint32_t MaterialID;
        uint32_t PrimitiveType;
        uint64_t IndexStart;
        uint64_t IndexCount;
        uint64_t VertexStart;
        uint64_t VertexCount;
    };

    struct SDKMESH_FRAME
    {
        char Name[MAX_FRAME_NAME];
        uint32_t Mesh;
        uint32_t ParentFrame;
        uint32_t ChildFrame;
        uint32_t SiblingFrame;
        DirectX::XMFLOAT4X4 Matrix;
        uint32_t AnimationDataIndex; //Used to index which set of keyframes transforms this frame
    };

    struct SDKMESH_MATERIAL
    {
        char    Name[MAX_MATERIAL_NAME];

        // Use MaterialInstancePath
        char    MaterialInstancePath[MAX_MATERIAL_PATH];

        // Or fall back to d3d8-type materials
        char    DiffuseTexture[MAX_TEXTURE_NAME];
        char    NormalTexture[MAX_TEXTURE_NAME];
        char    SpecularTexture[MAX_TEXTURE_NAME];

        DirectX::XMFLOAT4 Diffuse;
        DirectX::XMFLOAT4 Ambient;
        DirectX::XMFLOAT4 Specular;
        DirectX::XMFLOAT4 Emissive;
        float Power;

        uint64_t Force64_1;
        uint64_t Force64_2;
        uint64_t Force64_3;
        uint64_t Force64_4;
        uint64_t Force64_5;
        uint64_t Force64_6;
    };

    struct SDKMESH_MATERIAL_V2
    {
        char    Name[MAX_MATERIAL_NAME];

        // PBR materials
        char    RMATexture[MAX_TEXTURE_NAME];
        char    AlbedoTexture[MAX_TEXTURE_NAME];
        char    NormalTexture[MAX_TEXTURE_NAME];
        char    EmissiveTexture[MAX_TEXTURE_NAME];

        float   Alpha;

        char    Reserved[60];

        uint64_t Force64_1;
        uint64_t Force64_2;
        uint64_t Force64_3;
        uint64_t Force64_4;
        uint64_t Force64_5;
        uint64_t Force64_6;
    };

    struct SDKANIMATION_FILE_HEADER
    {
        uint32_t Version;
        uint8_t  IsBigEndian;
        uint32_t FrameTransformType;
        uint32_t NumFrames;
        uint32_t NumAnimationKeys;
        uint32_t AnimationFPS;
        uint64_t AnimationDataSize;
        uint64_t AnimationDataOffset;
    };

    struct SDKANIMATION_DATA
    {
        DirectX::XMFLOAT3 Translation;
        DirectX::XMFLOAT4 Orientation;
        DirectX::XMFLOAT3 Scaling;
    };

    struct SDKANIMATION_FRAME_DATA
    {
        char FrameName[MAX_FRAME_NAME];
        uint64_t DataOffset;
    };

    #pragma pack(pop)

} // namespace

static_assert( sizeof(DXUT::D3DVERTEXELEMENT9) == 8, "Direct3D9 Decl structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_HEADER)== 104, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_VERTEX_BUFFER_HEADER) == 288, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_INDEX_BUFFER_HEADER) == 32, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_MESH) == 224, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_SUBSET) == 144, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_FRAME) == 184, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_MATERIAL) == 1256, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_MATERIAL_V2) == sizeof(DXUT::SDKMESH_MATERIAL), "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKANIMATION_FILE_HEADER) == 40, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKANIMATION_DATA) == 40, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKANIMATION_FRAME_DATA) == 112, "SDK Mesh structure size incorrect" );

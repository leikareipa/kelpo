/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Software: File reader for the KAC 1.0 data format.
 * 
 * Provides functionality to read data from a KAC 1.0 file in an organized manner.
 * 
 * NOTE: This implementation assumes little-endian byte ordering and 32-bit floats.
 * 
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "import_kac_1_0.h"

/* An ID for each of the possible segments in a KAC 1.0 file.*/
enum
{
    KAC_1_0_SEGMENT_ID_KAC = 0,
    KAC_1_0_SEGMENT_ID_MATE,
    KAC_1_0_SEGMENT_ID_TXMD,
    KAC_1_0_SEGMENT_ID_VERT,
    KAC_1_0_SEGMENT_ID_NORM,
    KAC_1_0_SEGMENT_ID_UV,
    KAC_1_0_SEGMENT_ID_3MSH,
    KAC_1_0_SEGMENT_ID_TXPX,
    KAC_1_0_SEGMENT_ID_ENDS,

    /* Must be the last entry in this list.*/
    KAC_1_0_NUM_SEGMENTS
};

/* The KAC file we'll be reading from. This variable will be assigned at run-time
 * by a call to kac10_reader__open_file().*/
static FILE *INPUT_FILE;

/* Bit flags (KAC_1_0_SEGMENT_ID_xxx) for whether a given segment exists in the
 * current KAC file. This will be initialized by kac10_reader__open_file().*/
static uint32_t SEGMENTS_IN_FILE;

/* Byte offsets in the input file of the various data segments. This will be
 * initialized by kac10_reader__open_file().*/
static uint32_t SEGMENT_BYTE_OFFSETS[KAC_1_0_NUM_SEGMENTS];

int kac10_reader__input_stream_is_valid(void)
{
    return ((INPUT_FILE != NULL) &&
            !ferror(INPUT_FILE) &&
            !feof(INPUT_FILE));
}

static int scan_input_file_structure(void)
{
    size_t byteOffset = 0;

    assert(INPUT_FILE && "Attempting to scan a null input file.");

    #define SEGMENT_IDENTIFIER_IS(name) (int)(strncmp((name), segmentIdentifier, 4) == 0)

    #define SKIP_SEGMENT_DATA(elementByteSize) {uint32_t n = 0;\
                                                fread((char*)&n, sizeof(n), 1, INPUT_FILE);\
                                                fseek(INPUT_FILE, (n * (elementByteSize)), SEEK_CUR);\
                                                byteOffset = ftell(INPUT_FILE);}

    /* Loop through all segments in the file.*/
    while (1)
    {
        /* Note: The starting offset skips the 4-byte segment identifier.*/
        const int32_t segmentStartingOffset = (ftell(INPUT_FILE) + 4);
        char segmentIdentifier[4];

        assert((segmentStartingOffset != -1l) && "A call to ftell() failed.");

        fread(segmentIdentifier, 1, 4, INPUT_FILE);

        if (!kac10_reader__input_stream_is_valid())
        {
            fprintf(stderr, "ERROR: The KAC file is malformed\n");
            return 0;
        }

        if (SEGMENT_IDENTIFIER_IS("KAC "))
        {
            float fileFormatVersion = 0.0;

            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_KAC);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_KAC] = segmentStartingOffset;

            /* TODO: Test that this segment is the first in the file.*/

            fread((char*)&fileFormatVersion, sizeof(fileFormatVersion), 1, INPUT_FILE);
            if (fileFormatVersion != 1.0)
            {
                fprintf(stderr, "ERROR: The KAC file is of version %f, but only version 1.0 "
                                "files are supported by this program.\n", fileFormatVersion);
                return 0;
            }
        }
        else if (SEGMENT_IDENTIFIER_IS("ENDS"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_ENDS);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_ENDS] = segmentStartingOffset;

            /* TODO: Test that this segment is the last in the file.*/

            break;
        }
        else if (SEGMENT_IDENTIFIER_IS("MATE"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_MATE);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_MATE] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(4);
        }
        else if (SEGMENT_IDENTIFIER_IS("TXMD"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_TXMD);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_TXMD] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(20);
        }
        else if (SEGMENT_IDENTIFIER_IS("VERT"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_VERT);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_VERT] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(12);
        }
        else if (SEGMENT_IDENTIFIER_IS("NORM"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_NORM);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_NORM] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(12);
        }
        else if (SEGMENT_IDENTIFIER_IS("UV  "))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_UV);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_UV] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(8);
        }
        else if (SEGMENT_IDENTIFIER_IS("3MSH"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_3MSH);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_3MSH] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(20);
        }
        else if (SEGMENT_IDENTIFIER_IS("TXPX"))
        {
            SEGMENTS_IN_FILE |= (1 << KAC_1_0_SEGMENT_ID_TXPX);
            SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_TXPX] = segmentStartingOffset;
            SKIP_SEGMENT_DATA(2);
        }
        else
        {
            fprintf(stderr, "ERROR: The KAC file contains an unrecognized segment "
                            "\"%s\" at byte offset %lu\n", segmentIdentifier, byteOffset);
            return 0;
        }
    }

    #undef SEGMENT_IDENTIFIER_IS
    #undef SKIP_SEGMENT_DATA

    /* Each KAC file needs to contain at least the KAC header and an ENDS segment
     * for the file to be considered validly formed.*/
    {
        const uint32_t requiredSegments = (KAC_1_0_SEGMENT_ID_KAC | KAC_1_0_SEGMENT_ID_ENDS);

        if ((SEGMENTS_IN_FILE & requiredSegments) != requiredSegments)
        {
            return 0;
        }
    }

    return 1;
}

int kac10_reader__open_file(const char *const filename)
{
    assert(!INPUT_FILE && "Attempting to open a new KAC file before closing the previous one.");

    INPUT_FILE = fopen(filename, "rb");
    SEGMENTS_IN_FILE = 0;

    return ((INPUT_FILE != NULL) &&
            scan_input_file_structure());
}

/* Closes the input file opened by kac10_reader__open_file().*/
int kac10_reader__close_file(void)
{
    if (fclose(INPUT_FILE) == EOF)
    {
        return 0;
    }

    INPUT_FILE = NULL;
    SEGMENTS_IN_FILE = 0;

    return 1;
}

uint32_t kac10_reader__read_normals(struct kac_1_0_normal_s **normals)
{
    uint32_t i, numNormals = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_normals())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_NORM], SEEK_SET);
    fread((char*)&numNormals, sizeof(numNormals), 1, INPUT_FILE);

    *normals = calloc(numNormals, sizeof(struct kac_1_0_normal_s));
    for (i = 0; i < numNormals; i++)
    {
        fread((char*)&(*normals)[i].x, sizeof((*normals)[i].x), 1, INPUT_FILE);
        fread((char*)&(*normals)[i].y, sizeof((*normals)[i].y), 1, INPUT_FILE);
        fread((char*)&(*normals)[i].z, sizeof((*normals)[i].z), 1, INPUT_FILE);
    }

    return (kac10_reader__input_stream_is_valid()? numNormals : 0);
}

uint32_t kac10_reader__read_uv_coordinates(struct kac_1_0_uv_coordinates_s **uvCoords)
{
    uint32_t i, numUVCoords = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_uv_coordinates())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_UV], SEEK_SET);
    fread((char*)&numUVCoords, sizeof(numUVCoords), 1, INPUT_FILE);

    *uvCoords = calloc(numUVCoords, sizeof(struct kac_1_0_normal_s));
    for (i = 0; i < numUVCoords; i++)
    {
        fread((char*)&(*uvCoords)[i].u, sizeof((*uvCoords)[i].u), 1, INPUT_FILE);
        fread((char*)&(*uvCoords)[i].v, sizeof((*uvCoords)[i].v), 1, INPUT_FILE);
    }

    return (kac10_reader__input_stream_is_valid()? numUVCoords : 0);
}

uint32_t kac10_reader__read_texture_pixels(struct kac_1_0_texture_pixel_s **texturePixels)
{
    uint32_t i, numPixels = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_texture_pixels())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_TXPX], SEEK_SET);
    fread((char*)&numPixels, sizeof(numPixels), 1, INPUT_FILE);

    *texturePixels = calloc(numPixels, sizeof(struct kac_1_0_texture_pixel_s));
    for (i = 0; i < numPixels; i++)
    {
        uint16_t packedPixel = 0;

        fread((char*)&packedPixel, sizeof(packedPixel), 1, INPUT_FILE);

        (*texturePixels)[i].r = ((packedPixel >> 0)  & 0x1f);
        (*texturePixels)[i].g = ((packedPixel >> 5)  & 0x1f);
        (*texturePixels)[i].b = ((packedPixel >> 10) & 0x1f);
        (*texturePixels)[i].a = ((packedPixel >> 15) & 0x1);
    }

    return (kac10_reader__input_stream_is_valid()? numPixels : 0);
}

uint32_t kac10_reader__read_texture_metadata(struct kac_1_0_texture_metadata_s **textureMetadata)
{
    uint32_t i, numMetadataElements = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_texture_metadata())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_TXMD], SEEK_SET);
    fread((char*)&numMetadataElements, sizeof(numMetadataElements), 1, INPUT_FILE);

    *textureMetadata = calloc(numMetadataElements, sizeof(struct kac_1_0_texture_metadata_s));
    for (i = 0; i < numMetadataElements; i++)
    {
        uint32_t parameters = 0;
        uint8_t pixelHash[16] = {0};

        fread((char*)&parameters, sizeof(parameters), 1, INPUT_FILE);
        fread((char*)pixelHash, sizeof(pixelHash), 1, INPUT_FILE);

        (*textureMetadata)[i].sideLengthExponent = ((parameters >> 0) & 0x7);
        (*textureMetadata)[i].pixelDataOffset = ((parameters >> 3) & 0x1ffffff);
        memcpy((*textureMetadata)[i].pixelHash, pixelHash, sizeof(pixelHash));
    }

    return (kac10_reader__input_stream_is_valid()? numMetadataElements : 0);
}

uint32_t kac10_reader__read_materials(struct kac_1_0_material_s **materials)
{
    uint32_t i, numMaterials = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_materials())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_MATE], SEEK_SET);
    fread((char*)&numMaterials, sizeof(numMaterials), 1, INPUT_FILE);

    *materials = calloc(numMaterials, sizeof(struct kac_1_0_material_s));
    for (i = 0; i < numMaterials; i++)
    {
        uint32_t material = 0;

        fread((char*)&material, sizeof(material), 1, INPUT_FILE);

        (*materials)[i].color.r = ((material >> 0) & 0xf);
        (*materials)[i].color.g = ((material >> 4) & 0xf);
        (*materials)[i].color.b = ((material >> 8) & 0xf);
        (*materials)[i].color.a = ((material >> 12) & 0xf);

        (*materials)[i].metadata.textureMetadataIdx  = ((material >> 16) & 0x1ff);
        (*materials)[i].metadata.hasTexture          = ((material >> 25) & 0x1);
        (*materials)[i].metadata.hasTextureFiltering = ((material >> 26) & 0x1);
        (*materials)[i].metadata.hasSmoothShading    = ((material >> 27) & 0x1);
    }

    return (kac10_reader__input_stream_is_valid()? numMaterials : 0);
}

uint32_t kac10_reader__read_vertex_coordinates(struct kac_1_0_vertex_coordinates_s **vertexCoords)
{
    uint32_t i, numVertexCoords = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_vertex_coordinates())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_VERT], SEEK_SET);
    fread((char*)&numVertexCoords, sizeof(numVertexCoords), 1, INPUT_FILE);

    *vertexCoords = calloc(numVertexCoords, sizeof(struct kac_1_0_vertex_coordinates_s));
    for (i = 0; i < numVertexCoords; i++)
    {
        fread((char*)&(*vertexCoords)[i].x, sizeof((*vertexCoords)[i].x), 1, INPUT_FILE);
        fread((char*)&(*vertexCoords)[i].y, sizeof((*vertexCoords)[i].y), 1, INPUT_FILE);
        fread((char*)&(*vertexCoords)[i].z, sizeof((*vertexCoords)[i].z), 1, INPUT_FILE);
    }

    return (kac10_reader__input_stream_is_valid()? numVertexCoords : 0);
}

uint32_t kac10_reader__read_textures(struct kac_1_0_texture_s **textures)
{
    uint32_t i, numTextures = 0, numPixels = 0;
    struct kac_1_0_texture_pixel_s *pixels = NULL;
    struct kac_1_0_texture_metadata_s *metadata = NULL;

    /* First load in as individual segments the texture metadata and pixels,
     * then combine them into whole textures.*/
    if ((numTextures = kac10_reader__read_texture_metadata(&metadata)) &&
        (numPixels = kac10_reader__read_texture_pixels(&pixels)))
    {
        *textures = calloc(numTextures, sizeof(struct kac_1_0_texture_s));
        for (i = 0; i < numTextures; i++)
        {
            (*textures)[i].metadata = metadata[i];

            /* Copy this texture's portion of the master pixel data array into
             * the texture.*/
            {
                const uint32_t pixelSideLen = pow(2, ((*textures)[i].metadata.sideLengthExponent + 1));
                const uint32_t numPixelsInTexture = (pixelSideLen * pixelSideLen);

                (*textures)[i].pixels = malloc(sizeof(struct kac_1_0_texture_pixel_s) * numPixelsInTexture);
                
                memcpy((*textures)[i].pixels,
                       (pixels + (*textures)[i].metadata.pixelDataOffset),
                       (numPixelsInTexture * sizeof(pixels[0])));
            }
        }

        /* We've copied all the relevant data into the texture elements, so we
         * can free the temporary buffers.*/
        free(metadata);
        free(pixels);

        return numTextures;
    }

    return 0;
}

uint32_t kac10_reader__read_triangles(struct kac_1_0_triangle_s **triangles)
{
    uint32_t i, numTriangles = 0;

    if (!kac10_reader__input_stream_is_valid() ||
        !kac10_reader__file_has_vertex_coordinates())
    {
        return 0;
    }

    fseek(INPUT_FILE, SEGMENT_BYTE_OFFSETS[KAC_1_0_SEGMENT_ID_3MSH], SEEK_SET);
    fread((char*)&numTriangles, sizeof(numTriangles), 1, INPUT_FILE);

    *triangles = calloc(numTriangles, sizeof(struct kac_1_0_triangle_s));
    for (i = 0; i < numTriangles; i++)
    {
        uint32_t v;

        fread((char*)&(*triangles)[i].materialIdx, sizeof((*triangles)[i].materialIdx), 1, INPUT_FILE);

        for (v = 0; v < 3; v++)
        {
            struct kac_1_0_vertex_s *vertex = &(*triangles)[i].vertices[v];

            fread((char*)&vertex->vertexCoordinatesIdx, sizeof(vertex->vertexCoordinatesIdx), 1, INPUT_FILE);
            fread((char*)&vertex->normalIdx, sizeof(vertex->normalIdx), 1, INPUT_FILE);
            fread((char*)&vertex->uvIdx, sizeof(vertex->uvIdx), 1, INPUT_FILE);
        }
    }

    return (kac10_reader__input_stream_is_valid()? numTriangles : 0);
}

int kac10_reader__file_has_textures(void)
{
    /* Texture data in a KAC 1.0 file is divided into two segments: the metadata
     * segment, and the pixel data segment. Both of these segments need to be
     * present for the textures to be extractable.*/
    return (kac10_reader__file_has_texture_metadata() &&
            kac10_reader__file_has_texture_pixels());
}

int kac10_reader__file_has_texture_metadata(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_TXMD));
}

int kac10_reader__file_has_texture_pixels(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_TXPX));
}

int kac10_reader__file_has_normals(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_NORM));
}

int kac10_reader__file_has_materials(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_MATE));
}

int kac10_reader__file_has_triangles(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_3MSH));
}

int kac10_reader__file_has_uv_coordinates(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_UV));
}

int kac10_reader__file_has_vertex_coordinates(void)
{
    return (SEGMENTS_IN_FILE & (1 << KAC_1_0_SEGMENT_ID_VERT));
}

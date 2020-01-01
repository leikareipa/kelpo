/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Software: File reader for the KAC 1.0 data format.
 * 
 * Provides functionality to read data from a KAC 1.0 file in an organized manner.
 * 
 */

#ifndef IMPORT_KAC_1_0_H
#define IMPORT_KAC_1_0_H

#include "kac_1_0_types.h"

/* Sets the target file to be read from. Returns 1 if the target is a valid KAC
 * 1.0 file that is ready to be read from; otherwise, returns 0. This function
 * must be called prior to calling any functions that read data, since they
 * operate on the file set by this function.*/
int kac10_reader__open_file(const char *const filename);

/* Closes the target file set by kac10_reader__open_file(). Returns 1 if
 * the file was successfully closed; 0 otherwise.*/
int kac10_reader__close_file(void);

/* Returns 1 if the input file's IO stream is currently valid (the file is open,
 * there have been no read errors, etc.); otherwise, returns 0.*/
int kac10_reader__input_stream_is_valid(void);

/* Reads the given segment (e.g. normals) from the KAC 1.0 file. Takes in an
 * uninitialized (or NULL) pointer to a pointer, which will be initialized by
 * the function call to point to memory holding the data read from the KAC file.
 * Returns the number of elements read. If the value returned is 0, the file
 * either did not contain the given segment, or the segment existed but held no
 * data.
 * 
 * These functions operate on the file set by kac10_reader__open_file(),
 * which in other words needs to be called prior to calling these readers.
 */
uint32_t kac10_reader__read_normals(struct kac_1_0_normal_s **normals);
uint32_t kac10_reader__read_textures(struct kac_1_0_texture_s **textures);
uint32_t kac10_reader__read_materials(struct kac_1_0_material_s **materials);
uint32_t kac10_reader__read_triangles(struct kac_1_0_triangle_s **triangles);
uint32_t kac10_reader__read_uv_coordinates(struct kac_1_0_uv_coordinates_s **uvCoords);
uint32_t kac10_reader__read_texture_pixels(struct kac_1_0_texture_pixel_s **texturePixels);
uint32_t kac10_reader__read_texture_metadata(struct kac_1_0_texture_metadata_s **textureMetadata);
uint32_t kac10_reader__read_vertex_coordinates(struct kac_1_0_vertex_coordinates_s **vertexCoords);

/* Returns 1 if the file (specified with kac10_reader__open_file()) contains
 * the given segment; otherwise, 0 is returned.*/
int kac10_reader__file_has_normals(void);
int kac10_reader__file_has_textures(void);
int kac10_reader__file_has_materials(void);
int kac10_reader__file_has_triangles(void);
int kac10_reader__file_has_uv_coordinates(void);
int kac10_reader__file_has_texture_pixels(void);
int kac10_reader__file_has_texture_metadata(void);
int kac10_reader__file_has_vertex_coordinates(void);

#endif

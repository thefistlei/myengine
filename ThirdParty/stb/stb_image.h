#pragma once

#define STBI_VERSION 1

typedef unsigned char stbi_uc;

#ifdef __cplusplus
extern "C" {
#endif

void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);
stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
void stbi_image_free(void *retval_from_stbi_load);

#ifdef STB_IMAGE_IMPLEMENTATION
void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip) {}
stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels) {
    *x = 1; *y = 1; *channels_in_file = 4;
    stbi_uc* data = (stbi_uc*)malloc(4);
    data[0] = 255; data[1] = 255; data[2] = 255; data[3] = 255;
    return data;
}
void stbi_image_free(void *retval_from_stbi_load) { free(retval_from_stbi_load); }
#endif

#ifdef __cplusplus
}
#endif

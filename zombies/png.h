
int write_png(const char *file, unsigned char *image_data, unsigned int width, unsigned int height);
int write_frame_float(const char *file, float *frame_data, unsigned int width, unsigned int height);
int write_frame_float_scale(const char *file, float *frame_data, unsigned int width, unsigned int height,float min,float max);


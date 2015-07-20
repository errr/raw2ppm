#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libraw/libraw.h>

#define BLOCK_SIZE 1024*1024

#define SWAP(a, b) { a ^= b; a ^= (b ^= a); }


void libraw_error(ret)
{
    fprintf(stderr, "libraw  %s\n", libraw_strerror(ret));

    if (LIBRAW_FATAL_ERROR(ret))
        exit(1);
}


int main(int argc, char *argv[])
{
    char *buf = NULL;
    size_t buf_size = 0;
    char rbuf[BLOCK_SIZE];
    size_t n;

    for (;;) {
        n = fread(rbuf, sizeof(char), BLOCK_SIZE, stdin);
        buf_size += n;
        buf = realloc(buf, buf_size);
        memcpy(buf + (buf_size - n), rbuf, n);

        if (n < BLOCK_SIZE) {
            if (feof(stdin)) {
                break;
            } else {
                fprintf(stderr, "Read error\n");
                exit(1);
            }
        }
    }

    libraw_data_t *iprc = libraw_init(0);

    if (!iprc) {
        fprintf(stderr,"Cannot create libraw handle\n");
        exit(1);
    }

    iprc->params.user_mul[0] = 1;
    iprc->params.user_mul[1] = 0.5;
    iprc->params.user_mul[2] = 1;
    iprc->params.user_mul[3] = 0.5;
    iprc->params.use_camera_wb = 1;
    iprc->params.use_camera_matrix = 1;
    iprc->params.highlight = 9;
    iprc->params.output_color = 0;
    iprc->params.output_bps = 16;
    iprc->params.user_qual = 1;
    iprc->params.four_color_rgb = 1;
    iprc->params.no_auto_bright = 1;


    int ret = libraw_open_buffer(iprc, buf, buf_size);
    if (ret) libraw_error(ret);

    ret = libraw_unpack(iprc);
    if (ret) libraw_error(ret);

    ret = libraw_dcraw_process(iprc);
    if (ret) libraw_error(ret);

    libraw_processed_image_t *out;
    out = libraw_dcraw_make_mem_image(iprc, &ret);
    if (ret) libraw_error(ret);

    printf("P6\n%d %d\n%d\n", out->width, out->height, out->bits == 16 ? 65535 : 255);

    int i;
    for (i = 0; i < out->data_size; i += 2)
        SWAP(out->data[i], out->data[i+1]);

    n = fwrite(out->data, sizeof(char), out->data_size, stdout);
    fflush(stdout);

    if (n < out->data_size) {
        fprintf(stderr, "Write error\n");
        exit(1);
    }

    return 0;
}

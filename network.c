#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mynet.h"

#define LAYERNUM 16

float get_input_pixel(int row, int col, int channel, int kernel_row, int kernel_col, int pad, int image_size, float *image)
{
    int im_col = col - pad + kernel_col;
    int im_row = row - pad + kernel_row;
    if (im_col < 0 || im_row < 0 || im_col >= image_size || im_row >= image_size)
    {
        return 0;
    }
    int channel_offset = channel * image_size * image_size;
    int row_offset = im_row * image_size;
    int col_offset = im_col;

    return image[channel_offset + row_offset + col_offset];
}

void scale_bias(float *output, float *scales, int out_channel, int size_out)
{
    for (int oc = 0; oc < out_channel; ++oc)
    {
        int channel_offset = oc * size_out * size_out;
        for (int r = 0; r < size_out; ++r)
        {
            int row_offset = r * size_out;
            for (int c = 0; c < size_out; ++c)
            {
                int col_offset = c;
                output[channel_offset + row_offset + col_offset] *= scales[oc];
            }
        }
    }
}

void add_bias(float *output, float *biases, int out_channel, int size_out)
{
    for (int oc = 0; oc < out_channel; ++oc)
    {
        int channel_offset = oc * size_out * size_out;
        for (int r = 0; r < size_out; ++r)
        {
            int row_offset = r * size_out;
            for (int c = 0; c < size_out; ++c)
            {
                int col_offset = c;
                output[channel_offset + row_offset + col_offset] += biases[oc];
            }
        }
    }
}

void forward_batchnorm(float *output, float *rolling_mean, float *rolling_variance, int out_channel, int size_out)
{
    for (int oc = 0; oc < out_channel; ++oc)
    {
        int channel_offset = oc * size_out * size_out;
        for (int r = 0; r < size_out; ++r)
        {
            int row_offset = r * size_out;
            for (int c = 0; c < size_out; ++c)
            {
                int col_offset = c;
                int index = col_offset + row_offset + channel_offset;
                output[index] = (output[index] - rolling_mean[oc]) / (sqrt(rolling_variance[oc]) + .000001f);
            }
        }
    }
}

static inline float leaky_activate(float x){return (x>0) ? x : .1*x;}
static inline float linear_activate(float x){return x;}

void activate_array(float *output, int array_len, ACTIVATION a)
{
    if(a==LINEAR){
        for(int i=0; i<array_len; ++i){
            output[i] = linear_activate(output[i]);
        }
    }
    else if (a==LEAKY){
        for(int i=0; i<array_len; ++i){
            output[i] = leaky_activate(output[i]);
        }
    }
    else{
        fprintf(stderr, "unimplemented activation\n");
    }
}

void forward_max_layer(layer l, network net)
{
    int size_out = l.size_out;
    int input_channel = l.c;

    for(int ic = 0; ic < input_channel; ++ic){
        int channel_offset = ic * size_out *size_out;
        for(int r = 0; r < size_out; ++r){
            int row_offset = r * size_out;
            for(int c = 0; c < size_out; ++c){
            }
        }
    }
}

void forward_region_layer(layer l, network net)
{

}

void forward_conv_layer(layer l, network net)
{
    if (l.type != CONVOLUTION)
    {
        fprintf(stderr, "non conv layer calls forward_conv_layer\n");
        exit(1);
    }

    int pad = l.pad;
    // int stride = l.stride;
    int ksize = l.size;
    int input_size = l.size_in;
    int input_channel = l.c;
    int output_size = l.size_out;
    int output_channel = l.n;
    float *out = l.output;
    // int im_r, im_c;
    float *weights = l.weights;
    float *input = net.input;

    int output_channel_offset_kernel;
    int input_channel_offset_kernel;
    int row_offset_kernel;
    int col_offset_kernel;

    int output_channel_offset_out;
    int row_offset_out;
    int col_offset_out;

    // assume weight aligned (w, h, ic, oc)
    // add padding
    // for each output channel
    for (int oc = 0; oc < output_channel; ++oc)
    {
        output_channel_offset_kernel = oc * ksize * ksize * output_channel;
        output_channel_offset_out = oc * output_size * output_size * output_channel;
        // for each input channel
        for (int ic = 0; ic < input_channel; ++ic)
        {
            input_channel_offset_kernel = ic * ksize * ksize;
            // for each row
            for (int r = 0; r < output_size; ++r)
            {
                row_offset_out = r * output_size;
                // for each col
                for (int c = 0; c < output_size; ++c)
                {
                    col_offset_out = c;
                    // for row in kernel
                    for (int i = 0; i < ksize; ++i)
                    {
                        row_offset_kernel = c * ksize;
                        // for col in kernel
                        for (int j = 0; j < ksize; ++j)
                        {
                            col_offset_kernel = j;
                            float kernel_value = weights[col_offset_kernel +
                                                         row_offset_kernel +
                                                         input_channel_offset_kernel +
                                                         output_channel_offset_kernel];
                            // get_input_pixel(int row, int col, int channel, int kernel_row, int kernel_col, int pad, int image_size, float *image)
                            float image_value = get_input_pixel(r, c, ic, i, j, pad, input_size, input);
                            out[col_offset_out +
                                row_offset_out +
                                output_channel_offset_out] += (kernel_value * image_value);
                            // out += w * get_input_pixel
                        }
                    }
                }
            }
        }
    }

    if (l.batch_normalize)
    {
        forward_batchnorm(l.output, l.rolling_mean, l.rolling_variance, l.n, l.size_out);
        scale_bias(l.output, l.scales, l.n, l.size_out);
        add_bias(l.output, l.biases, l.n, l.size_out);
    }
    else
    {
        add_bias(l.output, l.biases, l.n, l.size_out);
    }
    activate_array(l.output, l.size_out * l.size_out * l.n, l.activation);
}

void make_conv_layer(layer *l, int num_kernel, int kernel_size, int stride, int pad,
                     int num_channel_in, ACTIVATION act, int batch, int size_in, int size_out)
{
    int num;
    l->type = CONVOLUTION;
    l->size_in = size_in;
    l->size_out = size_out;
    l->n = num_kernel;
    l->size = kernel_size;
    l->stride = stride;
    l->pad = pad;
    l->c = num_channel_in;
    l->activation = act;
    l->batch_normalize = batch;
    // l->workspace_size = l->size_out * l->size_out * l->size * l->size * l->c * sizeof(float);

    l->forward = forward_conv_layer;

    num = l->c * l->n * l->size * l->size;
    l->biases = calloc(l->n, sizeof(float));
    l->weights = calloc(num, sizeof(float));
    l->output = calloc(l->size_out * l->size_out * l->n, sizeof(float));

    if (l->batch_normalize)
    {
        l->scales = calloc(l->n, sizeof(float));
        l->rolling_mean = calloc(l->n, sizeof(float));
        l->rolling_variance = calloc(l->n, sizeof(float));
    }
}

void make_maxpool_layer(layer *l, int num_channel_out, int kernel_size,
                        int stride, int pad, int num_channel_in, int size_in, int size_out)
{
    l->type = MAXPOOL;
    l->size_in = size_in;
    l->size_out = size_out;
    l->n = num_channel_out;
    l->size = kernel_size;
    l->stride = stride;
    l->pad = pad;
    l->c = num_channel_in;
    l->forward = forward_max_layer;
    l->output = calloc(l->size_out * l->size_out * l->n, sizeof(float));
}

void make_region_layer(layer *l, float thresh)
{
    l->type = REGION;
    l->thresh = thresh;
    l->forward = forward_region_layer;
    // l->workspace_size = 0;
    l->biases = calloc(10, sizeof(float));
    l->biases[0] = 1.08;
    l->biases[1] = 1.19;

    l->biases[2] = 3.42;
    l->biases[3] = 4.41;

    l->biases[4] = 6.63;
    l->biases[5] = 11.38;

    l->biases[6] = 9.42;
    l->biases[7] = 5.11;

    l->biases[8] = 16.62;
    l->biases[9] = 10.52;
}

void free_conv_layer(layer *l)
{
    free(l->biases);
    free(l->weights);
    free(l->output);
    if (l->batch_normalize)
    {
        free(l->scales);
        free(l->rolling_mean);
        free(l->rolling_variance);
    }
}

void free_region_layer(layer *l)
{
    free(l->biases);
}

void free_maxpool_layer(layer *l)
{
    free(l->output);
}

void load_conv_weights(layer *l, FILE *fp)
{
    int num = l->c * l->n * l->size * l->size;

    fread(l->biases, sizeof(float), l->n, fp);
    fread(l->weights, sizeof(float), num, fp);
    if (l->batch_normalize)
    {
        fread(l->scales, sizeof(float), l->n, fp);
        fread(l->rolling_mean, sizeof(float), l->n, fp);
        fread(l->rolling_variance, sizeof(float), l->n, fp);
    }
}

void load_connected_weights(layer *l, FILE *fp)
{
}

void load_weights(network *net, char *weights)
{
    fprintf(stderr, "Loading weights from %s... \n", weights);
    fflush(stdout);
    FILE *weights_fp = fopen(weights, "rb");
    if (!weights_fp)
    {
        fprintf(stderr, "cannot open file %s", weights);
        exit(1);
    }

    int i;
    for (i = 0; i < net->n; ++i)
    {
        layer l = net->layers[i];
        if (l.type == CONVOLUTION)
        {
            // fprintf(stderr, "loading conv layer weights\n");
            load_conv_weights(&l, weights_fp);
            fprintf(stderr, "finish loading conv layer weights\n");
        }
        else if (l.type == CONNECTED)
        {
            fprintf(stderr, "loading connected layer weights\n");
            load_connected_weights(&l, weights_fp);
        }
        else
        {
            fprintf(stderr, "load layer weight not implement\n");
        }
    }
}

network *make_network()
{
    // size_t workspace_size = 0;
    network *net = calloc(1, sizeof(network));
    net->n = LAYERNUM;
    net->layers = calloc(net->n, sizeof(layer));
    net->channels = 3;
    net->height = 416;
    net->width = 416;
    net->input = 0;
    net->output = 0;

    // set basic structure
    /*
    layer     filters    size              input                output
    0 conv     16  3 x 3 / 1   416 x 416 x   3   ->   416 x 416 x  16
    1 max          2 x 2 / 2   416 x 416 x  16   ->   208 x 208 x  16
    2 conv     32  3 x 3 / 1   208 x 208 x  16   ->   208 x 208 x  32
    3 max          2 x 2 / 2   208 x 208 x  32   ->   104 x 104 x  32
    4 conv     64  3 x 3 / 1   104 x 104 x  32   ->   104 x 104 x  64
    5 max          2 x 2 / 2   104 x 104 x  64   ->    52 x  52 x  64
    6 conv    128  3 x 3 / 1    52 x  52 x  64   ->    52 x  52 x 128
    7 max          2 x 2 / 2    52 x  52 x 128   ->    26 x  26 x 128
    8 conv    256  3 x 3 / 1    26 x  26 x 128   ->    26 x  26 x 256
    9 max          2 x 2 / 2    26 x  26 x 256   ->    13 x  13 x 256
   10 conv    512  3 x 3 / 1    13 x  13 x 256   ->    13 x  13 x 512
   11 max          2 x 2 / 1    13 x  13 x 512   ->    13 x  13 x 512
   12 conv   1024  3 x 3 / 1    13 x  13 x 512   ->    13 x  13 x1024
   13 conv   1024  3 x 3 / 1    13 x  13 x1024   ->    13 x  13 x1024
   14 conv    125  1 x 1 / 1    13 x  13 x1024   ->    13 x  13 x 125
   15 detection
    */

    make_conv_layer(&(net->layers[0]), 16, 3, 1, 1, 3, LEAKY, 1, 416, 416);

    make_maxpool_layer(&(net->layers[1]), 16, 2, 2, 1, 16, 416, 208);

    make_conv_layer(&(net->layers[2]), 32, 3, 1, 1, 16, LEAKY, 1, 208, 208);

    make_maxpool_layer(&(net->layers[3]), 32, 2, 2, 1, 32, 208, 104);

    make_conv_layer(&(net->layers[4]), 64, 3, 1, 1, 32, LEAKY, 1, 104, 104);

    make_maxpool_layer(&(net->layers[5]), 64, 2, 2, 1, 64, 104, 52);

    make_conv_layer(&(net->layers[6]), 128, 3, 1, 1, 64, LEAKY, 1, 52, 52);

    make_maxpool_layer(&(net->layers[7]), 128, 2, 2, 1, 128, 52, 26);

    make_conv_layer(&(net->layers[8]), 256, 3, 1, 1, 128, LEAKY, 1, 26, 26);

    make_maxpool_layer(&(net->layers[9]), 256, 2, 2, 1, 256, 26, 13);

    make_conv_layer(&(net->layers[10]), 512, 3, 1, 1, 256, LEAKY, 1, 13, 13);

    make_maxpool_layer(&(net->layers[11]), 512, 2, 1, 1, 512, 13, 13);

    make_conv_layer(&(net->layers[12]), 1024, 3, 1, 1, 512, LEAKY, 1, 13, 13);

    make_conv_layer(&(net->layers[13]), 1024, 3, 1, 1, 1024, LEAKY, 1, 13, 13);

    make_conv_layer(&(net->layers[14]), 125, 1, 1, 1, 1024, LINEAR, 0, 13, 13);

    // detection layer
    make_region_layer(&(net->layers[15]), 0.6);

    // for (int i=0; i<net->n; ++i){
    //     if (workspace_size<net->layers[i].workspace_size){
    //         workspace_size = net->layers[i].workspace_size;
    //     }
    // }
    // net->workspace = calloc(1, workspace_size);

    return net;
}

void free_layer(layer *l)
{
    if (l->type == CONVOLUTION)
    {
        free_conv_layer(l);
    }
    else if (l->type == REGION)
    {
        free_region_layer(l);
    }
    else if (l->type == MAXPOOL)
    {
        free_maxpool_layer(l);
    }
    else
    {
        fprintf(stderr, "free not implemented for this layer type");
        // exit(0);
    }
}

void free_network(network *net)
{
    int i;
    for (i = 0; i < net->n; i++)
    {
        free_layer(&(net->layers[i]));
    }
    free(net->layers);
    // free(net->workspace);
    free(net);
}

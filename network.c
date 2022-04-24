#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mynet.h"

#define LAYERNUM 16
#define FLT_MAX  10000

#define QUANTIZE 0
#define PRINT_WEIGHT 0
#define PRINT_BNM 0

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
    fprintf(stderr, "doing forward batchnorm\n");
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
static inline float logistic_activate(float x){return 1./(1. + exp(-x));}


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
    else if (a==LOGISTIC){
        for(int i=0; i<array_len; ++i){
            output[i] = logistic_activate(output[i]);
        }
    }
    else{
        fprintf(stderr, "unimplemented activation\n");
    }
}

void forward_max_layer(layer l, network net)
{
    fprintf(stderr, "doing forward maxpool\n");
    int size_out = l.size_out;
    int input_channel = l.c;
    int filter_size = l.size;
    int size_in = l.size_in;
    int stride = l.stride;

    for(int ic = 0; ic < input_channel; ++ic){
        int channel_offset_out = ic * size_out * size_out;
        int channel_offset_in = ic * size_in * size_in;
        for(int r = 0; r < size_out; ++r){
            int row_offset = r * size_out;
            for(int c = 0; c < size_out; ++c){
                int col_offset = c;
                int out_index = col_offset + row_offset + channel_offset_out;
                float max = -FLT_MAX;
                // for each row in filter
                for (int h = 0; h<filter_size; ++h){
                    int h_input = r*stride + h;
                    // for each col in filter
                    for (int w = 0; w<filter_size; ++w) {
                        int w_input = c*stride + w;
                        int index_in = w_input + h_input*size_in + channel_offset_in;
                        int valid = (h_input >= 0 && h_input < size_in &&
                                     w_input >= 0 && w_input < size_in);
                        float input_val = (valid != 0) ? net.input[index_in] : -FLT_MAX;
                        max = (input_val > max) ? input_val : max;
                    }
                }
                l.output[out_index] = max;
            }
        }
    }
}


int entry_index(layer l, int location, int entry)
{
    int n =   location / (l.size_out*l.size_out);
    int loc = location % (l.size_out*l.size_out);
    return n*l.size_out*l.size_out*(l.coords+l.classes+1) + entry*l.size_out*l.size_out + loc;
}

void softmax(float *input, int n, float temp, int stride, float *output)
{
    int i;
    float sum = 0;
    float largest = -FLT_MAX;
    for(i = 0; i < n; ++i){
        if(input[i*stride] > largest) largest = input[i*stride];
    }
    for(i = 0; i < n; ++i){
        float e = exp(input[i*stride]/temp - largest/temp);
        sum += e;
        output[i*stride] = e;
    }
    for(i = 0; i < n; ++i){
        output[i*stride] /= sum;
    }
}

void softmax_cpu(float *input, int n, int batch, int batch_offset, int groups, int group_offset, int stride, float temp, float *output)
{
    int g, b;
    for(b = 0; b < batch; ++b){
        for(g = 0; g < groups; ++g){
            softmax(input + b*batch_offset + g*group_offset, n, temp, stride, output + b*batch_offset + g*group_offset);
        }
    }
}

void forward_region_layer(layer l, network net)
{
    fprintf(stderr, "doing forward region");

    int index;
    int size_in = l.size_in;

    memcpy(l.output, net.input, l.size_out*l.size_out*l.n*sizeof(float));
    for(int num = 0; num<l.num; ++num){
        // l->n = l->num*(classes + coords + 1);
        //int num_offset = num*l.size_out*l.size_out*(l.coords+l.classes+1);
        index = entry_index(l, num*l.size_out*l.size_out, 0);
        activate_array(l.output + index, 2*l.size_out*l.size_out, LOGISTIC);
        index = entry_index(l, num*l.size_out*l.size_out, l.coords);
        activate_array(l.output + index, l.size_out*l.size_out, LOGISTIC);
    }
    index = entry_index(l, 0, l.coords + 1);
    //softmax_cpu(net.input + index, l.classes + l.background, l.batch*l.n, l.inputs/l.n, l.w*l.h, 1, l.w*l.h, 1, l.output + index);
    softmax_cpu(net.input + index, l.classes, l.n, size_in*size_in, size_in*size_in, 1, size_in*size_in, 1, l.output + index);



}

void forward_conv_layer(layer l, network net)
{
    if (l.type != CONVOLUTION)
    {
        fprintf(stderr, "non conv layer calls forward_conv_layer\n");
        exit(1);
    }
    fprintf(stderr, "doing forward conv...\n");

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
        output_channel_offset_kernel = oc * ksize * ksize * input_channel;
        output_channel_offset_out = oc * output_size * output_size;
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
                            //fprintf(stderr, "       update output value\n");
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

void make_region_layer(layer *l, float thresh, int num, int size_in, int classes, int coords)
{
    l->type = REGION;
    l->thresh = thresh;
    

    l->num = num;
    l->size_in = size_in;
    l->size_out = l->size_in;
    l->c = l->num*(classes + coords + 1);
    l->n = l->c;
    l->classes = classes;
    l->coords = coords;

    l->biases = calloc(num*2, sizeof(float));
    l->output = calloc(l->size_out*l->size_out*l->n, sizeof(float));

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

    l->forward = forward_region_layer;
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
    free(l->output);
}

void free_maxpool_layer(layer *l)
{
    free(l->output);
}

void load_conv_weights(layer *l, FILE *fp)
{
    FILE *out_fp;
    FILE *bnm_fp;
    FILE *bias_fp;

    if (PRINT_WEIGHT)
    {
        if (QUANTIZE)
            out_fp = fopen("mynet_quant_weight.txt", "a");
        else
        {
            out_fp = fopen("mynet_yolov2_tiny_voc_weight_23.txt", "a");
            bias_fp = fopen("mynet_bias_23.txt", "a");
        }
    }

    if (PRINT_BNM)
        bnm_fp = fopen("mynet_batch_norm_23.txt", "a");


    int num = l->c * l->n * l->size * l->size;
    
    fread(l->biases, sizeof(float), l->n, fp);
    if (PRINT_WEIGHT)
    {
        int k;
        fprintf(bias_fp, "\n");
        fprintf(bias_fp, " bias: %d\n", num);
        for (k = 0; k < l->n; ++k)
        {
            fprintf(bias_fp, "%g, ", l->biases[k]);
            if (k % 10 == 9)
                fprintf(bias_fp, "\n");
        }
        fprintf(bias_fp, "\n\n");
    }


    if (l->batch_normalize)
    {
        fread(l->scales, sizeof(float), l->n, fp);
        fread(l->rolling_mean, sizeof(float), l->n, fp);
        fread(l->rolling_variance, sizeof(float), l->n, fp);

        if (PRINT_BNM)
        {
            int i;
            fprintf(bnm_fp, "scale: %d\n", l->n);
            for (i = 0; i < l->n; ++i)
            {
                fprintf(bnm_fp, "%g, ", l->scales[i]);
                if (i % 10 == 9)
                    fprintf(bnm_fp, "\n");
            }
            fprintf(bnm_fp, "\n\n");

            fprintf(bnm_fp, "rolling_mean: %d\n", l->n);
            for (i = 0; i < l->n; ++i)
            {
                fprintf(bnm_fp, "%g, ", l->rolling_mean[i]);
                if (i % 10 == 9)
                    fprintf(bnm_fp, "\n");
            }
            fprintf(bnm_fp, "\n\n");

            fprintf(bnm_fp, "rolling_var: %d\n", l->n);
            for (i = 0; i < l->n; ++i)
            {
                fprintf(bnm_fp, "%g, ", l->rolling_variance[i]);
                if (i % 10 == 9)
                    fprintf(bnm_fp, "\n");
            }
            fprintf(bnm_fp, "\n\n");
        }
    }

    fread(l->weights, sizeof(float), num, fp);
    if (PRINT_WEIGHT)
    {
        int k;
        fprintf(out_fp, "conv weights: %d\n", num);
        for (k = 0; k < num; ++k)
        {
            fprintf(out_fp, "%g, ", l->weights[k]);
            if (k % 10 == 9)
                fprintf(out_fp, "\n");
        }
        fprintf(out_fp, "\n\n");
    }

    if (PRINT_WEIGHT){
        fclose(out_fp);
        fclose(bias_fp);
    }

    if (PRINT_BNM)
        fclose(bnm_fp);

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
        else if (l.type == MAXPOOL)
        {
            fprintf(stderr, "no loading needed for maxpool layer\n");
        }
        else if (l.type == REGION)
        {
            fprintf(stderr, "no loading needed for region layer\n");
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
    // make_region_layer(layer *l, float thresh, int num, int size_in, int classes, int coords)
    make_region_layer(&(net->layers[15]), 0.6, 5, 13, 20, 4);

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

box get_region_box(float *x, float *biases, int n, int index, int i, int j, int w, int h, int stride)
{
    box b;
    b.x = (i + x[index + 0*stride]) / w;
    b.y = (j + x[index + 1*stride]) / h;
    b.w = exp(x[index + 2*stride]) * biases[2*n]   / w;
    b.h = exp(x[index + 3*stride]) * biases[2*n+1] / h;
    return b;
}

void get_region_detections(layer l, int w, int h, int netw, int neth, float thresh,
                            int *map, float tree_thresh, int relative, detection *dets)
{
    int i,j,n;
    float *predictions = l.output;
    for (i = 0; i < l.size_in*l.size_in; ++i){
        int row = i / l.size_in;
        int col = i % l.size_in;
        for(n = 0; n < l.num; ++n){
            int index = n*l.size_in*l.size_in + i;
            for(j = 0; j < l.classes; ++j){
                dets[index].prob[j] = 0;
            }
            int obj_index  = entry_index(l, n*l.size_in*l.size_in + i, l.coords);
            int box_index  = entry_index(l, n*l.size_in*l.size_in + i, 0);
            // int mask_index = entry_index(l, n*l.size_in*l.size_in + i, 4);
            float scale = predictions[obj_index];
            dets[index].bbox = get_region_box(predictions, l.biases, n, box_index, col, row, l.size_in, l.size_in, l.size_in*l.size_in);
            dets[index].objectness = scale > thresh ? scale : 0;
            
            if(dets[index].objectness){
                for(j = 0; j < l.classes; ++j){
                    int class_index = entry_index(l, l.size_in*l.size_in + i, l.coords + 1 + j);
                    float prob = scale*predictions[class_index];
                    dets[index].prob[j] = (prob > thresh) ? prob : 0;
                }
            }
            
        }
    }
}


//detection boxes

void fill_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, detection *dets)
{
    int j;
    for(j = 0; j < net->n; ++j){
        layer l = net->layers[j];
        if(l.type == REGION){
            get_region_detections(l, w, h, net->width, net->height, thresh, map, hier, relative, dets);
            // dets += l.size_in*l.size_in*l.num;
        }
    }
}

int num_detections(network *net, float thresh) {
    int i;
    int s = 0;
    for(i = 0; i < net->n; ++i){
        layer l = net->layers[i];
        if(l.type == REGION){
            s += l.size_out*l.size_out*l.num;
        }
    }
    return s;
}

detection *make_network_boxes(network *net, float thresh, int *num)
{
    layer l = net->layers[net->n - 1];
    int i;
    int nboxes = num_detections(net, thresh);
    if(num) *num = nboxes;
    detection *dets = calloc(nboxes, sizeof(detection));
    for(i = 0; i < nboxes; ++i){
        dets[i].prob = calloc(l.classes, sizeof(float));
        if(l.coords > 4){
            dets[i].mask = calloc(l.coords-4, sizeof(float));
        }
    }
    return dets;
}

// detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
detection *get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num)
{
    detection *dets = make_network_boxes(net, thresh, num);
    fill_network_boxes(net, w, h, thresh, hier, map, relative, dets);
    return dets;
}

void free_detections(detection *dets, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(dets[i].prob);
        if(dets[i].mask) free(dets[i].mask);
    }
    free(dets);
}
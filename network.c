#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mynet.h"

#define FIND_RANGE 0
#define FIND_OUT_RANGE 0

float full_precision_dequantize(int xq, float amax, int bitnum)
{
    int out_max = (pow(2, (bitnum - 1))-1);
    float x_dq = xq / (float)out_max * amax;
    return x_dq;
}

float int8_dequantize(int8_t xq, float amax, int bitnum)
{
    int out_max = (pow(2, (bitnum - 1))-1);
    float x_dq = xq / (float)out_max * amax;
    return x_dq;
}

int8_t int8_quantize(float x, float amax, int bitnum)
{
    int8_t xq;
    int out_max;
    int out_min; 
    int tmp;
    float x_dq;

    out_max = (pow(2, (bitnum - 1))-1);
    out_min = (-(pow(2, (bitnum - 1))));
    // fprintf(stderr, "out_max = %" PRIi8 "\n", out_max);
    // float scale = 2*amax/pow(2, bitnum);
    tmp = round(x * out_max / amax);
    tmp = (tmp > out_max) ? out_max : tmp;
    tmp = (tmp < out_min) ? out_min : tmp;
    xq =  (int8_t) tmp;
    // fprintf(stderr, "quantized x = %" PRIi8 "\n", xq);
    x_dq = xq / (float)out_max * amax;
    // fprintf(stderr, "dq x = %g\n", x_dq);
    return xq;
}

float quantize(float x, float amax, int bitnum)
{
    int xq;
    int out_max;
    float x_dq;
    out_max = pow(2, (bitnum - 1));
    // float scale = 2*amax/pow(2, bitnum);
    xq = round(x * out_max / amax);
    xq = (xq > out_max) ? out_max : xq;
    xq = (xq < -out_max) ? -out_max : xq;

    x_dq = xq / (float)out_max * amax;
    return x_dq;
}

void quantize_array(float *output, int8_t *int8_output, int array_len, float amax, int bitnum)
{
    for (int i = 0; i < array_len; ++i)
        {
            int8_output[i] = int8_quantize(output[i], amax, bitnum);
        }
}

void sudo_quantize_array(float *output, int array_len, float amax, int bitnum)
{
    for (int i = 0; i < array_len; ++i)
        {
            output[i] = quantize(output[i], amax, bitnum);
        }
}

void dequantize_array(int8_t *int8_output, float *output,  int array_len, float amax, int bitnum)
{
    for (int i = 0; i < array_len; ++i)
        {
            output[i] = int8_dequantize(int8_output[i], amax, bitnum);
        }
}

void full_precision_dequantize_array(int *full_precision_output, float *output,  int array_len, float amax, int bitnum)
{
    for (int i = 0; i < array_len; ++i)
        {
            output[i] = full_precision_dequantize(full_precision_output[i], amax, bitnum);
        }
}

void print_range_array(float *output, int array_len){
    float max_out = -100000;
    float min_out = 100000;
    float out;
    for (int i=0; i <array_len; ++i){
        out = output[i];
        if (max_out < out)
            max_out = out;
        if (min_out > out)
            min_out = out;
    }
    fprintf(stderr, "max_out = %g ", max_out);
    fprintf(stderr, "min_out = %g\n", min_out);
}

// correct
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

int8_t int8_get_input_pixel(int row, int col, int channel, int kernel_row, int kernel_col, int pad, int image_size, int8_t *image)
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

void int8_scale_bias(int *output, int8_t *scales, int out_channel, int size_out)
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
                output[channel_offset + row_offset + col_offset] *= (int) scales[oc];
            }
        }
    }
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

void int8_add_bias(int *output, int8_t *biases, int out_channel, int size_out)
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
                output[channel_offset + row_offset + col_offset] += (int)biases[oc];
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

void int8_forward_batchnorm(int *output, int8_t *rolling_mean, int8_t *rolling_variance, int out_channel, int size_out)
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
                output[index] = (output[index] - (int)rolling_mean[oc]) / (sqrt((float)rolling_variance[oc]) + .000001f);
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

static inline float leaky_activate(float x) { return (x > 0) ? x : 0.125 * x; }
static inline float linear_activate(float x) { return x; }
static inline float logistic_activate(float x) { return 1. / (1. + exp(-x)); }

void activate_array(float *output, int array_len, ACTIVATION a)
{
    if (a == LINEAR)
    {
        for (int i = 0; i < array_len; ++i)
        {
            output[i] = linear_activate(output[i]);
        }
    }
    else if (a == LEAKY)
    {
        for (int i = 0; i < array_len; ++i)
        {
            output[i] = leaky_activate(output[i]);
        }
    }
    else if (a == LOGISTIC)
    {
        for (int i = 0; i < array_len; ++i)
        {
            output[i] = logistic_activate(output[i]);
        }
    }
    else
    {
        fprintf(stderr, "unimplemented activation\n");
    }
}

void int8_forward_max_layer(layer l, network net)
{
    fprintf(stderr, "doing int8 forward maxpool\n");
    int size_out = l.size_out;
    int input_channel = l.c;
    int filter_size = l.size;
    int size_in = l.size_in;
    int stride = l.stride;

    for (int ic = 0; ic < input_channel; ++ic)
    {
        int channel_offset_out = ic * size_out * size_out;
        int channel_offset_in = ic * size_in * size_in;
        for (int r = 0; r < size_out; ++r)
        {
            int row_offset = r * size_out;
            for (int c = 0; c < size_out; ++c)
            {
                int col_offset = c;
                int out_index = col_offset + row_offset + channel_offset_out;
                float max = -FLT_MAX;
                // for each row in filter
                for (int h = 0; h < filter_size; ++h)
                {
                    int h_input = r * stride + h;
                    // for each col in filter
                    for (int w = 0; w < filter_size; ++w)
                    {
                        int w_input = c * stride + w;
                        int index_in = w_input + h_input * size_in + channel_offset_in;
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
    // fprintf(stderr, "forward max finish\n");
}

void forward_max_layer(layer l, network net)
{
    fprintf(stderr, "doing forward maxpool\n");
    int size_out = l.size_out;
    int input_channel = l.c;
    int filter_size = l.size;
    int size_in = l.size_in;
    int stride = l.stride;

for (int ic = 0; ic < input_channel; ++ic)
{
    int channel_offset_out = ic * size_out * size_out;
    int channel_offset_in = ic * size_in * size_in;
    for (int r = 0; r < size_out; ++r)
    {
        int row_offset = r * size_out;
        for (int c = 0; c < size_out; ++c)
        {
            int col_offset = c;
            int out_index = col_offset + row_offset + channel_offset_out;
            float max = -FLT_MAX;
            // for each row in filter
            for (int h = 0; h < filter_size; ++h)
            {
                int h_input = r * stride + h;
                // for each col in filter
                for (int w = 0; w < filter_size; ++w)
                {
                    int w_input = c * stride + w;
                    int index_in = w_input + h_input * size_in + channel_offset_in;
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
    // fprintf(stderr, "forward max finish\n");
}

int entry_index(layer l, int location, int entry)
{
    int n = location / (l.size_out * l.size_out);
    int loc = location % (l.size_out * l.size_out);
    return n * l.size_out * l.size_out * (l.coords + l.classes + 1) + entry * l.size_out * l.size_out + loc;
}

void softmax(float *input, int n, float temp, int stride, float *output)
{
    int i;
    float sum = 0;
    float largest = -FLT_MAX;
    for (i = 0; i < n; ++i)
    {
        if (input[i * stride] > largest)
            largest = input[i * stride];
    }
    for (i = 0; i < n; ++i)
    {
        float e = exp(input[i * stride] / temp - largest / temp);
        sum += e;
        output[i * stride] = e;
    }
    for (i = 0; i < n; ++i)
    {
        output[i * stride] /= sum;
    }
}

void softmax_cpu(float *input, int n, int batch, int batch_offset, int groups, int group_offset, int stride, float temp, float *output)
{
    int g, b;
    for (b = 0; b < batch; ++b)
    {
        for (g = 0; g < groups; ++g)
        {
            softmax(input + b * batch_offset + g * group_offset, n, temp, stride, output + b * batch_offset + g * group_offset);
        }
    }
}

void forward_region_layer(layer l, network net)
{
    fprintf(stderr, "doing forward region\n");

    int index;
    int size_in = l.size_in;

    memcpy(l.output, net.input, l.size_out * l.size_out * l.n * sizeof(float));
    for (int num = 0; num < l.num; ++num)
    {
        index = entry_index(l, num * l.size_out * l.size_out, 0);
        activate_array(l.output + index, 2 * l.size_out * l.size_out, LOGISTIC);
        index = entry_index(l, num * l.size_out * l.size_out, l.coords);
        activate_array(l.output + index, l.size_out * l.size_out, LOGISTIC);
    }
    index = entry_index(l, 0, l.coords + 1);
    softmax_cpu(net.input + index, l.classes, l.num, size_in * size_in * l.c / l.num, size_in * size_in, 1, size_in * size_in, 1, l.output + index);
}

void int8_forward_conv_layer(layer l, network net)
{
    if (l.type != CONVOLUTION)
    {
        fprintf(stderr, "non conv layer calls forward conv layer\n");
        exit(1);
    }
    fprintf(stderr, "doing int8 forward conv...\n");

    int pad = l.pad;
    int ksize = l.size;
    int input_size = l.size_in;
    int input_channel = l.c;
    int output_size = l.size_out;
    int output_channel = l.n;

    // int8_t *int8_input = net.int8_input;

    int8_t *int8_out = l.int8_output;
    int8_t *int8_weights = l.int8_weights;
    int8_t *int8_input = net.int8_input;
    int *inter_out = l.intermediate_output;


    int output_channel_offset_kernel;
    int input_channel_offset_kernel;
    int row_offset_kernel;
    int col_offset_kernel;

    int output_channel_offset_out;
    int row_offset_out;
    int col_offset_out;

    // convolution
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
                        row_offset_kernel = i * ksize;
                        // for col in kernel
                        for (int j = 0; j < ksize; ++j)
                        {
                            col_offset_kernel = j;
                            int8_t int8_kernel_value = int8_weights[col_offset_kernel +
                                                         row_offset_kernel +
                                                         input_channel_offset_kernel +
                                                         output_channel_offset_kernel];
                            int8_t int8_image_value = int8_get_input_pixel(r, c, ic, i, j, pad, input_size, int8_input);
                            inter_out[col_offset_out +
                                row_offset_out +
                                output_channel_offset_out] += (((int)int8_kernel_value) * ((int)int8_image_value));
                        }
                    }
                }
            }
        }
    }

    // dequantize
    float input_range;
    if(l.index==0){
        input_range = 1;
    }
    else{
        layer prev_l = net.layers[l.index-1];
        input_range = prev_l.amax_out;
    }

    if (l.batch_normalize)
    {
        //todo change function to int8
        int8_forward_batchnorm(inter_out, l.int8_rolling_mean, l.int8_rolling_variance, l.n, l.size_out);
        //todo change function to int8
        int8_scale_bias(inter_out, l.int8_scales, l.n, l.size_out);
        //todo change function to int8
        int8_add_bias(inter_out, l.int8_biases, l.n, l.size_out);
    }

    full_precision_dequantize_array(inter_out, l.output, l.size_out * l.size_out * l.n, l.amax_w*input_range, 8+8-1);
   
    quantize_array(l.output, int8_out, l.size_out * l.size_out * l.n, l.amax_out, 8);

    // fprintf(stderr, "finish final quantize\n");
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
    int ksize = l.size;
    int input_size = l.size_in;
    int input_channel = l.c;
    int output_size = l.size_out;
    int output_channel = l.n;

    float *out = l.output;
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
                        row_offset_kernel = i * ksize;
                        // for col in kernel
                        for (int j = 0; j < ksize; ++j)
                        {
                            col_offset_kernel = j;
                            float kernel_value = weights[col_offset_kernel +
                                                         row_offset_kernel +
                                                         input_channel_offset_kernel +
                                                         output_channel_offset_kernel];
                            float image_value = get_input_pixel(r, c, ic, i, j, pad, input_size, input);
                            out[col_offset_out +
                                row_offset_out +
                                output_channel_offset_out] += (kernel_value * image_value);
                        }
                    }
                }
            }
        }
    }
    // if(l.index == 0){
    //     for (int oc = 0; oc < l.n; ++oc)
    //         {
    //             fprintf(stderr, "*************** oc: %d ***************\n", oc);
    //             for (int r = 0; r < 10; ++r)
    //             {
    //                 for (int c = 0; c < 10; ++c)
    //                 {
    //                     fprintf(stderr, "%g, ", out[oc * l.size_out * l.size_out + r * l.size_out + c]);
    //                 }
    //                 fprintf(stderr, "\n");
    //             }
    //         }
    // }

    // print_range_array(out, l.size_out*l.size_out*l.n);

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
    if(FIND_OUT_RANGE){
        fprintf(stderr, "layer %d\n", l.index);
        print_range_array(l.output, l.size_out * l.size_out * l.n);
    }
    if(l.quantize)
        sudo_quantize_array(l.output, l.size_out * l.size_out * l.n, l.amax_out, 8);
}

void make_conv_layer(layer *l, int index, int num_kernel, int kernel_size, int stride, int pad,
                     int num_channel_in, ACTIVATION act, int batch, int size_in, int size_out)
{
    int num;
    l->index = index;
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
    
    l->forward = forward_conv_layer;
    l->int8_forward = int8_forward_conv_layer;

    num = l->c * l->n * l->size * l->size;
    l->biases = calloc(l->n, sizeof(float));
    l->weights = calloc(num, sizeof(float));

    l->int8_biases = calloc(l->n, sizeof(int8_t));
    l->int8_weights = calloc(num, sizeof(int8_t));

    l->output = calloc(l->size_out * l->size_out * l->n, sizeof(float));
    l->int8_output = calloc(l->size_out * l->size_out * l->n, sizeof(int8_t));
    l->intermediate_output = calloc(l->size_out * l->size_out * l->n, sizeof(int));

    if (l->batch_normalize)
    {
        l->scales = calloc(l->n, sizeof(float));
        l->rolling_mean = calloc(l->n, sizeof(float));
        l->rolling_variance = calloc(l->n, sizeof(float));

        l->int8_scales = calloc(l->n, sizeof(int8_t));
        l->int8_rolling_mean = calloc(l->n, sizeof(int8_t));
        l->int8_rolling_variance = calloc(l->n, sizeof(int8_t));
    }

}

void make_maxpool_layer(layer *l, int index, int num_channel_out, int kernel_size,
                        int stride, int pad, int num_channel_in, int size_in, int size_out)
{
    l->index = index;
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
    l->int8_output = calloc(l->size_out * l->size_out * l->n, sizeof(int8_t));

}

void make_region_layer(layer *l, int index, float thresh, int num, int size_in, int classes, int coords)
{
    l->index = index;
    l->type = REGION;
    l->thresh = thresh;

    l->num = num;
    l->size_in = size_in;
    l->size_out = l->size_in;
    l->c = l->num * (classes + coords + 1);
    l->n = l->c;
    l->classes = classes;
    l->coords = coords;

    l->biases = calloc(num * 2, sizeof(float));
    l->output = calloc(l->size_out * l->size_out * l->n, sizeof(float));

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
    srand(0);
}

void free_conv_layer(layer *l)
{
    free(l->biases);
    free(l->weights);
    free(l->output);
    free(l->int8_biases);
    free(l->int8_weights);
    free(l->int8_output);
    if (l->batch_normalize)
    {
        free(l->scales);
        free(l->rolling_mean);
        free(l->rolling_variance);
        free(l->int8_scales);
        free(l->int8_rolling_mean);
        free(l->int8_rolling_variance);
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
    float max_w = -10;
    float min_w = 10;
    float max_mean = -10;
    float min_mean = 10;
    float max_variance = -10;
    float min_variance = 10;
    float max_scale = -10;
    float min_scale = 10;
    int num = l->c * l->n * l->size * l->size;

    fread(l->biases, sizeof(float), l->n, fp);

    if (l->batch_normalize)
    {
        fread(l->scales, sizeof(float), l->n, fp);
        fread(l->rolling_mean, sizeof(float), l->n, fp);
        fread(l->rolling_variance, sizeof(float), l->n, fp);
    }

    fread(l->weights, sizeof(float), num, fp);

    if (l->quantize){
        // quantize weight
        if (FULL_INT8)
            quantize_array(l->weights, l->int8_weights, num, l->amax_w, 8);
        else
            sudo_quantize_array(l->weights, num, l->amax_w, 8);

        if (l->batch_normalize)
        {
            if (FULL_INT8) {
                quantize_array(l->rolling_mean, l->int8_rolling_mean, l->n, l->amax_m, 8);
                quantize_array(l->rolling_variance, l->int8_rolling_variance, l->n, l->amax_var, 8);
                quantize_array(l->scales, l->int8_scales, l->n, l->amax_scale, 8);
            }
            else{
                sudo_quantize_array(l->rolling_mean, l->n, l->amax_m, 8);
                sudo_quantize_array(l->rolling_variance, l->n, l->amax_var, 8);
                sudo_quantize_array(l->scales, l->n, l->amax_scale, 8);
            }
        }
    }

    if (FIND_RANGE)
    {
        // find max min w
        for (int i = 0; i < num; ++i)
        {
            if (max_w < l->weights[i])
            {
                max_w = l->weights[i];
            }
            if (min_w > l->weights[i])
            {
                min_w = l->weights[i];
            }
        }
        fprintf(stderr, "max_w: %g, min_w: %g\n", max_w, min_w);

        if (l->batch_normalize)
        {
            // find max min mean
            for (int i = 0; i < l->n; ++i)
            {
                if (max_mean < l->rolling_mean[i])
                {
                    max_mean = l->rolling_mean[i];
                }
                if (min_mean > l->rolling_mean[i])
                {
                    min_mean = l->rolling_mean[i];
                }
            }
            fprintf(stderr, "max_mean: %g, min_mean: %g\n", max_mean, min_mean);

            // find max min var
            for (int i = 0; i < l->n; ++i)
            {
                if (max_variance < l->rolling_variance[i])
                {
                    max_variance = l->rolling_variance[i];
                }
                if (min_variance > l->rolling_variance[i])
                {
                    min_variance = l->rolling_variance[i];
                }
            }
            fprintf(stderr, "max_variance: %g, min_variance: %g\n", max_variance, min_variance);

            // find max min scale
            for (int i = 0; i < l->n; ++i)
            {
                if (max_scale < l->scales[i])
                {
                    max_scale = l->scales[i];
                }
                if (min_scale > l->scales[i])
                {
                    min_scale = l->scales[i];
                }
            }
            fprintf(stderr, "max_scale: %g, min_scale: %g\n", max_scale, min_scale);
        }
        fprintf(stderr, "\n");
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
            fprintf(stderr, "loading %d conv layer weights\n", i);
            load_conv_weights(&l, weights_fp);
        }
        else if (l.type == CONNECTED)
        {
            fprintf(stderr, "loading connected layer weights\n");
            load_connected_weights(&l, weights_fp);
        }
        else if (l.type == MAXPOOL)
        {
            // fprintf(stderr, "no loading needed for maxpool layer\n");
        }
        else if (l.type == REGION)
        {
            // printf(stderr, "no loading needed for region layer\n");
        }
    }
}

network *make_network()
{
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
   15 region
    */

    net->layers[0].amax_w = 1;
    net->layers[0].amax_out = 60;
    net->layers[0].amax_m = 1;
    net->layers[0].amax_var = 0.2;
    net->layers[0].amax_scale = 6;
    net->layers[0].amax_out_raw = 5;
    net->layers[0].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[0]), 0, 16, 3, 1, 1, 3, LEAKY, 1, 416, 416);

    net->layers[1].amax_out = 60;
    make_maxpool_layer(&(net->layers[1]), 1, 16, 2, 2, 1, 16, 416, 208);

    net->layers[2].amax_w = 1;
    net->layers[2].amax_out = 30;   
    net->layers[2].amax_m = 10;
    net->layers[2].amax_var = 65;
    net->layers[2].amax_scale = 6;
    net->layers[2].amax_out_raw = 80;
    net->layers[2].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[2]), 2, 32, 3, 1, 1, 16, LEAKY, 1, 208, 208);

    net->layers[3].amax_out = 30; 
    make_maxpool_layer(&(net->layers[3]), 3, 32, 2, 2, 1, 32, 208, 104);

    net->layers[4].amax_w = 1;
    net->layers[4].amax_out = 20;
    net->layers[4].amax_m = 5;
    net->layers[4].amax_var = 10;
    net->layers[4].amax_scale = 5;
    net->layers[4].amax_out_raw = 30;
    net->layers[4].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[4]), 4, 64, 3, 1, 1, 32, LEAKY, 1, 104, 104);

    net->layers[5].amax_out = 20;
    make_maxpool_layer(&(net->layers[5]), 5, 64, 2, 2, 1, 64, 104, 52);

    net->layers[6].amax_w = 0.6;
    net->layers[6].amax_out = 20;
    net->layers[6].amax_m = 5;
    net->layers[6].amax_var = 10;
    net->layers[6].amax_scale = 5;
    net->layers[6].amax_out_raw = 25;
    net->layers[6].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[6]), 6, 128, 3, 1, 1, 64, LEAKY, 1, 52, 52);

    net->layers[7].amax_out = 20;
    make_maxpool_layer(&(net->layers[7]), 7, 128, 2, 2, 1, 128, 52, 26);

    net->layers[8].amax_w = 0.5;
    net->layers[8].amax_out = 15;
    net->layers[8].amax_m = 3;
    net->layers[8].amax_var = 5;
    net->layers[8].amax_scale = 3;
    net->layers[8].amax_out_raw = 15;
    net->layers[8].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[8]), 8, 256, 3, 1, 1, 128, LEAKY, 1, 26, 26);

    net->layers[9].amax_out = 15;
    make_maxpool_layer(&(net->layers[9]), 9, 256, 2, 2, 1, 256, 26, 13);

    net->layers[10].amax_w = 0.4;
    net->layers[10].amax_out = 11;
    net->layers[10].amax_m = 2;
    net->layers[10].amax_var = 3;
    net->layers[10].amax_scale = 3;
    net->layers[10].amax_out_raw = 10;
    net->layers[10].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[10]), 10, 512, 3, 1, 1, 256, LEAKY, 1, 13, 13);

    net->layers[11].amax_out = 11;
    make_maxpool_layer(&(net->layers[11]), 11, 512, 2, 1, 1, 512, 13, 13);

    net->layers[12].amax_w = 0.12;
    net->layers[12].amax_out = 10;
    net->layers[12].amax_m = 1;
    net->layers[12].amax_var = 3;
    net->layers[12].amax_scale = 10;
    net->layers[12].amax_out_raw = 5;
    net->layers[12].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[12]), 12, 1024, 3, 1, 1, 512, LEAKY, 1, 13, 13);

    net->layers[13].amax_w = 0.05;
    net->layers[13].amax_out = 10;
    net->layers[13].amax_m = 3;
    net->layers[13].amax_var = 17;
    net->layers[13].amax_scale = 1;
    net->layers[13].amax_out_raw = 25;
    net->layers[13].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[13]), 13, 1024, 3, 1, 1, 1024, LEAKY, 1, 13, 13);

    net->layers[14].amax_w = 0.25;
    net->layers[14].amax_out = 25;
    net->layers[14].amax_out_raw = 25;
    net->layers[14].quantize = QUANTIZE_ENABLE;
    make_conv_layer(&(net->layers[14]), 14, 125, 1, 1, 0, 1024, LINEAR, 0, 13, 13);

    make_region_layer(&(net->layers[15]), 15, 0.6, 5, 13, 20, 4);

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
    free(net);
}

box get_region_box(float *x, float *biases, int n, int index, int i, int j, int w, int h, int stride)
{
    box b;
    b.x = (i + x[index + 0 * stride]) / w;
    b.y = (j + x[index + 1 * stride]) / h;
    b.w = exp(x[index + 2 * stride]) * biases[2 * n] / w;
    b.h = exp(x[index + 3 * stride]) * biases[2 * n + 1] / h;
    return b;
}

void get_region_detections(layer l, int w, int h, int netw, int neth, float thresh,
                           int *map, float tree_thresh, int relative, detection *dets)
{
    int i, j, n;
    float *predictions = l.output;
    for (i = 0; i < l.size_in * l.size_in; ++i)
    {
        int row = i / l.size_in;
        int col = i % l.size_in;
        for (n = 0; n < l.num; ++n)
        {
            int index = n * l.size_in * l.size_in + i;
            for (j = 0; j < l.classes; ++j)
            {
                dets[index].prob[j] = 0;
            }
            int obj_index = entry_index(l, n * l.size_in * l.size_in + i, l.coords);
            int box_index = entry_index(l, n * l.size_in * l.size_in + i, 0);
            float scale = predictions[obj_index];
            dets[index].bbox = get_region_box(predictions, l.biases, n, box_index, col, row, l.size_in, l.size_in, l.size_in * l.size_in);
            dets[index].objectness = scale > thresh ? scale : 0;

            if (dets[index].objectness)
            {
                for (j = 0; j < l.classes; ++j)
                {
                    int class_index = entry_index(l, l.size_in * l.size_in + i, l.coords + 1 + j);
                    float prob = scale * predictions[class_index];
                    dets[index].prob[j] = (prob > thresh) ? prob : 0;
                }
            }
        }
    }
}

// detection boxes

void fill_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, detection *dets)
{
    int j;
    for (j = 0; j < net->n; ++j)
    {
        layer l = net->layers[j];
        if (l.type == REGION)
        {
            get_region_detections(l, w, h, net->width, net->height, thresh, map, hier, relative, dets);
        }
    }
}

int num_detections(network *net, float thresh)
{
    int i;
    int s = 0;
    for (i = 0; i < net->n; ++i)
    {
        layer l = net->layers[i];
        if (l.type == REGION)
        {
            s += l.size_out * l.size_out * l.num;
        }
    }
    return s;
}

detection *make_network_boxes(network *net, float thresh, int *num)
{
    layer l = net->layers[net->n - 1];
    int i;
    int nboxes = num_detections(net, thresh);
    if (num)
        *num = nboxes;
    detection *dets = calloc(nboxes, sizeof(detection));
    for (i = 0; i < nboxes; ++i)
    {
        dets[i].prob = calloc(l.classes, sizeof(float));
        if (l.coords > 4)
        {
            dets[i].mask = calloc(l.coords - 4, sizeof(float));
        }
    }
    return dets;
}

detection *get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num)
{
    detection *dets = make_network_boxes(net, thresh, num);
    fill_network_boxes(net, w, h, thresh, hier, map, relative, dets);
    return dets;
}

void free_detections(detection *dets, int n)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        free(dets[i].prob);
        if (dets[i].mask)
            free(dets[i].mask);
    }
    free(dets);
}
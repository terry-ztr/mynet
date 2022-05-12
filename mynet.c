#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mynet.h"


#define PRINT_INPUT 0
#define PRINT_BNM 0

char *fgetl(FILE *fp)
{
    if (feof(fp))
        return 0;
    size_t size = 512;
    char *line = malloc(size * sizeof(char));
    if (!fgets(line, size, fp))
    {
        free(line);
        return 0;
    }

    size_t curr = strlen(line);

    while ((line[curr - 1] != '\n') && !feof(fp))
    {
        if (curr == size - 1)
        {
            size *= 2;
            line = realloc(line, size * sizeof(char));
            if (!line)
            {
                printf("%ld\n", size);
                fprintf(stderr, "Malloc error\n");
                exit(-1);
            }
        }
        size_t readsize = size - curr;
        if (readsize > INT_MAX)
            readsize = INT_MAX - 1;
        fgets(&line[curr], readsize, fp);
        curr = strlen(line);
    }
    if (line[curr - 1] == '\n')
        line[curr - 1] = '\0';

    return line;
}

void **list_to_array(list *l)
{
    void **a = calloc(l->size, sizeof(void *));
    int count = 0;
    node *n = l->front;
    while (n)
    {
        a[count++] = n->val;
        n = n->next;
    }
    return a;
}

void file_error(char *s)
{
    fprintf(stderr, "Couldn't open file: %s\n", s);
    exit(0);
}

list *make_list()
{
    list *l = malloc(sizeof(list));
    l->size = 0;
    l->front = 0;
    l->back = 0;
    return l;
}

void free_node(node *n)
{
    node *next;
    while (n)
    {
        next = n->next;
        free(n);
        n = next;
    }
}

void list_insert(list *l, void *val)
{
    node *new = malloc(sizeof(node));
    new->val = val;
    new->next = 0;

    if (!l->back)
    {
        l->front = new;
        new->prev = 0;
    }
    else
    {
        l->back->next = new;
        new->prev = l->back;
    }
    l->back = new;
    ++l->size;
}

list *get_paths(char *filename)
{
    char *path;
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Couldn't open file: %s\n", filename);
        exit(0);
    }
    list *lines = make_list();
    while ((path = fgetl(file)))
    {
        list_insert(lines, path);
    }
    fclose(file);
    return lines;
}

void free_list(list *l)
{
    free_node(l->front);
    free(l);
}

char **get_labels(char *filename)
{
    list *plist = get_paths(filename);
    char **labels = (char **)list_to_array(plist);
    free_list(plist);
    return labels;
}

image **load_alphabet()
{
    int i, j;
    const int nsize = 8;
    image **alphabets = calloc(nsize, sizeof(image));
    for (j = 0; j < nsize; ++j)
    {
        alphabets[j] = calloc(128, sizeof(image));
        for (i = 32; i < 127; ++i)
        {
            char buff[256];
            sprintf(buff, "data/labels/%d_%d.png", i, j);
            alphabets[j][i] = load_image_color(buff, 0, 0);
        }
    }
    return alphabets;
}

int main(int argc, char **argv)
{
    // int8_t y = int8_quantize(-0.228, 0.06, 8);
    // float ydq = int8_dequantize(y, 0.06, 8);
    // fprintf(stderr, "ydq: %f\n", ydq);
    // return 0;

    if (argc < 2)
    {
        fprintf(stderr, "usage: give a filename\n");
        return 0;
    }

    int test_get_pixel = 0;
    if (test_get_pixel)
    {
        char *filename = argv[1];
        image im = load_image(filename, 416, 416, 3);
        for (int ic = 0; ic < 3; ++ic)
        {
            for (int kr = 0; kr < 3; ++kr)
            {
                for (int kc = 0; kc < 3; ++kc)
                {
                    float image_value = get_input_pixel(1, 1, ic, kr, kc, 1, 416, im.data);
                    fprintf(stderr, "%g, ", image_value);
                }
                fprintf(stderr, "\n");
            }
            fprintf(stderr, "****** ic: %d *****\n", ic);
        }
        return 1;
    }

    FILE *input_fp;
    // FILE *check_weight_fp;
    FILE *check_bnm_fp;

    if (PRINT_BNM)
        check_bnm_fp = fopen("mynet_batchnorm_layer_1.txt", "a");

    if (PRINT_INPUT)
        input_fp = fopen("my_dog_input.txt", "a");

    char *filename = argv[1];
    char *weights_file = "float_genv2_tiny_voc.weights";

    char *names[20] = {"aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"};

    // image **alphabet = load_alphabet();
    image **alphabet = NULL;

    // create network structure
    network *net = make_network();

    for (int i = 0; i < net->n; ++i)
    {
        layer ltmp = net->layers[i];
        if (ltmp.type == CONVOLUTION)
        {
            fprintf(stderr, "%d conv  %5d %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n",
                    i, ltmp.n, ltmp.size, ltmp.size, ltmp.stride, ltmp.size_in, ltmp.size_in, ltmp.c, ltmp.size_out, ltmp.size_out, ltmp.n);
        }
        else if (ltmp.type == MAXPOOL)
        {
            fprintf(stderr, "%d max         %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n",
                    i, ltmp.size, ltmp.size, ltmp.stride, ltmp.size_in, ltmp.size_in, ltmp.c, ltmp.size_out, ltmp.size_out, ltmp.n);
        }
        else if (ltmp.type == REGION)
        {
            fprintf(stderr, "%d reg\n", i);
            fprintf(stderr, "l.num: %d, l.classes: %d, l.coords: %d, l.size_in: %d, l.c: %d, l.w_out %d, l.out_c: %d\n", ltmp.num, ltmp.classes, ltmp.coords, ltmp.size_in, ltmp.c, ltmp.size_out, ltmp.n);

            for (int i = 0; i < 10; ++i)
            {
                fprintf(stderr, "bias[%d]: %g, ", i, ltmp.biases[i]);
            }
            fprintf(stderr, "\n");
        }
    }

    // load in weights
    load_weights(net, weights_file);

    // load image and resize it to the network input size

    image im = load_image(filename, net->width, net->height, net->channels);

    net->input = im.data;
    // quantize input image
    if(QUANTIZE_ENABLE)
    {
        // quantize input image
        if (FULL_INT8){
            net->int8_input = calloc(net->width*net->height*net->channels, sizeof(int8_t));
            quantize_array(net->input, net->int8_input, net->width*net->height*net->channels, 1, 8);
        }
        sudo_quantize_array(net->input, net->width*net->height*net->channels, 1, 8);
    }

    layer l;

    if (PRINT_INPUT)
    {
        for (int ic = 0; ic < 3; ++ic)
        {
            fprintf(input_fp, "**************** ic: %d ***************\n", ic);
            for (int r = 0; r < 3; ++r)
            {
                for (int c = 0; c < 3; ++c)
                {
                    fprintf(input_fp, "%g, ", net->input[ic * 416 * 416 + r * 416 + c]);
                }
                fprintf(input_fp, "\n");
            }
        }
    }
    if (PRINT_INPUT)
        fclose(input_fp);

    // inference
    for (int i = 0; i < net->n; ++i)
    {
        net->index = i;
        l = net->layers[i];
        if(FULL_INT8 && l.type == CONVOLUTION)
        {
            l.int8_forward(l, *net);
            net->int8_input = l.int8_output;
            net->input = l.output;
            // fprintf(stderr, "finish forward layer %d\n", i);
        }
        else
        {
            l.forward(l, *net);
            net->input = l.output;
            // fprintf(stderr, "finish forward layer %d\n", i);
        }
    }

    l = net->layers[15];
    if (l.type == MAXPOOL)
    {
        fprintf(stderr, "check output for MAXPOOL layer\n");
    }

    if (PRINT_OUTPUT)
    {
        for (int i = 0; i < net->n; ++i)
        {
            layer l = net->layers[i];
            char fp32_file[50], int8_file[50], sufix[10], num[10];
            strcpy(fp32_file, "../test_output/fp32_output_layer_");
            strcpy(int8_file, "../test_output/int8_output_layer_");
            strcpy(sufix, ".txt");
            sprintf(num, "%d", l.index);
            strcat(fp32_file, num);
            strcat(int8_file, num);
            strcat(fp32_file, sufix);
            strcat(int8_file, sufix);

            FILE *output_fp;
            if(QUANTIZE_ENABLE)
                output_fp= fopen(int8_file, "w");
            else
                output_fp = fopen(fp32_file, "w");
            if (!output_fp)
                file_error("output file");

            for (int oc = 0; oc < l.n; ++oc)
            {
                fprintf(output_fp, "*************** oc: %d ***************\n", oc);
                for (int r = 0; r < 10; ++r)
                {
                    for (int c = 0; c < 10; ++c)
                    {
                        fprintf(output_fp, "%g, ", l.output[oc * l.size_out * l.size_out + r * l.size_out + c]);
                    }
                    fprintf(output_fp, "\n");
                }
            }
            fclose(output_fp);
        }
    }


    // assume weight align (w, h, ic, oc) correct!!!
    if (PRINT_WEIGHT)
    {
        for (int i = 0; i < net->n; ++i)
        {
            layer l = net->layers[i];
            if(l.type != CONVOLUTION)
                continue;

            char fp32_file[50], int8_file[50], sufix[10], num[10];
            strcpy(fp32_file, "../test_weights/fp32_weight_layer_");
            strcpy(int8_file, "../test_weights/int8_weight_layer_");
            strcpy(sufix, ".txt");
            sprintf(num, "%d", l.index);
            strcat(fp32_file, num);
            strcat(int8_file, num);
            strcat(fp32_file, sufix);
            strcat(int8_file, sufix);

            FILE *weights_fp;
            if(l.quantize)
                weights_fp= fopen(int8_file, "w");
            else
                weights_fp = fopen(fp32_file, "w");

            if (!weights_fp)
                file_error("weight file");

            for (int oc = 0; oc < l.n; ++oc)
            {
                int output_channel_offset = oc * l.size * l.size * l.c;
                fprintf(weights_fp, "############### oc: %d ###############\n", oc);
                for (int ic = 0; ic < l.c; ++ic)
                {
                    int input_channel_offset = ic * l.size * l.size;
                    fprintf(weights_fp, "**************** ic: %d ***************\n", ic);
                    for (int r = 0; r < l.size; ++r)
                    {
                        for (int c = 0; c < l.size; ++c)
                        {
                            fprintf(weights_fp, "%g, ",
                                    l.weights[input_channel_offset + output_channel_offset + r * l.size + c]);
                        }
                        fprintf(weights_fp, "\n");
                    }
                }
            }
            fclose(weights_fp);
        }
    }

    // print out layer 0 batchnorm
    if (PRINT_BNM)
    {
        int i;
        fprintf(check_bnm_fp, "scale: %d\n", l.n);
        for (i = 0; i < l.n; ++i)
        {
            fprintf(check_bnm_fp, "%g, ", l.scales[i]);
            if (i % 10 == 9)
                fprintf(check_bnm_fp, "\n");
        }
        fprintf(check_bnm_fp, "\n\n");

        fprintf(check_bnm_fp, "rolling_mean: %d\n", l.n);
        for (i = 0; i < l.n; ++i)
        {
            fprintf(check_bnm_fp, "%g, ", l.rolling_mean[i]);
            if (i % 10 == 9)
                fprintf(check_bnm_fp, "\n");
        }
        fprintf(check_bnm_fp, "\n\n");

        fprintf(check_bnm_fp, "rolling_var: %d\n", l.n);
        for (i = 0; i < l.n; ++i)
        {
            fprintf(check_bnm_fp, "%g, ", l.rolling_variance[i]);
            if (i % 10 == 9)
                fprintf(check_bnm_fp, "\n");
        }
        fprintf(check_bnm_fp, "\n\n");
    }
    if (PRINT_BNM)
        fclose(check_bnm_fp);

    float thresh = 0.5;
    float hier_thresh = 0.5;

    int nboxes = 0;
    detection *dets = get_network_boxes(net, im.w, im.h, thresh,
                                        hier_thresh, 0, 1, &nboxes);

    l = net->layers[net->n - 1];
    draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);

    free_detections(dets, nboxes);

    if(QUANTIZE_ENABLE)
        save_image(im, "int8_predictions");
    else
        save_image(im, "fp32_predictions");

    free_network(net);
    free_image(im);

    return 1;
}
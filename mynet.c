#include <stdlib.h>
#include <stdio.h>

#include "mynet.h"

#define PRINT_WEIGHTS 1


int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr, "usage: give a filename\n");
        return 0;
    }

    char *filename = argv[1];
    char *weights_file = "genv2_tiny_voc.weights";
    
    FILE *check_w_fp = NULL;
    if (PRINT_WEIGHTS)
        check_w_fp = fopen("mynet_weights.txt", "a");

    // create network structure
    network *net = make_network();

    // check network
    for (int i = 0; i<net->n; ++i){
        layer ltmp = net->layers[i];
        if (ltmp.type == CONVOLUTION){
            fprintf(stderr, "%d conv  %5d %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n",
                    i, ltmp.n, ltmp.size, ltmp.size, ltmp.stride, ltmp.size_in, ltmp.size_in, ltmp.c, ltmp.size_out, ltmp.size_out, ltmp.n);
        }
        else if (ltmp.type == MAXPOOL){
            fprintf(stderr, "%d max         %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n",
                    i,  ltmp.size, ltmp.size, ltmp.stride, ltmp.size_in, ltmp.size_in, ltmp.c, ltmp.size_out, ltmp.size_out, ltmp.n);
        }
        else if (ltmp.type == REGION){
            fprintf(stderr, "%d reg\n", i);
        }
    }

    // load in weights
    load_weights(net, weights_file);

    if (PRINT_WEIGHTS) {
        for (int i = 0; i<net->n; ++i){
            layer ltmp = net->layers[i];
            if (ltmp.type == CONVOLUTION){
                int k;
                int num = ltmp.c * ltmp.n * ltmp.size * ltmp.size;
                fprintf(check_w_fp, "conv weights: %d\n", num);
                for (k = 0; k < num; ++k)
                {
                    fprintf(check_w_fp, "%g, ", ltmp.weights[k]);
                    if (k % 10 == 9)
                        fprintf(check_w_fp, "\n");
                }
                fprintf(check_w_fp, "\n\n");
            }
        }
    }
    if (PRINT_WEIGHTS)
        fclose(check_w_fp);

    // load image and resize it to the network input size
    image im = load_image(filename, net->width, net->height, net->channels);
    
    net->input = im.data;
    //forward_network(net);
    // for (int 1 = 0; i < net->n; ++i) {
    //     net->index = i;
    //     layer l = net->layers[i];
    //     //l.forward(l, *net);
    //     net->input = l.output;
    // }
    // float *out = net.input;

    free_network(net);
    free_image(im);

    return 1;
}
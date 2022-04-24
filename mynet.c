#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mynet.h"

#define INT_MAX  2147483647

#define PRINT_WEIGHTS 0
#define PRINT_OUTPUT 1

char *fgetl(FILE *fp)
{
    if(feof(fp)) return 0;
    size_t size = 512;
    char *line = malloc(size*sizeof(char));
    if(!fgets(line, size, fp)){
        free(line);
        return 0;
    }

    size_t curr = strlen(line);

    while((line[curr-1] != '\n') && !feof(fp)){
        if(curr == size-1){
            size *= 2;
            line = realloc(line, size*sizeof(char));
            if(!line) {
                printf("%ld\n", size);
                fprintf(stderr, "Malloc error\n");
                exit(-1);
            }
        }
        size_t readsize = size-curr;
        if(readsize > INT_MAX) readsize = INT_MAX-1;
        fgets(&line[curr], readsize, fp);
        curr = strlen(line);
    }
    if(line[curr-1] == '\n') line[curr-1] = '\0';

    return line;
}

void **list_to_array(list *l)
{
    void **a = calloc(l->size, sizeof(void*));
    int count = 0;
    node *n = l->front;
    while(n){
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
	while(n) {
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

	if(!l->back){
		l->front = new;
		new->prev = 0;
	}else{
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
    if(!file) {
        fprintf(stderr, "Couldn't open file: %s\n", filename);
        exit(0);
    }
    list *lines = make_list();
    while((path=fgetl(file))){
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
    for(j = 0; j < nsize; ++j){
        alphabets[j] = calloc(128, sizeof(image));
        for(i = 32; i < 127; ++i){
            char buff[256];
            sprintf(buff, "data/labels/%d_%d.png", i, j);
            alphabets[j][i] = load_image_color(buff, 0, 0);
        }
    }
    return alphabets;
}

int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr, "usage: give a filename\n");
        return 0;
    }

    char *filename = argv[1];
    char *weights_file = "genv2_tiny_voc.weights";
    
    FILE *check_w_fp;
    FILE *check_out_fp;

    if (PRINT_WEIGHTS)
        check_w_fp = fopen("mynet_weights.txt", "a");

    if (PRINT_OUTPUT)
        check_out_fp = fopen("mynet_outputs_23.txt", "a");

    char *names[20] = {"aeroplane", "bicycle", "bird", "boat", "bottle", "bus","car", "cat", "chair", "cow", "diningtable", "dog", "horse","motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"};
    
    // image **alphabet = load_alphabet();
    image **alphabet = NULL;
    

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

    for (int i = 0; i < net->n; ++i) {
        net->index = i;
        layer l = net->layers[i];
        // fprintf(stderr, "forward pass for layer %d...\n", i);
        l.forward(l, *net);

        net->input = l.output;
    }

    // layer 0 output
    layer ltmp = net->layers[0];
    int len = ltmp.size_out*ltmp.size_out*ltmp.n;
    if (PRINT_OUTPUT){
        fprintf(check_out_fp, "\n");
        fprintf(check_out_fp, "layer %d output: %d\n", 0, len);
        for (int j=0; j<len; ++j){
            fprintf(check_out_fp, "%g, ", ltmp.output[j]);
            if (j % 10 == 9)
                fprintf(check_out_fp, "\n");
        }
        fprintf(check_out_fp, "\n\n");
    }

    if (PRINT_OUTPUT)
        fclose(check_out_fp);
 

    float thresh = 0.5;
    float hier_thresh = 0.5;

    int nboxes = 0;
    detection *dets = get_network_boxes(net, im.w, im.h, thresh, 
                                        hier_thresh, 0, 1, &nboxes);

    layer l = net->layers[net->n-1];
    draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);

    free_detections(dets, nboxes);
    save_image(im, "predictions");

    free_network(net);
    free_image(im);


    return 1;
}
typedef struct
{
    int w;
    int h;
    int c;
    float *data;
} image;

typedef enum {
    CONVOLUTION,
    CONNECTED,
    MAXPOOL,
    REGION
} LAYER_TYPE;

typedef enum {
    LINEAR,
    LEAKY,
    LOGISTIC
} ACTIVATION;

typedef struct{
    float x, y, w, h;
} box;

typedef struct detection{
    box bbox;
    int classes;
    float *prob;
    float *mask;
    float objectness;
    int sort_class;
} detection;


typedef enum{
    PNG, BMP, TGA, JPG
} IMTYPE;

struct network;
typedef struct network network;

struct layer;
typedef struct layer layer;

struct layer{
    LAYER_TYPE type;
    int index;
    // input size
    int size_in;
    // output size
    int size_out;
    // num ouput channel
    int n;
    // conv/maxpool stride
    int stride;
    // conv padding
    int pad;
    // num input channel
    int c;
    // kernel/maxpool sizes
    int size;
    // activation function
    ACTIVATION activation;
    // whether use batch_normalize
    int batch_normalize;
    // detection thresh hold
    float thresh;
    // space needed for intermidiate calculation in byte
    // size_t workspace_size;

    // region layer
    int classes;
    int coords;
    int num;

    // for quantization purpose
    float amax;

    // storage
    float *weights;
    float *biases;
    float *scales;
    float *rolling_mean;
    float *rolling_variance;
    float *output;
    void (*forward)   (struct layer, struct network);
};

typedef struct network{
    int n;
    int width;
    int height;
    int channels;
    layer *layers;
    int index;
    float *output;
    float *input;
    float *workspace;
} network;

typedef struct node{
    void *val;
    struct node *next;
    struct node *prev;
} node;

typedef struct list{
    int size;
    node *front;
    node *back;
} list;

image load_image(char *filename, int width, int height, int channels);

void free_image(image im);

network *make_network();

void free_network(network *net);

void load_weights(network *net, char *weights);

detection *get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num);

void draw_detections(image im, detection *dets, int num, float thresh, char **names, image **alphabet, int classes);

void free_detections(detection *dets, int n);

void save_image(image im, const char *name);

image load_image_color(char *filename, int w, int h);

float get_input_pixel(int row, int col, int channel, int kernel_row, int kernel_col, int pad, int image_size, float *image);

float quantize(float x, float amax, int bitnum);

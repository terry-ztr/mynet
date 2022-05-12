#define QUANTIZE_ENABLE 1
#define PRINT_OUTPUT 1
#define PRINT_WEIGHT 1
#define FIND_RANGE 0
#define FIND_OUT_RANGE 0

#define INT_MAX 2147483647
#define FLT_MAX 10000

#define LAYERNUM 16

#define FULL_INT8 0


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

    // region layer
    int classes;
    int coords;
    int num;

    // for quantization purpose
    float amax_w;
    float amax_out;
    float amax_m;
    float amax_var;
    float amax_scale;
    // before batchnorm
    float amax_out_raw;
    int quantize;

    // storage
    float *weights;
    float *biases;
    float *scales;
    float *rolling_mean;
    float *rolling_variance;
    float *output;
    
    int8_t *int8_weights;
    int8_t *int8_biases;
    int8_t *int8_scales;
    int8_t *int8_rolling_mean;
    int8_t *int8_rolling_variance;
    int *intermediate_output;
    int8_t *int8_output;

    void (*forward)   (struct layer, struct network);
    void (*int8_forward)   (struct layer, struct network);
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
    int8_t *int8_output;
    int8_t *int8_input;
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

int8_t int8_quantize(float x, float amax, int bitnum);

float int8_dequantize(int8_t xq, float amax, int bitnum);

void quantize_array(float *output, int8_t *int8_output, int array_len, float amax, int bitnum);

void sudo_quantize_array(float *output, int array_len, float amax, int bitnum);
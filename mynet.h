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
    LEAKY
} ACTIVATION;

struct network;
typedef struct network network;

struct layer;
typedef struct layer layer;

struct layer{
    LAYER_TYPE type;
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

image load_image(char *filename, int width, int height, int channels);

void free_image(image im);

network *make_network();

void free_network(network *net);

void load_weights(network *net, char *weights);
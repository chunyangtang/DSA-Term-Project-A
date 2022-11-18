#include <assert.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#pragma pack(1)

using namespace std;

#define SOURCE_HEIGHT 512
#define SOURCE_WIDTH 512
#define QUERY_HEIGHT 64
#define QUERY_WIDTH 64
#define MD 600011
#define QUERY_TURNS 8
#define PI 3.1415926
#define NUM 64
#define BLOCK_SIZE 8

int conflict_count = 0;

// DCT by CSDN at https://blog.csdn.net/BigDream123/article/details/104395587
// process data using dct and return the average
int DCT(int data[NUM][NUM])
{
    int output[NUM][NUM];
    double alpha, beta; // C(k) C(l)
    int m = 0, n = 0, k = 0, l = 0;
    int sum = 0;
    for (k = 0; k < NUM; k++)
    {
        for (l = 0; l < NUM; l++)
        {
            if (k == 0)
            {
                alpha = sqrt(1.0 / NUM);
            }
            else
            {
                alpha = sqrt(2.0 / NUM);
            }
            if (l == 0)
            {
                beta = sqrt(1.0 / NUM);
            }
            else
            {
                beta = sqrt(2.0 / NUM);
            }
            double temp = 0.0;
            for (m = 0; m < NUM; m++)
            {
                for (n = 0; n < NUM; n++)
                {
                    temp += data[m][n] * cos((2 * m + 1) * k * PI / (2.0 * NUM)) * cos((2 * n + 1) * l * PI / (2.0 * NUM));
                }
            }
            int out = alpha * beta * temp;
            output[k][l] = out >= 0 ? out + 0.5 : out - 0.5;
            sum += output[k][l];
        }
    }
    memset(data, 0, sizeof(int) * NUM * NUM);
    memcpy(data, output, sizeof(int) * NUM * NUM);
    return sum / NUM / NUM;
}

struct Node
{
    short int h, w;
    Node *ptr;
    Node(int x = -1, int y = 0, Node *ptr = nullptr) : h(x), w(y), ptr(ptr) {}
    void insert(int height, int width)
    {
        if (h == -1)
        {
            h = height;
            w = width;
        }
        else
        {
            conflict_count++;
            if (ptr == nullptr)
                ptr = new Node(height, width);
            else
                ptr->insert(height, width);
        }
    }
};

string data_path = "test_folder_advanced/custom_1/";

int main()
{
    // opening the source file
    FILE *source_file;
    string source_path = data_path + "source_g.data";
    source_file = fopen(source_path.c_str(), "rb");
    if (source_file == NULL)
    {
        cout << "Error: source file not found!" << endl;
        return 0;
    }

    // storing the height and width of source image
    int h, w;
    fread(&h, sizeof(int), 1, source_file);
    fread(&w, sizeof(int), 1, source_file);
    assert(h == SOURCE_HEIGHT && w == SOURCE_WIDTH);

    // reading the source image into the array
    int source_array[SOURCE_HEIGHT][SOURCE_WIDTH];
    for (int r = 0; r < h; r++)
    {
        for (int c = 0; c < w; c++)
        {
            int t;
            fread(&t, sizeof(int), 1, source_file);
            source_array[r][c] = t;
        }
    }
    // closing the source file
    fclose(source_file);

    // computing hash value of each part of source image
    int hash_count = 0;
    Node hash_table[MD];
    int current_height = 0, current_width = 0;
    int part_array[QUERY_HEIGHT][QUERY_WIDTH];
    while (current_height <= h - QUERY_HEIGHT)
    {
        while (current_width <= w - QUERY_WIDTH)
        {
            int current_hash = 0;
            for (int r = current_height; r < current_height + QUERY_HEIGHT; r++)
            {
                for (int c = current_width; c < current_width + QUERY_WIDTH; c++)
                {
                    part_array[r - current_height][c - current_width] = source_array[r][c]; // need optimization
                }
            }

            // DCT
            int threshold = DCT(part_array);
            for (int r = 0; r < BLOCK_SIZE; r++)
                for (int c = 0; c < BLOCK_SIZE; c++)
                    if (part_array[r][c] >= threshold)
                        current_hash = (256 * current_hash + 1) % MD;
                    else
                        current_hash = (256 * current_hash) % MD;

            hash_count++;
            hash_table[current_hash].insert(current_height, current_width);
            current_width++;
        }
        current_width = 0;
        current_height++;
    }
    // assert(hash_count == (SOURCE_HEIGHT - QUERY_HEIGHT + 1) * (SOURCE_WIDTH - QUERY_WIDTH + 1));

    // circularly opening the query file and compute result
    FILE *query_file;
    int query_turn = 0;

    string result;

    while (query_turn < QUERY_TURNS)
    {
        string query_path = data_path + "query" + to_string(query_turn) + "_g.data";
        query_file = fopen(query_path.c_str(), "rb");
        if (query_file == NULL)
        {
            cout << "Error: query file " << query_turn << " not found!" << endl;
            return 0;
        }

        int h, w;
        fread(&h, sizeof(int), 1, query_file);
        fread(&w, sizeof(int), 1, query_file);
        assert(h == QUERY_HEIGHT && w == QUERY_WIDTH);

        int query_array[QUERY_HEIGHT][QUERY_WIDTH];
        for (int r = 0; r < h; r++)
        {
            for (int c = 0; c < w; c++)
            {
                int t;
                fread(&t, sizeof(int), 1, query_file);
                query_array[r][c] = t;
            }
        }
        fclose(query_file);

        // computing hash value of query image
        int query_hash = 0;
        int threshold = DCT(query_array);
        for (int r = 0; r < BLOCK_SIZE; r++)
            for (int c = 0; c < BLOCK_SIZE; c++)
                if (query_array[r][c] >= threshold)
                    query_hash = (256 * query_hash + 1) % MD;
                else
                    query_hash = (256 * query_hash) % MD;
        // searching the hash table
        if (hash_table[query_hash].h == -1)
        {
            while (hash_table[query_hash].h == -1)
            {
                query_hash = (query_hash + 1) % MD;
            }
        }
        result += to_string(hash_table[query_hash].w) + " " + to_string(hash_table[query_hash].h) + "\n";

        query_turn++;
    }

    // writing the result to the file
    FILE *result_file;
    string result_path = data_path + "result.txt";
    result_file = fopen(result_path.c_str(), "w");
    if (result_file == NULL)
    {
        cout << "Error: result file not found!" << endl;
        return 0;
    }
    fprintf(result_file, "%s", result.c_str());
    fclose(result_file);

    cout << "Result exported, conflict count: " << conflict_count << endl;

    return 0;
}
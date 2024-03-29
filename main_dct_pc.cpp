#include <assert.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>

#pragma pack(1)

using namespace std;

#define SOURCE_HEIGHT 512
#define SOURCE_WIDTH 512
#define QUERY_HEIGHT 64
#define QUERY_WIDTH 64
#define MD 600011 // 5000011
#define QUERY_TURNS 8
#define PI 3.1415926
#define NUM 64
#define BLOCK_SIZE 8
#define LARGE_BLOCK_SIZE 32

int conflict_count = 0;

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

string data_output_path = "test_folder_advanced/test/";
string data_path = "process_folder.nosync/";

int main()
{
    float initial_time = clock();
    float start_time = clock(), end_time;

    // computing hash value of each part of source image
    int hash_count = 0;
    // Node *hash_table = new Node[MD];
    Node hash_table[MD];
    int current_height = 0, current_width = 0;

    FILE *part_source_file;
    string source_path;
    while (current_height <= SOURCE_HEIGHT - QUERY_HEIGHT)
    {
        while (current_width <= SOURCE_WIDTH - QUERY_WIDTH)
        {
            int current_hash = 0;
            // opening the part source file
            source_path = data_path + "source_g" + to_string(current_height) + "_" + to_string(current_width) + ".data";
            part_source_file = fopen(source_path.c_str(), "rb");
            if (part_source_file == NULL)
            {
                cout << "Error: source file " << current_height << ' ' << current_width << " not found!" << endl;
                return 0;
            }

            // storing the height and width of source image
            int h, w;
            fread(&h, sizeof(int), 1, part_source_file);
            fread(&w, sizeof(int), 1, part_source_file);
            assert(h == QUERY_HEIGHT && w == QUERY_WIDTH);

            // computing the hash value of the part
            int hash_value = 0;
            int t;
            srand(1);
            for (int r = 0; r < h; r++)
            {
                for (int c = 0; c < w; c++)
                {
                    fread(&t, sizeof(int), 1, part_source_file);
                    if (r < BLOCK_SIZE && c < BLOCK_SIZE)
                        hash_value = (256 * hash_value + t * (rand() % 1000000)) % MD;
                }
            }
            // closing the source file
            fclose(part_source_file);
            hash_count++;
            hash_table[hash_value].insert(current_height, current_width);
            current_width++;
        }
        current_width = 0;
        current_height++;
        cout << '-';
    }
    assert(hash_count == (SOURCE_HEIGHT - QUERY_HEIGHT + 1) * (SOURCE_WIDTH - QUERY_WIDTH + 1));

    cout << endl;
    end_time = clock();
    cout << "Hash table build time: " << (end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
    start_time = clock();

    // circularly opening the query file and compute result
    FILE *query_file;
    int query_turn = 0;
    string query_path;
    string result;

    while (query_turn < QUERY_TURNS)
    {
        query_path = data_path + "query" + to_string(query_turn) + "_g.data";
        query_file = fopen(query_path.c_str(), "rb");
        if (query_file == NULL)
        {
            cout << "Error: query file " << query_turn << " not found!" << endl;
            return 0;
        }

        int h, w;
        int query_hash = 0;
        fread(&h, sizeof(int), 1, query_file);
        fread(&w, sizeof(int), 1, query_file);
        assert(h == QUERY_HEIGHT && w == QUERY_WIDTH);

        // computing hash value of query image
        int t;
        srand(1);
        for (int r = 0; r < h; r++)
        {
            for (int c = 0; c < w; c++)
            {
                fread(&t, sizeof(int), 1, query_file);
                if (r < BLOCK_SIZE && c < BLOCK_SIZE)
                    query_hash = (256 * query_hash + t * (rand() % 1000000)) % MD;
            }
        }

        fclose(query_file);

        // searching the hash table
        if (hash_table[query_hash].h == -1)
        {
            while (hash_table[query_hash].h == -1)
            {
                query_hash = (query_hash + 1) % MD;
            }
        }
        if (hash_table[query_hash].ptr != nullptr)
        {
            // conpare the query image with the source image
            bool found = false;
            while (hash_table[query_hash].ptr != nullptr)
            {
                // computing pHash of part source file with larger block size

                source_path = data_path + "source_g" + to_string(hash_table[query_hash].h) + '_' + to_string(hash_table[query_hash].w) + ".data";
                part_source_file = fopen(source_path.c_str(), "rb");
                if (part_source_file == NULL)
                {
                    cout << "Error: source file " << hash_table[query_hash].h << ' ' << hash_table[query_hash].w << " not found!" << endl;
                    return 0;
                }

                // storing the height and width of source image
                int h, w;
                fread(&h, sizeof(int), 1, part_source_file);
                fread(&w, sizeof(int), 1, part_source_file);
                assert(h == QUERY_HEIGHT && w == QUERY_WIDTH);

                // computing the hash value of the part
                int hash_value = 0;
                int t;
                srand(1);
                for (int r = 0; r < h; r++)
                {
                    for (int c = 0; c < w; c++)
                    {
                        fread(&t, sizeof(int), 1, part_source_file);
                        if (r < LARGE_BLOCK_SIZE && c < LARGE_BLOCK_SIZE)
                            hash_value = (256 * hash_value + t * (rand() % 1000000)) % MD;
                    }
                }
                // closing the source file
                fclose(part_source_file);

                query_file = fopen(query_path.c_str(), "rb");
                if (query_file == NULL)
                {
                    cout << "Error: query file " << query_turn << " not found!" << endl;
                    return 0;
                }

                int query_hash_large = 0;
                fread(&h, sizeof(int), 1, query_file);
                fread(&w, sizeof(int), 1, query_file);
                assert(h == QUERY_HEIGHT && w == QUERY_WIDTH);

                // computing hash value of query image
                srand(1);
                for (int r = 0; r < h; r++)
                {
                    for (int c = 0; c < w; c++)
                    {
                        fread(&t, sizeof(int), 1, query_file);
                        if (r < LARGE_BLOCK_SIZE && c < LARGE_BLOCK_SIZE)
                            query_hash_large = (256 * query_hash_large + t * (rand() % 1000000)) % MD;
                    }
                }
                fclose(query_file);

                if (hash_value == query_hash_large)
                {
                    found = true;
                    break;
                }
                hash_table[query_hash].h = hash_table[query_hash].ptr->h;
                hash_table[query_hash].w = hash_table[query_hash].ptr->w;
                hash_table[query_hash].ptr = hash_table[query_hash].ptr->ptr;
            }
        }
        result += to_string(hash_table[query_hash].w) + " " + to_string(hash_table[query_hash].h) + "\n";

        query_turn++;
    }

    end_time = clock();
    cout << "Query time: " << (end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;

    // writing the result to the file
    FILE *result_file;
    string result_path = data_output_path + "result.txt";
    result_file = fopen(result_path.c_str(), "w");
    if (result_file == NULL)
    {
        cout << "Error: result file not found!" << endl;
        return 0;
    }
    fprintf(result_file, "%s", result.c_str());
    fclose(result_file);

    cout << "Result exported, conflict count: " << conflict_count << endl;
    // delete[] hash_table;
    end_time = clock();
    cout << "Total time: " << (end_time - initial_time) / CLOCKS_PER_SEC << "s" << endl;

    return 0;
}
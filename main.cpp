#include <assert.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <time.h>

#pragma pack(1)

using namespace std;

#define SOURCE_HEIGHT 512
#define SOURCE_WIDTH 512
#define QUERY_HEIGHT 64
#define QUERY_WIDTH 64
#define MD 5000011
#define QUERY_TURNS 8

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

string data_path = "test_folder_base/test/";

int main()
{
    float initial_time = clock();
    float start_time = clock(), end_time;
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

    // cout << h << " " << w << endl;

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

    end_time = clock();
    cout << "Time taken to read source file: " << (end_time - start_time) / CLOCKS_PER_SEC << " seconds" << endl;
    start_time = clock();

    // computing hash value of each part of source image
    int hash_count;
    Node *hash_table = new Node[MD];
    int current_height = 0, current_width = 0;
    while (current_height <= h - QUERY_HEIGHT)
    {
        while (current_width <= w - QUERY_WIDTH)
        {
            int current_hash = 0;
            for (int r = current_height; r < current_height + QUERY_HEIGHT; r++)
            {
                for (int c = current_width; c < current_width + QUERY_WIDTH; c++)
                {
                    current_hash = (256 * current_hash + source_array[r][c]) % MD;
                }
            }
            hash_count++;
            hash_table[current_hash].insert(current_height, current_width);
            current_width++;
        }
        current_width = 0;
        current_height++;
    }
    assert(hash_count == (SOURCE_HEIGHT - QUERY_HEIGHT + 1) * (SOURCE_WIDTH - QUERY_WIDTH + 1));

    end_time = clock();
    cout << "Time taken to compute hash table: " << (end_time - start_time) / CLOCKS_PER_SEC << " seconds" << endl;
    start_time = clock();

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
        for (int r = 0; r < h; r++)
        {
            for (int c = 0; c < w; c++)
            {
                query_hash = (256 * query_hash + query_array[r][c]) % MD;
            }
        }

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
            int current_height = hash_table[query_hash].h;
            int current_width = hash_table[query_hash].w;
            bool found = false;
            while (hash_table[query_hash].ptr != nullptr)
            {
                bool match = true;
                for (int r = 0; r < QUERY_HEIGHT; r++)
                {
                    for (int c = 0; c < QUERY_WIDTH; c++)
                    {
                        if (source_array[current_height + r][current_width + c] != query_array[r][c])
                        {
                            match = false;
                            break;
                        }
                    }
                    if (!match)
                        break;
                }
                if (match)
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
    cout << "Time taken of query: " << (end_time - start_time) / CLOCKS_PER_SEC << " seconds" << endl;

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

    delete[] hash_table;

    cout << "Result exported, conflict count: " << conflict_count << endl;
    end_time = clock();
    cout << "Total time taken: " << (end_time - initial_time) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
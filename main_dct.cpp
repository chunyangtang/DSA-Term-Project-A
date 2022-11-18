#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>
#include <assert.h>
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

string data_path = "test_folder/basic2/";

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

    // initializing python interface
    // Set PYTHONPATH TO working directory
    setenv("PYTHONPATH", ".", 1);
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")");

    PyObject *pModule = PyImport_ImportModule("my_dct");
    PyObject *pFunc = PyObject_GetAttrString(pModule, "dct");
    PyObject *pArgs = PyTuple_New(1);

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
                    // current_hash = (256 * current_hash + source_array[r][c]) % MD;
                }
            }
            // converting part_array into a python 2d list
            PyObject *pList = PyList_New(0);
            for (int r = 0; r < QUERY_HEIGHT; r++)
            {
                PyObject *pRow = PyList_New(0);
                for (int c = 0; c < QUERY_WIDTH; c++)
                {
                    PyObject *pValue = Py_BuildValue("i", part_array[r][c]);
                    PyList_Append(pRow, pValue);
                }
                PyList_Append(pList, pRow);
            }
            // passing the list to python function
            PyTuple_SetItem(pArgs, 0, pList);
            PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
            // getting the hash value from python function
            current_hash = PyLong_AsLong(pValue);

            hash_count++;
            hash_table[current_hash].insert(current_height, current_width);
            current_width++;
        }
        current_width = 0;
        current_height++;
    }
    assert(hash_count == (SOURCE_HEIGHT - QUERY_HEIGHT + 1) * (SOURCE_WIDTH - QUERY_WIDTH + 1));

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
        // for (int r = 0; r < h; r++)
        // {
        //     for (int c = 0; c < w; c++)
        //     {
        //         query_hash = (256 * query_hash + query_array[r][c]) % MD;
        //     }
        // }

        // converting part_array into a python 2d list
        PyObject *pList = PyList_New(0);
        for (int r = 0; r < QUERY_HEIGHT; r++)
        {
            PyObject *pRow = PyList_New(0);
            for (int c = 0; c < QUERY_WIDTH; c++)
            {
                PyObject *pValue = Py_BuildValue("i", query_array[r][c]);
                PyList_Append(pRow, pValue);
            }
            PyList_Append(pList, pRow);
        }
        // passing the list to python function
        PyTuple_SetItem(pArgs, 0, pList);
        PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
        // getting the hash value from python function
        query_hash = PyLong_AsLong(pValue);

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

    Py_FinalizeEx();

    cout << "Result exported, conflict count: " << conflict_count << endl;

    return 0;
}
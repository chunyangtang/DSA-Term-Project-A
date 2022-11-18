#include <iostream>
using namespace std;

int source_array[512][512];

int main()
{
    // read
    FILE *im_file;
    int h, w;
    im_file = fopen("data_basic1/query0_g.data", "rb");
    fread(&h, sizeof(int), 1, im_file);
    fread(&w, sizeof(int), 1, im_file);
    cout << h << " " << w << endl;
    for (int r = 0; r < h; r++)
    {
        for (int c = 0; c < w; c++)
        {
            int t;
            fread(&t, sizeof(int), 1, im_file);
            source_array[r][c] = t;
        }
    }
    fclose(im_file);

    // write
    im_file = fopen("data_basic1/query0_g.data", "wb");
    fwrite(&h, sizeof(int), 1, im_file);
    fwrite(&w, sizeof(int), 1, im_file);
    cout << h << " " << w << endl;
    for (int r = 0; r < h; r++)
    {
        for (int c = 0; c < w; c++)
        {
            int t = source_array[r][c];
            fwrite(&t, sizeof(int), 1, im_file);
        }
    }
    fclose(im_file);
    return 0;
}

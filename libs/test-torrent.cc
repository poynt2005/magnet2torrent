#include "GetTorrent.h"
#include <stdio.h>
#include <fstream>

int main(){
    const char* uri = "magnet:?xt=urn:btih:THECXNZVAWR4BNCT7H5A5CA5NZNDFIGB&dn=ubuntu-22.10-desktop-amd64.iso&xl=4071903232&tr=https%3A%2F%2Ftorrent.ubuntu.com%2Fannounce";
    char* bin = NULL;
    char* filename = NULL;
    int size = 0;
    if(GetTorrent(uri, &filename, &bin, &size)){
        printf("%s\n", filename);
        printf("%s\n", bin);
        printf("%d\n", size);

        std::ofstream outFile("out.torrent", std::ios::binary);
        outFile.write(bin ,size);
        outFile.close();

        ReleaseTorrent(&filename, &bin);

    }

    return 1;
}
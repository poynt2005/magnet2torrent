#include "Magnet2Torrent.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

int main()
{
    const torrent_t torrent = CreateTorrent();

    if (TORRENT_SUCCEED != SetMagnet(torrent, "magnet:?xt=urn:btih:IQ6HMAVU7XUD2EKU23M5USEAQQMLDANW&dn=ubuntu-23.04-desktop-amd64.iso&xl=4932407296&tr=https%3A%2F%2Ftorrent.ubuntu.com%2Fannounce"))
    {
        printf("Cannot set magnet, %d\n", GetTorrentError(torrent));
        exit(-1);
    }

    if (TORRENT_SUCCEED != Magnet2Torrent(torrent))
    {
        printf("Cannot transform magnet 2 torrent, %d\n", GetTorrentError(torrent));
        exit(-1);
    }

    size_t size = 0;
    const char *buffer = GetTorrent(torrent, &size);
    if (buffer == NULL)
    {
        printf("Cannot get torrent buffer, %d\n", GetTorrentError(torrent));
        exit(-1);
    }

    FILE *fp = fopen("test.torrent", "wb");

    fwrite(buffer, size, 1, fp);
    fclose(fp);

    DestroyTorrent(torrent);

    printf("Done\n");
    return 0;
}
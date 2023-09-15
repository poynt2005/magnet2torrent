#include "Magnet2Torrent.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

int main()
{
    const torrent_t torrent = CreateTorrent();

    FILE *fp = fopen("test.torrent", "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    char *buffer = (char *)malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(buffer, size, 1, fp);
    fclose(fp);

    if (TORRENT_SUCCEED != SetTorrentBuffer(torrent, buffer, size))
    {
        printf("Cannot set torrent buffer, %d\n", GetTorrentError(torrent));
        DestroyTorrent(torrent);
        exit(-1);
    }

    if (TORRENT_SUCCEED != Torrent2Magnet(torrent))
    {
        printf("Cannot transform torrent 2 magnet, %d\n", GetTorrentError(torrent));
        DestroyTorrent(torrent);
        exit(-1);
    }

    const char *mangetURL = GetMagnet(torrent);

    if (mangetURL == NULL)
    {
        printf("Cannot get magnet link, %d\n", GetTorrentError(torrent));
        DestroyTorrent(torrent);
        exit(-1);
    }

    printf("Magnet link: %s\n", mangetURL);
    DestroyTorrent(torrent);
    printf("Done\n");
    return 0;
}
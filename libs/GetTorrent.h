#ifndef __GET_TORRENT_H__
#define __GET_TORRENT_H__

#ifdef __cplusplus
extern "C" {
#endif


int GetTorrent(const char *, char** ,char **, int*);
void ReleaseTorrent(char**, char**);

#ifdef __cplusplus
}
#endif

#endif
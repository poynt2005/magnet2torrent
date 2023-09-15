#ifndef __MAGNET_TO_TORRENT_H__
#define __MAGNET_TO_TORRENT_H__

#ifdef __WIN32
#define M2TAPI __declspec(dllexport)
#else
#define M2TAPI
#endif

#include <stddef.h>

// typedef struct __torrent_t
// {
//     char *magnetLink;
//     char *buffer;
//     size_t bufferSize;
// } torrent_t;

typedef unsigned long long torrent_t;

#define SEARCH_TIME_OUT 250

#define TORRENT_CONTEXT_NOT_CREATED -1

#define TORRENT_SUCCEED 1
#define TORRENT_FAILED 0

enum M2T_ERROR_CODE
{
    M2T_ERROR_NONE = 0,
    M2T_MAGNET_LINK_NOT_SET,
    M2T_BUFFER_EMPTY,
    M2T_MANGET_LINK_NOT_VALID,
    M2T_FAILED_CREATE_TEMP_FOLDER,
    M2T_FAILED_GET_FILE_FROM_TORRENT_HANDLE,
    M2T_LT_RECERIEVED_FAILED_ALERT,
    M2T_WRITE_TORRENT_ERROR,
    M2T_LT_FAILED_DECODE_BENCODE,
    M2T_LT_FAILED_DECODE_MAGNET,
    M2T_LT_SEARCH_TIME_OUT,
};

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Get torrent buffer by magnet
     *
     * @param torrent This is the torrent_t handle
     * @return int
     */
    M2TAPI int Magnet2Torrent(const torrent_t torrent);

    /**
     * @brief Get magnet from torrent buffer
     *
     * @param torrent This is the torrent_t handle
     * @return int
     */
    M2TAPI int Torrent2Magnet(const torrent_t torrent);

    /**
     * @brief Create a Torrent object
     *
     * This function will create a torrent_t instance
     * @return torrent_t
     */
    M2TAPI const torrent_t CreateTorrent();

    /**
     * @brief Set the Torrent Buffer
     *
     * @param torrent This is the torrent_t handle
     * @param buffer This is the torrent binary buffer
     * @param size This is the torrent binary size
     * @return int
     */
    M2TAPI int SetTorrentBuffer(const torrent_t torrent, const char *buffer, const size_t size);

    /**
     * @brief Set the Magnet link
     *
     * @param torrent This is the torrent_t handle
     * @param magnet Magnet link string
     * @return int
     */
    M2TAPI int SetMagnet(const torrent_t torrent, const char *magnet);

    /**
     * @brief Get the Torrent buffer
     *
     * @param torrent This is the torrent_t handle
     * @param size This is a reference to get torrent binary buffer size
     * @return const*
     */
    M2TAPI const char *GetTorrent(const torrent_t torrent, size_t *size);

    /**
     * @brief Get the Magnet Link
     *
     * @param torrent This is the torrent_t handle
     * @return const*
     */
    M2TAPI const char *GetMagnet(const torrent_t torrent);

    /**
     * @brief Get the Torrent Error code
     *
     * @param torrent This is the torrent_t handle
     * @return int
     */
    M2TAPI int GetTorrentError(const torrent_t torrent);

    /**
     * @brief Destroy the target torrent
     *
     * @param torrent This is the torrent_t handle
     */
    M2TAPI void DestroyTorrent(const torrent_t torrent);

#ifdef __cplusplus
}
#endif

#endif
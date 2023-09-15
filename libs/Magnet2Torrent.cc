#include <memory>
#include <string>
#include <filesystem>
#include <random>
#include <string.h>
#include <thread>
#include <chrono>
#include <vector>
#include <optional>
#include <unordered_map>
#include <regex>

#include <stdio.h>

#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/disabled_disk_io.hpp>
#include <libtorrent/write_resume_data.hpp>

#include "Magnet2Torrent.h"

namespace fs = std::filesystem;

using TorrentContext = struct torrentContext
{
    std::vector<char> buffer;
    std::string magnetUrl;
    int errorCode;
};

std::unordered_map<torrent_t, std::unique_ptr<TorrentContext>> contextStore;

std::string gen_random(const int len)
{
    const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::string tmp_s;
    tmp_s.reserve(len);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, strlen(alphanum) - 1);

    for (int i = 0; i < len; ++i)
    {
        tmp_s += alphanum[distribution(gen)];
    }

    return tmp_s;
}

std::optional<std::vector<char>> GetTorrent(const std::string &magnetLink, int &errCode)
{
    lt::session_params params;
    params.disk_io_constructor = lt::disabled_disk_io_constructor;
    params.settings.set_int(lt::settings_pack::alert_mask, lt::alert_category::status | lt::alert_category::error);

    const auto tmpTorrentDir = fs::temp_directory_path() / gen_random(30);

    if (!fs::create_directories(tmpTorrentDir))
    {
        errCode = M2T_FAILED_CREATE_TEMP_FOLDER;
        return std::nullopt;
    }

    lt::session session(std::move(params));

    lt::add_torrent_params atp;

    try
    {
        atp = lt::parse_magnet_uri(magnetLink);
    }
    catch (...)
    {
        errCode = M2T_LT_FAILED_DECODE_MAGNET;
        fs::remove_all(tmpTorrentDir);
        return std::nullopt;
    }

    atp.save_path = tmpTorrentDir.string();
    // 自動開始下載，torrent 準備好之後暫停
    atp.flags &= ~(lt::torrent_flags::auto_managed | lt::torrent_flags::paused);
    atp.file_priorities.resize(100, lt::dont_download);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    session.add_torrent(std::move(atp));
    session.add_dht_node(std::pair<std::string, int>("router.utorrent.com", 6881));
    session.add_dht_node(std::pair<std::string, int>("dht.transmissionbt.com", 6881));
    session.add_dht_node(std::pair<std::string, int>("router.bitcomet.com", 6881));
    session.add_dht_node(std::pair<std::string, int>("dht.aelitis.com", 6881));
    session.add_dht_node(std::pair<std::string, int>("router.bittorrent.com", 6881));

    lt::entry *torrentEntry = nullptr;

    auto searchStart = std::chrono::high_resolution_clock::now();

    bool isDoneCheck = false;
    while (!isDoneCheck)
    {
        auto curSearchTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<int> elapsed = std::chrono::duration_cast<std::chrono::seconds>(curSearchTime - searchStart);

        if (elapsed.count() > SEARCH_TIME_OUT)
        {
            errCode = M2T_LT_SEARCH_TIME_OUT;
            break;
        }

        std::vector<lt::alert *> alerts;
        session.pop_alerts(&alerts);
        for (lt::alert *a : alerts)
        {
            const auto receivedAlert = lt::alert_cast<lt::metadata_received_alert>(a);
            const auto saveResumeDataAlert = lt::alert_cast<lt::save_resume_data_alert>(a);
            const auto saveResumeDataFailedAlert = lt::alert_cast<lt::save_resume_data_failed_alert>(a);

            // cast receivedAlert 成功
            if (receivedAlert != nullptr)
            {
                const auto &handle = receivedAlert->handle;
                auto ti = handle.torrent_file();
                if (ti == nullptr)
                {
                    errCode = M2T_FAILED_GET_FILE_FROM_TORRENT_HANDLE;
                    isDoneCheck = true;
                    break;
                }
                handle.save_resume_data(lt::torrent_handle::save_info_dict);
                handle.set_flags(lt::torrent_flags::paused);
            }
            // cast saveResumeDataAlert 成功
            else if (saveResumeDataAlert != nullptr)
            {
                saveResumeDataAlert->params.merkle_trees.clear();

                torrentEntry = new lt::entry(lt::write_torrent_file(saveResumeDataAlert->params, lt::write_flags::allow_missing_piece_layer));
                isDoneCheck = true;
                break;
            }
            // cast saveResumeDataFailedAlert
            else if (saveResumeDataFailedAlert != nullptr)
            {
                errCode = M2T_LT_RECERIEVED_FAILED_ALERT;
                isDoneCheck = true;
                break;
            }
        }
    }

    if (torrentEntry == nullptr)
    {
        fs::remove_all(tmpTorrentDir);
        return std::nullopt;
    }

    std::vector<char> torrentBufferVec;
    lt::bencode(std::back_inserter(torrentBufferVec), *torrentEntry);

    delete torrentEntry;
    torrentEntry = nullptr;
    fs::remove_all(tmpTorrentDir);

    return torrentBufferVec;
}

std::optional<std::string> GetMagnet(const std::vector<char> &torrentBuffer, int &errCode)
{
    std::string magnetLink;
    try
    {
        magnetLink = lt::make_magnet_uri(lt::torrent_info(torrentBuffer.data(), torrentBuffer.size()));
    }
    catch (...)
    {
        errCode = M2T_LT_FAILED_DECODE_BENCODE;
        return std::nullopt;
    }

    return magnetLink;
}

bool CheckTorrentExists(const torrent_t torrent)
{
    return contextStore.count(torrent) != 0;
}

/** C API */
int Magnet2Torrent(const torrent_t torrent)
{
    if (!CheckTorrentExists(torrent))
    {
        return TORRENT_CONTEXT_NOT_CREATED;
    }

    auto &ctx = contextStore[torrent];
    ctx->errorCode = M2T_ERROR_NONE;

    const auto &magnetUrl = ctx->magnetUrl;

    if (magnetUrl.empty() || !magnetUrl.length())
    {
        ctx->errorCode = M2T_MAGNET_LINK_NOT_SET;
        return TORRENT_FAILED;
    }

    int getTorrentErrCode = M2T_ERROR_NONE;
    auto getResult = GetTorrent(magnetUrl, getTorrentErrCode);

    if (getResult.has_value())
    {
        ctx->buffer = getResult.value();
        return TORRENT_SUCCEED;
    }

    ctx->errorCode = getTorrentErrCode;
    return TORRENT_FAILED;
}

int Torrent2Magnet(const torrent_t torrent)
{
    if (!CheckTorrentExists(torrent))
    {
        return TORRENT_CONTEXT_NOT_CREATED;
    }

    auto &ctx = contextStore[torrent];
    ctx->errorCode = M2T_ERROR_NONE;

    if ((ctx->buffer).empty() || !((ctx->buffer).size()))
    {
        ctx->errorCode = M2T_BUFFER_EMPTY;
        return TORRENT_FAILED;
    }

    int getMagnetErrCode = M2T_ERROR_NONE;
    auto getResult = GetMagnet(ctx->buffer, getMagnetErrCode);

    if (getResult.has_value())
    {
        ctx->magnetUrl = getResult.value();
        return TORRENT_SUCCEED;
    }

    ctx->errorCode = getMagnetErrCode;
    return TORRENT_FAILED;
}

void DestroyTorrent(const torrent_t torrent)
{
    if (!CheckTorrentExists(torrent))
    {
        return;
    }

    contextStore.erase(torrent);
}

const torrent_t CreateTorrent()
{
    auto ctx = std::make_unique<TorrentContext>(TorrentContext{
        std::vector<char>{},
        std::string(""),
        M2T_ERROR_NONE});

    const auto addr = reinterpret_cast<torrent_t>(ctx.get());

    contextStore.insert(std::pair<torrent_t, std::unique_ptr<TorrentContext>>(addr, std::move(ctx)));

    return addr;
}

int SetTorrentBuffer(const torrent_t torrent, const char *buffer, const size_t size)
{
    if (!CheckTorrentExists(torrent))
    {
        return TORRENT_CONTEXT_NOT_CREATED;
    }

    auto &ctx = contextStore[torrent];

    ctx->errorCode = M2T_ERROR_NONE;

    if (size <= 0)
    {
        ctx->errorCode = M2T_BUFFER_EMPTY;
        return TORRENT_FAILED;
    }

    (ctx->buffer).resize(size);
    memcpy((ctx->buffer).data(), buffer, size);

    return TORRENT_SUCCEED;
}

int SetMagnet(const torrent_t torrent, const char *magnet)
{
    if (!CheckTorrentExists(torrent))
    {
        return TORRENT_CONTEXT_NOT_CREATED;
    }

    auto &ctx = contextStore[torrent];

    ctx->errorCode = M2T_ERROR_NONE;

    const std::regex magnetPattern("magnet:\\?xt=urn:[a-z0-9]+:[a-z0-9]{32}", std::regex_constants::icase);

    if (strlen(magnet) > 0 && std::regex_search(magnet, magnetPattern))
    {
        ctx->magnetUrl = magnet;
        return TORRENT_SUCCEED;
    }

    ctx->errorCode = M2T_MANGET_LINK_NOT_VALID;
    return TORRENT_FAILED;
}

const char *GetTorrent(const torrent_t torrent, size_t *size)
{
    if (!CheckTorrentExists(torrent))
    {
        return nullptr;
    }

    auto &ctx = contextStore[torrent];

    ctx->errorCode = M2T_ERROR_NONE;

    if (!((ctx->buffer).empty()) && (ctx->buffer).size())
    {
        *size = (ctx->buffer).size();
        return (ctx->buffer).data();
    }

    *size = 0;
    ctx->errorCode = M2T_BUFFER_EMPTY;
    return nullptr;
}

const char *GetMagnet(const torrent_t torrent)
{
    if (!CheckTorrentExists(torrent))
    {
        return nullptr;
    }

    auto &ctx = contextStore[torrent];

    ctx->errorCode = M2T_ERROR_NONE;

    if (!((ctx->magnetUrl).empty()) && (ctx->magnetUrl).length())
    {
        return (ctx->magnetUrl).c_str();
    }

    ctx->errorCode = M2T_MAGNET_LINK_NOT_SET;
    return nullptr;
}

int GetTorrentError(const torrent_t torrent)
{
    if (!CheckTorrentExists(torrent))
    {
        return TORRENT_CONTEXT_NOT_CREATED;
    }

    auto &ctx = contextStore[torrent];

    return ctx->errorCode;
}
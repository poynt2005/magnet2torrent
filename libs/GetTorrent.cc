#include "GetTorrent.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <map>
#include <random>
#include <filesystem>
#include <algorithm>
#include <string.h>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/disabled_disk_io.hpp>
#include <libtorrent/write_resume_data.hpp>

bool GetTorrent(const std::string &mangetURI, std::string &outputTorrentFile, char** binFile, int* size)
{
    lt::session_params params;
    params.disk_io_constructor = lt::disabled_disk_io_constructor;

    params.settings.set_int(lt::settings_pack::alert_mask, lt::alert_category::status | lt::alert_category::error);

    lt::session ses(std::move(params));

    lt::add_torrent_params atp = lt::parse_magnet_uri(mangetURI.data());
    atp.save_path = ".";
    atp.flags &= ~(lt::torrent_flags::auto_managed | lt::torrent_flags::paused);
    atp.file_priorities.resize(100, lt::dont_download);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    lt::torrent_handle h = ses.add_torrent(std::move(atp));

    std::pair<std::string, int> utorrentNode("router.utorrent.com", 6881);
    std::pair<std::string, int> bittorrentNode("router.bittorrent.com", 6881);
    std::pair<std::string, int> transmissionNode("dht.transmissionbt.com", 6881);
    std::pair<std::string, int> bitcometNode("router.bitcomet.com", 6881);
    std::pair<std::string, int> aelitisNode("dht.aelitis.com", 6881);

    ses.add_dht_node(std::move(utorrentNode));
    ses.add_dht_node(std::move(bittorrentNode));
    ses.add_dht_node(std::move(transmissionNode));
    ses.add_dht_node(std::move(bitcometNode));
    ses.add_dht_node(std::move(aelitisNode));

    bool isDone = false;
    while (!isDone)
    {
        std::vector<lt::alert *> alerts;
        ses.pop_alerts(&alerts);
        for (lt::alert *a : alerts)
        {
            if (auto const *mra = lt::alert_cast<lt::metadata_received_alert>(a))
            {
                auto const handle = mra->handle;
                std::shared_ptr<lt::torrent_info const> ti = handle.torrent_file();
                if (!ti)
                {
                    return false;
                }
                handle.save_resume_data(lt::torrent_handle::save_info_dict);
                handle.set_flags(lt::torrent_flags::paused);
            }
            else if (auto *rda = lt::alert_cast<lt::save_resume_data_alert>(a))
            {
                rda->params.merkle_trees.clear();
                lt::entry e = lt::write_torrent_file(rda->params, lt::write_flags::allow_missing_piece_layer);
                std::vector<char> torrent;
                lt::bencode(std::back_inserter(torrent), e);
                auto handle = rda->handle;
                outputTorrentFile = handle.torrent_file()->name();
                *size = torrent.size();
                *binFile = new char[*size];
                memcpy(*binFile, torrent.data(), *size);
                isDone = true;
                break;
            }
            else if (auto const *rdf = lt::alert_cast<lt::save_resume_data_failed_alert>(a))
            {
                return false;
            }
        }
    }
    return true;
}

int GetTorrent(const char *uri, char** outputTorrentName ,char **outputTorrentFile, int* size)
{
    std::string outputFileName;
    if (GetTorrent(std::string(uri), outputFileName, outputTorrentFile, size))
    {
        *outputTorrentName = new char[outputFileName.length() + 1];
        strcpy(*outputTorrentName, outputFileName.data());
        return 1;
    }
    else
    {
        return 0;
    }
}

void ReleaseTorrent(char** fileName, char** fileBin)
{
    delete[] (*fileName);
    *fileName = nullptr;

    delete[] (*fileBin);
    *fileBin = nullptr;
}

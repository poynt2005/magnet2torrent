# magnet2torrent

最近發現 TorrentInfo_Web 的 bug 太多，修不好了  
這次利用 libtorrent 配合 python 寫一個純 magnet to torrent api 的小專案

用法:
docker run --rm -p <your port>:8080 -d poynt2005/magnet2torrent

API:
GET /get_torrent?link=<magnet link>

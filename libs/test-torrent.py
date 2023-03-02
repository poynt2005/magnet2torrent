import PyGetTorrent 

uri = 'magnet:?xt=urn:btih:THECXNZVAWR4BNCT7H5A5CA5NZNDFIGB&dn=ubuntu-22.10-desktop-amd64.iso&xl=4071903232&tr=https%3A%2F%2Ftorrent.ubuntu.com%2Fannounce'

torrent = PyGetTorrent.GetTorrent(uri)

print(torrent)

with open("pyout.torrent", 'wb') as f:
    f.write(torrent['torrent_binary'])
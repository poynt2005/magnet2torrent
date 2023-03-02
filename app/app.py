from flask import Flask, request, jsonify, redirect, send_file
from io import BytesIO
import PyGetTorrent as pgt

app = Flask(__name__,
            static_url_path='',
            static_folder='dist'
            )


@app.route('/', methods=['GET'])
def index():
    return 'This is Magnet2Torrent api service', 200


@app.route('/get_torrent', methods=['GET'])
def get_torrent():
    uri = request.args.get("link", None)

    if uri is None:
        return 'Magnet Uri not specified', 400
    
    torrent = pgt.GetTorrent(uri)

    if torrent is None:
        return 'Magnet Uri not found', 400

    bin_file = BytesIO()
    bin_file.write(torrent['torrent_binary'])
    bin_file.seek(0)

    return send_file(bin_file, download_name=torrent['torrent_name'] + '.torrent', mimetype='application/x-bittorrent')


if __name__ == "__main__":
    app.debug = True
    app.run(host='0.0.0.0', port=8080)

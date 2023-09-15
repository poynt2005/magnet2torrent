# magnet2torrent

最近發現 TorrentInfo_Web 的 bug 太多，修不好了  
這次利用 libtorrent 配合 python 寫一個純 magnet to torrent api 的小專案  

### 2023-09-15 更新
因為建出來的 image 太大 > 3千兆字節，於是重改架構  
重整的部分：  
1. C++ 調整架構，新增錯誤碼的定義，使用句柄操控對象    
2. 完全移除 python，改用 go 建置 api  
3. 因為有用到 glibc 的函數，所以很遺憾無法使用 alpine 作為基底鏡像  
調整後的鏡像容量大概為 260兆字節，雖然還是不小，不過比原本的降低不少了  

### 建置
```
docker build -t magnet2torrent . 
```

### 運行
請參考 runscript.txt，其中  
1. your_api_port: 你的 api 連線埠
2. your_dht_port: 你的 dht 連線埠

### 使用
直接調用 api  
GET /get_torrent?link=<磁力鏈接> -> 獲得該磁力鏈接的 torrent 檔案  
POST /get_magnet formdata 帶入 torrent 二進位檔案 -> 獲得磁力鏈接  
可以參考  add_upload_input.js 的調用方法  

### 備註
推至個人的私有容器倉庫

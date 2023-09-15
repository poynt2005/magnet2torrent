package main

import (
	"fmt"
	"net/http"
	"server/torrentcontext"
	"strconv"
	"time"

	"github.com/bitly/go-simplejson"
	"github.com/gorilla/mux"
)

const SERV_PORT = 6133

func main() {

	router := mux.NewRouter()

	router.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Printf("[Info] %s on route /get_torrent\n", r.Method)
		respJson := simplejson.New()
		respJson.Set("isSuccess", false)

		isSuccess := false
		if r.Method == "GET" {
			isSuccess = true
			respJson.Set("isSuccess", true)
			respJson.Set("data", "This is Magnet2Torrent api service")
		} else {
			respJson.Set("reason", fmt.Sprintf("Method %s Not Allowed", r.Method))
		}

		if !isSuccess {
			w.WriteHeader(http.StatusBadRequest)
		} else {
			w.WriteHeader(http.StatusOK)
		}
		w.Header().Set("Content-Type", "application/json; charset=utf-8")
		txt, _ := respJson.MarshalJSON()
		w.Write(txt)
	})

	router.HandleFunc("/get_torrent", func(w http.ResponseWriter, r *http.Request) {
		fmt.Printf("[Info] %s on route /get_torrent\n", r.Method)
		respJson := simplejson.New()
		respJson.Set("isSuccess", false)

		isSuccess := false
		if r.Method == "GET" {
			link := r.URL.Query().Get("link")

			if len(link) > 0 {
				tortCtx := torrentcontext.NewTorrent()
				defer tortCtx.Dispose()
				tortCtx.MagnetURL = link

				if err := tortCtx.MagnetToTorrent(); err != nil {
					respJson.Set("reason", err.Error())
				} else {
					isSuccess = true

					w.Header().Set("Content-Type", "applications/x-bittorrent")
					w.Header().Set("Content-Length", strconv.Itoa(len(tortCtx.TorrentBuffer)))
					w.Write(tortCtx.TorrentBuffer)
					return
				}
			} else {
				respJson.Set("reason", "Magnet Uri not specified")
			}
		} else {
			respJson.Set("reason", fmt.Sprintf("Method %s Not Allowed", r.Method))
		}

		if !isSuccess {
			w.WriteHeader(http.StatusBadRequest)
		} else {
			w.WriteHeader(http.StatusOK)
		}

		w.Header().Set("Content-Type", "application/json; charset=utf-8")
		txt, _ := respJson.MarshalJSON()
		w.Write(txt)
	})

	router.HandleFunc("/get_magnet", func(w http.ResponseWriter, r *http.Request) {
		fmt.Printf("[Info] %s on route /get_torrent\n", r.Method)
		respJson := simplejson.New()
		respJson.Set("isSuccess", false)

		isSuccess := false

		if r.Method == "POST" {
			r.ParseMultipartForm(32 << 20)

			file, _, err := r.FormFile("torrent")
			defer file.Close()
			if err != nil {
				respJson.Set("reason", "Cannot get file from form body")
			} else {
				contents := []byte{}
				for {
					buf := make([]byte, 1204)
					n, err := file.Read(buf)
					contents = append(contents, buf[:n]...)
					if err != nil {
						break
					}
				}

				if len(contents) == 0 {
					respJson.Set("reason", "Read bytes file from file failed")
				} else {
					tortCtx := torrentcontext.NewTorrent()
					defer tortCtx.Dispose()
					tortCtx.TorrentBuffer = contents

					if err := tortCtx.TorrentToMagnet(); err != nil {
						respJson.Set("reason", err.Error())
					} else {
						isSuccess = true
						respJson.Set("isSuccess", true)
						respJson.Set("data", tortCtx.MagnetURL)
					}
				}
			}
		} else {
			respJson.Set("reason", fmt.Sprintf("Method %s Not Allowed", r.Method))
		}

		if !isSuccess {
			w.WriteHeader(http.StatusBadRequest)
		} else {
			w.WriteHeader(http.StatusOK)
		}

		w.Header().Set("Content-Type", "application/json; charset=utf-8")
		txt, _ := respJson.MarshalJSON()
		w.Write(txt)
	})

	router.NotFoundHandler = http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		respJson := simplejson.New()
		respJson.Set("isSuccess", false)
		respJson.Set("reason", fmt.Sprintf("Cannot match this route: %s", r.URL.Path))
		w.Header().Set("Content-Type", "application/json; charset=utf-8")
		txt, _ := respJson.MarshalJSON()
		w.Write(txt)
	})

	srv := &http.Server{
		Addr:    ":"+strconv.Itoa(SERV_PORT),
		WriteTimeout: 300 * time.Second,
		Handler: router,
	}

	fmt.Printf("[Info] Server is running on port %d\n", SERV_PORT)
	if serverError := srv.ListenAndServe(); serverError != nil {
		fmt.Printf("[Error] Cannot start server on port %d\n", SERV_PORT)
	}
}

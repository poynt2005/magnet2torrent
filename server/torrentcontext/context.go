package torrentcontext

/*
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#cgo windows CFLAGS: -DCGO_OS_WINDOWS=1
#cgo linux  CFLAGS: -DCGO_OS_LINUX=1
#if defined(CGO_OS_WINDOWS)
#include "../../libs/Magnet2Torrent.h"
#cgo LDFLAGS: -L../../libs
#cgo LDFLAGS: -lMagnet2Torrent
#elif defined(CGO_OS_LINUX)
#include "../../m2t-libs/Magnet2Torrent.h"
#cgo LDFLAGS: -L../../m2t-libs
#cgo LDFLAGS: -lMagnet2Torrent -ltorrent-rasterbar -lboost_system
#endif
*/
import "C"
import (
	"errors"
	"unsafe"
)

type TorrentContext struct {
	TorrentBuffer []byte
	MagnetURL     string
	torrentHandle uint64
}

type TorrentConv interface {
	TorrentToMagnet() error
	MagnetToTorrent() error
	Dispose()
}

func NewTorrent() *TorrentContext {
	torrentHandle := C.CreateTorrent()

	return &TorrentContext{
		[]byte{},
		"",
		uint64(torrentHandle),
	}
}

func (ctx *TorrentContext) Dispose() {
	C.DestroyTorrent(C.ulonglong(ctx.torrentHandle))
}

func mapError(code C.int) error {
	switch code {
	case C.TORRENT_CONTEXT_NOT_CREATED:
		return errors.New("Torrent Context Not Created")
	case C.TORRENT_FAILED:
		return errors.New("Torrent Operation Failed")
	case C.M2T_MAGNET_LINK_NOT_SET:
		return errors.New("Magnet Link Not Set")
	case C.M2T_BUFFER_EMPTY:
		return errors.New("Buffer Empty")
	case C.M2T_MANGET_LINK_NOT_VALID:
		return errors.New("Magnet Link Not Valid")
	case C.M2T_FAILED_CREATE_TEMP_FOLDER:
		return errors.New("Failed Create Temp Folder")
	case C.M2T_FAILED_GET_FILE_FROM_TORRENT_HANDLE:
		return errors.New("Failed Get File From Torrent Handle")
	case C.M2T_LT_RECERIEVED_FAILED_ALERT:
		return errors.New("LibTorrent Recerieved Failed Alert")
	case C.M2T_WRITE_TORRENT_ERROR:
		return errors.New("Write Torrent Error")
	case C.M2T_LT_FAILED_DECODE_BENCODE:
		return errors.New("LibTorrent Failed Decode Bencode")
	case C.M2T_LT_FAILED_DECODE_MAGNET:
		return errors.New("LibTorrent Failed Decode Magnet")
	case C.M2T_LT_SEARCH_TIME_OUT:
		return errors.New("LibTorrent Magnet Search Timeout")
	default:
		return errors.New("Unknown Error")
	}
}

func (ctx *TorrentContext) setTorrentBuffer(buffer []byte, size uint64) error {

	cBuffer := unsafe.Pointer(&buffer[0])
	cSize := C.size_t(size)

	result := C.SetTorrentBuffer(C.ulonglong(ctx.torrentHandle), (*C.char)(cBuffer), cSize)

	if result == C.TORRENT_CONTEXT_NOT_CREATED {
		return mapError(result)
	}

	if err := mapError(C.GetTorrentError(C.ulonglong(ctx.torrentHandle))); result == C.TORRENT_FAILED {
		return err
	}

	return nil
}

func (ctx *TorrentContext) setMagnet(magnetURL string) error {
	cStr := C.CString(magnetURL)

	result := C.SetMagnet(C.ulonglong(ctx.torrentHandle), cStr)
	C.free(unsafe.Pointer(cStr))

	if result == C.TORRENT_CONTEXT_NOT_CREATED {
		return mapError(result)
	}

	if err := mapError(C.GetTorrentError(C.ulonglong(ctx.torrentHandle))); result == C.TORRENT_FAILED {
		return err
	}

	return nil
}

func (ctx *TorrentContext) getTorrent() ([]byte, error) {
	cSize := C.size_t(0)
	cBuffer := C.GetTorrent(C.ulonglong(ctx.torrentHandle), &cSize)

	if cBuffer == nil {
		return []byte{}, mapError(C.GetTorrentError(C.ulonglong(ctx.torrentHandle)))
	}

	buffer := C.GoBytes(unsafe.Pointer(cBuffer), C.int(cSize))

	return buffer, nil
}

func (ctx *TorrentContext) getMagnet() (string, error) {
	cStr := C.GetMagnet(C.ulonglong(ctx.torrentHandle))

	if cStr == nil {
		return "", mapError(C.GetTorrentError(C.ulonglong(ctx.torrentHandle)))
	}

	return C.GoString(cStr), nil
}

func (ctx *TorrentContext) MagnetToTorrent() error {
	if err := ctx.setMagnet(ctx.MagnetURL); err != nil {
		return err
	}

	result := C.Magnet2Torrent(C.ulonglong(ctx.torrentHandle))

	if result == C.TORRENT_CONTEXT_NOT_CREATED {
		return mapError(result)
	}

	if err := mapError(C.GetTorrentError(C.ulonglong(ctx.torrentHandle))); result == C.TORRENT_FAILED {
		return err
	}

	torrentBytes, err := ctx.getTorrent()

	if err != nil {
		return err
	}

	ctx.TorrentBuffer = torrentBytes

	return nil
}

func (ctx *TorrentContext) TorrentToMagnet() error {
	if err := ctx.setTorrentBuffer(ctx.TorrentBuffer, uint64(len(ctx.TorrentBuffer))); err != nil {
		return err
	}

	result := C.Torrent2Magnet(C.ulonglong(ctx.torrentHandle))

	if result == C.TORRENT_CONTEXT_NOT_CREATED {
		return mapError(result)
	}

	if err := mapError(C.GetTorrentError(C.ulonglong(ctx.torrentHandle))); result == C.TORRENT_FAILED {
		return err
	}

	magnetURL, err := ctx.getMagnet()

	if err != nil {
		return err
	}

	ctx.MagnetURL = magnetURL

	return nil
}
